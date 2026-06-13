// 05_core_burn.c
// Build: gcc -O2 -Wall -Wextra -pthread 05_core_burn.c -o 05_core_burn
// Run:   ./05_core_burn 4 8
//
// What this file proves:
//   Multiple pthread workers can keep multiple CPU cores busy at the same time.
//
// What to watch when running it:
//   Open top or htop in another terminal. Run with 1 thread, then 4 threads.
//   You should see CPU usage spread across more cores for the 4-thread run.
//
// Why this matters:
//   A threadpool only helps if the work is large enough to keep cores busy.
//   This demo burns CPU intentionally so the effect is visible in top/htop.

#define _GNU_SOURCE
#include <pthread.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

typedef struct {
    int ith;
    int seconds;
    volatile double result;
} worker_arg_t;

static double now_seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

static void *burn_worker(void *ptr) {
    worker_arg_t *arg = (worker_arg_t *)ptr;
    const double end = now_seconds() + arg->seconds;
    uint64_t iters = 0;
    double x = 1.000001 + (double)arg->ith;

    while (now_seconds() < end) {
        // Deliberately do floating-point work that the compiler cannot delete.
        x = x * 1.0000001 + 0.0000003;
        if (x > 1000000.0) x = 1.000001;
        iters++;
    }

    arg->result = x;
    printf("worker %d finished on cpu %d after %llu iterations\n",
           arg->ith, sched_getcpu(), (unsigned long long)iters);
    return NULL;
}

int main(int argc, char **argv) {
    int nthreads = argc > 1 ? atoi(argv[1]) : 4;
    int seconds = argc > 2 ? atoi(argv[2]) : 8;
    if (nthreads < 1) nthreads = 1;
    if (nthreads > 128) nthreads = 128;
    if (seconds < 1) seconds = 1;

    pthread_t *threads = calloc((size_t)nthreads, sizeof(*threads));
    worker_arg_t *args = calloc((size_t)nthreads, sizeof(*args));
    if (!threads || !args) return 1;

    printf("burning CPU with %d worker thread(s) for %d second(s)\n", nthreads, seconds);
    printf("watch in another terminal: htop  or  top -H -p %d\n", getpid());

    for (int i = 0; i < nthreads; ++i) {
        args[i].ith = i;
        args[i].seconds = seconds;
        if (pthread_create(&threads[i], NULL, burn_worker, &args[i]) != 0) {
            fprintf(stderr, "pthread_create failed for worker %d\n", i);
            return 1;
        }
    }

    for (int i = 0; i < nthreads; ++i) {
        pthread_join(threads[i], NULL);
    }

    free(args);
    free(threads);
    return 0;
}
