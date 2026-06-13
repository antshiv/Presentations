// threadpool_demo.c
// Build: gcc -O2 -Wall -Wextra -pthread threadpool_demo.c -o threadpool_demo
// Run:   ./threadpool_demo 4
//
// What this file proves:
//   The previous four stages become a small runtime: create workers once,
//   dispatch work many times, partition rows, wait for completion, and shut down.
//
// What to watch when running it:
//   Each thread owns a half-open row range [r0, r1). The exact print order may
//   vary, but every row should be covered exactly once. Dispatch 2 reuses the
//   same worker threads without calling pthread_create again.
//
// Why this matters for C-Kernel-Engine:
//   This is the runtime shape behind CPU model execution. Replace matvec_work()
//   with GEMV, RMSNorm, attention, or optimizer kernels and the orchestration
//   contract stays the same.
//
// This is a teaching demo, not a production threadpool.

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef void (*work_fn_t)(int ith, int nth, void *arg);

typedef struct threadpool threadpool_t;

typedef struct {
    threadpool_t *pool;
    int ith;
} worker_arg_t;

struct threadpool {
    pthread_t *threads;
    worker_arg_t *args;
    int nthreads;

    pthread_mutex_t mu;
    pthread_cond_t has_work;
    pthread_cond_t done;

    work_fn_t fn;
    void *arg;

    int generation;      // incremented for every dispatch
    int completed;       // background workers completed in this generation
    int stop;            // shutdown flag consumed by sleeping workers
};

static void die_if_pthread_error(int rc, const char *what) {
    if (rc != 0) {
        fprintf(stderr, "%s failed with pthread error code %d\n", what, rc);
        exit(1);
    }
}

static void *worker_main(void *ptr) {
    worker_arg_t *wa = (worker_arg_t *)ptr;
    threadpool_t *p = wa->pool;
    int ith = wa->ith;
    int seen_generation = 0;

    for (;;) {
        pthread_mutex_lock(&p->mu);
        while (!p->stop && p->generation == seen_generation) {
            pthread_cond_wait(&p->has_work, &p->mu);
        }
        if (p->stop) {
            pthread_mutex_unlock(&p->mu);
            break;
        }

        // Snapshot the dispatch while protected, then do expensive work unlocked.
        work_fn_t fn = p->fn;
        void *arg = p->arg;
        int nth = p->nthreads;
        seen_generation = p->generation;
        pthread_mutex_unlock(&p->mu);

        fn(ith, nth, arg);

        pthread_mutex_lock(&p->mu);
        p->completed++;
        if (p->completed == p->nthreads - 1) {
            pthread_cond_signal(&p->done);
        }
        pthread_mutex_unlock(&p->mu);
    }

    return NULL;
}

static threadpool_t *threadpool_create(int nthreads) {
    if (nthreads < 1) nthreads = 1;

    threadpool_t *p = calloc(1, sizeof(*p));
    if (!p) return NULL;

    p->nthreads = nthreads;
    die_if_pthread_error(pthread_mutex_init(&p->mu, NULL), "pthread_mutex_init");
    die_if_pthread_error(pthread_cond_init(&p->has_work, NULL), "pthread_cond_init has_work");
    die_if_pthread_error(pthread_cond_init(&p->done, NULL), "pthread_cond_init done");

    if (nthreads > 1) {
        p->threads = calloc((size_t)(nthreads - 1), sizeof(pthread_t));
        p->args = calloc((size_t)(nthreads - 1), sizeof(worker_arg_t));
        if (!p->threads || !p->args) {
            free(p->args);
            free(p->threads);
            pthread_cond_destroy(&p->done);
            pthread_cond_destroy(&p->has_work);
            pthread_mutex_destroy(&p->mu);
            free(p);
            return NULL;
        }

        // Worker args live on the pool because worker threads use them until shutdown.
        for (int i = 1; i < nthreads; ++i) {
            p->args[i - 1].pool = p;
            p->args[i - 1].ith = i;
            die_if_pthread_error(pthread_create(&p->threads[i - 1], NULL,
                                                worker_main, &p->args[i - 1]),
                                 "pthread_create");
        }
    }

    return p;
}

static void threadpool_dispatch(threadpool_t *p, work_fn_t fn, void *arg) {
    if (p->nthreads == 1) {
        fn(0, 1, arg);
        return;
    }

    pthread_mutex_lock(&p->mu);
    p->fn = fn;
    p->arg = arg;
    p->completed = 0;
    p->generation++;
    pthread_cond_broadcast(&p->has_work);
    pthread_mutex_unlock(&p->mu);

    // Thread 0 is the caller, and it does real work too.
    fn(0, p->nthreads, arg);

    // Wait until background workers 1..N-1 finish this generation.
    pthread_mutex_lock(&p->mu);
    while (p->completed < p->nthreads - 1) {
        pthread_cond_wait(&p->done, &p->mu);
    }
    pthread_mutex_unlock(&p->mu);
}

static void threadpool_destroy(threadpool_t *p) {
    if (!p) return;

    pthread_mutex_lock(&p->mu);
    p->stop = 1;
    pthread_cond_broadcast(&p->has_work);
    pthread_mutex_unlock(&p->mu);

    for (int i = 1; i < p->nthreads; ++i) {
        die_if_pthread_error(pthread_join(p->threads[i - 1], NULL), "pthread_join");
    }

    free(p->args);
    free(p->threads);
    pthread_cond_destroy(&p->done);
    pthread_cond_destroy(&p->has_work);
    pthread_mutex_destroy(&p->mu);
    free(p);
}

typedef struct {
    float *out;
    const float *x;
    const float *w;
    int rows;
    int cols;
} matvec_job_t;

static void matvec_work(int ith, int nth, void *arg) {
    matvec_job_t *job = (matvec_job_t *)arg;

    // Contiguous row partitioning: every row belongs to exactly one worker.
    int r0 = (ith * job->rows) / nth;
    int r1 = ((ith + 1) * job->rows) / nth;

    printf("thread %d/%d owns rows [%d, %d)\n", ith, nth, r0, r1);

    for (int r = r0; r < r1; ++r) {
        float sum = 0.0f;
        for (int c = 0; c < job->cols; ++c) {
            sum += job->w[r * job->cols + c] * job->x[c];
        }
        job->out[r] = sum;
    }
}

int main(int argc, char **argv) {
    int nthreads = argc > 1 ? atoi(argv[1]) : 4;
    int rows = 12;
    int cols = 8;

    float *w = calloc((size_t)rows * cols, sizeof(float));
    float *x = calloc((size_t)cols, sizeof(float));
    float *out = calloc((size_t)rows, sizeof(float));
    if (!w || !x || !out) return 1;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            w[r * cols + c] = (float)(r + 1) * 0.1f + (float)c;
        }
    }
    for (int c = 0; c < cols; ++c) x[c] = 1.0f;

    threadpool_t *pool = threadpool_create(nthreads);
    if (!pool) return 1;

    matvec_job_t job = {.out = out, .x = x, .w = w, .rows = rows, .cols = cols};

    printf("dispatch 1: matrix-vector dummy workload\n");
    threadpool_dispatch(pool, matvec_work, &job);

    printf("\noutput:\n");
    for (int r = 0; r < rows; ++r) {
        printf("out[%02d] = %.3f\n", r, out[r]);
    }

    printf("\ndispatch 2: same workers, same pool, no thread creation\n");
    memset(out, 0, (size_t)rows * sizeof(float));
    threadpool_dispatch(pool, matvec_work, &job);

    threadpool_destroy(pool);
    free(out);
    free(x);
    free(w);
    return 0;
}
