// 01_pthreads_minimal.c
// Build: gcc -O2 -Wall -Wextra -pthread 01_pthreads_minimal.c -o 01_pthreads_minimal
// Run:   ./01_pthreads_minimal
//
// What this file proves:
//   A POSIX thread is just a worker function started by pthread_create().
//   The main thread and worker thread then run independently until we join.
//
// What to watch when running it:
//   The print order is not guaranteed. Sometimes the main thread prints first;
//   sometimes the worker does. That nondeterminism is the first real lesson.
//
// Why this matters for a threadpool:
//   A threadpool is not magic. It is pthread_create() plus a long-lived worker
//   loop, a queue/dispatch contract, and a shutdown contract.

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

static void die_if_pthread_error(int rc, const char *what) {
    if (rc != 0) {
        fprintf(stderr, "%s failed with pthread error code %d\n", what, rc);
        exit(1);
    }
}

static void *worker_main(void *arg) {
    // pthread_create() gives the worker exactly one void* argument.
    // Real runtimes usually pass a small struct here. For stage 1, one int is enough.
    int id = *(int *)arg;
    printf("hello from worker thread %d\n", id);
    return NULL;
}

int main(void) {
    pthread_t thread;
    int id = 1;

    // -pthread is required at build time so the compiler and linker use pthread support.
    die_if_pthread_error(pthread_create(&thread, NULL, worker_main, &id),
                         "pthread_create");

    printf("hello from main thread 0\n");

    // join is the simplest barrier: main cannot exit until the worker is finished.
    die_if_pthread_error(pthread_join(thread, NULL), "pthread_join");

    return 0;
}
