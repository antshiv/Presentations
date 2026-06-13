// 03_pool_create.c
// Build: gcc -O2 -Wall -Wextra -pthread 03_pool_create.c -o 03_pool_create
// Run:   ./03_pool_create 4
//
// What this file proves:
//   Create worker threads once, make them sleep on a condition variable, and wake
//   them only for shutdown. This removes thread creation from the hot path.
//
// What to watch when running it:
//   Workers print that they are alive, then they wait. Destroy broadcasts stop,
//   each worker exits, and main joins them cleanly.
//
// Why this matters for kernels:
//   If every GEMV/GEMM/RMSNorm dispatch creates new threads, runtime overhead will
//   dominate small kernels. Persistent workers are the foundation for fast reuse.

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct threadpool threadpool_t;

typedef struct {
    threadpool_t *pool;
    int ith;
} worker_arg_t;

struct threadpool {
    pthread_t *threads;
    worker_arg_t *args;
    int nthreads;
    int stop;

    pthread_mutex_t mu;
    pthread_cond_t has_work;
};

static void sleep_millis(long ms) {
    struct timespec ts = {.tv_sec = ms / 1000, .tv_nsec = (ms % 1000) * 1000000L};
    nanosleep(&ts, NULL);
}

static void die_if_pthread_error(int rc, const char *what) {
    if (rc != 0) {
        fprintf(stderr, "%s failed with pthread error code %d\n", what, rc);
        exit(1);
    }
}

static void *worker_main(void *ptr) {
    worker_arg_t *wa = (worker_arg_t *)ptr;
    threadpool_t *p = wa->pool;

    pthread_mutex_lock(&p->mu);
    while (!p->stop) {
        printf("worker %d alive, waiting for work or shutdown\n", wa->ith);

        // pthread_cond_wait atomically unlocks p->mu and sleeps.
        // When it wakes, it locks p->mu again before returning.
        pthread_cond_wait(&p->has_work, &p->mu);
    }
    pthread_mutex_unlock(&p->mu);

    printf("worker %d exiting\n", wa->ith);
    return NULL;
}

static threadpool_t *threadpool_create(int nthreads) {
    if (nthreads < 1) nthreads = 1;

    threadpool_t *p = calloc(1, sizeof(*p));
    if (!p) return NULL;

    p->nthreads = nthreads;
    die_if_pthread_error(pthread_mutex_init(&p->mu, NULL), "pthread_mutex_init");
    die_if_pthread_error(pthread_cond_init(&p->has_work, NULL), "pthread_cond_init");

    // Thread 0 is the caller/main thread. We only create background workers 1..N-1.
    if (nthreads > 1) {
        p->threads = calloc((size_t)(nthreads - 1), sizeof(*p->threads));
        p->args = calloc((size_t)(nthreads - 1), sizeof(*p->args));
        if (!p->threads || !p->args) {
            free(p->args);
            free(p->threads);
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

static void threadpool_destroy(threadpool_t *p) {
    if (!p) return;

    pthread_mutex_lock(&p->mu);
    p->stop = 1;

    // Broadcast because every sleeping worker must wake up and see stop=1.
    pthread_cond_broadcast(&p->has_work);
    pthread_mutex_unlock(&p->mu);

    for (int i = 1; i < p->nthreads; ++i) {
        die_if_pthread_error(pthread_join(p->threads[i - 1], NULL), "pthread_join");
    }

    free(p->args);
    free(p->threads);
    pthread_cond_destroy(&p->has_work);
    pthread_mutex_destroy(&p->mu);
    free(p);
}

int main(int argc, char **argv) {
    int nthreads = argc > 1 ? atoi(argv[1]) : 4;

    printf("creating pool with %d total threads; main is thread 0\n", nthreads);
    threadpool_t *pool = threadpool_create(nthreads);
    if (!pool) return 1;

    printf("pool created; no work dispatch yet\n");
    sleep_millis(80); // Give workers time to reach the waiting state for the demo output.
    threadpool_destroy(pool);
    printf("pool destroyed cleanly\n");
    return 0;
}
