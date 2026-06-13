// 04_dispatch_minimal.c
// Build: gcc -O2 -Wall -Wextra -pthread 04_dispatch_minimal.c -o 04_dispatch_minimal
// Run:   ./04_dispatch_minimal 4
//
// What this file proves:
//   The same persistent worker threads can receive multiple dispatches. The
//   generation counter marks each new work item, and the done condition acts as
//   a reusable barrier for the main thread.
//
// What to watch when running it:
//   Dispatch 1 and dispatch 2 use the same pool. Thread creation happens once;
//   work dispatch happens repeatedly.
//
// Why this matters for model runtimes:
//   A transformer layer is many kernel dispatches. Reusing the same workers keeps
//   the runtime cost predictable across GEMV, RMSNorm, attention, and MLP kernels.

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

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

    int generation;
    int completed;
    int stop;
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

        // Sleep until either shutdown is requested or generation changes.
        // The while loop handles spurious wakeups and workers waking together.
        while (!p->stop && p->generation == seen_generation) {
            pthread_cond_wait(&p->has_work, &p->mu);
        }
        if (p->stop) {
            pthread_mutex_unlock(&p->mu);
            break;
        }

        // Copy dispatch state while holding the mutex, then release it before work.
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
        p->threads = calloc((size_t)(nthreads - 1), sizeof(*p->threads));
        p->args = calloc((size_t)(nthreads - 1), sizeof(*p->args));
        if (!p->threads || !p->args) {
            free(p->args);
            free(p->threads);
            pthread_cond_destroy(&p->done);
            pthread_cond_destroy(&p->has_work);
            pthread_mutex_destroy(&p->mu);
            free(p);
            return NULL;
        }

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

    // Wake all workers. Each worker compares generation against its local seen value.
    pthread_cond_broadcast(&p->has_work);
    pthread_mutex_unlock(&p->mu);

    // Thread 0 is the caller, and it participates in the same work function.
    fn(0, p->nthreads, arg);

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

static void hello_work(int ith, int nth, void *arg) {
    const char *label = (const char *)arg;
    printf("%s: thread %d/%d received dispatch\n", label, ith, nth);
}

int main(int argc, char **argv) {
    int nthreads = argc > 1 ? atoi(argv[1]) : 4;
    threadpool_t *pool = threadpool_create(nthreads);
    if (!pool) return 1;

    threadpool_dispatch(pool, hello_work, "dispatch 1");
    printf("second dispatch reuses the same pool\n");
    threadpool_dispatch(pool, hello_work, "dispatch 2");

    threadpool_destroy(pool);
    return 0;
}
