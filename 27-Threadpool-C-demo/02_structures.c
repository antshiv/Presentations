// 02_structures.c
// Build: gcc -O2 -Wall -Wextra -pthread 02_structures.c -o 02_structures
// Run:   ./02_structures
//
// What this file proves:
//   Before writing worker logic, define the runtime contract. A useful
//   threadpool owns worker handles, synchronization objects, current work,
//   completion state, and shutdown state.
//
// What to watch when running it:
//   This file does not create threads. It only prints sizes and names the fields.
//   The point is to make the hidden runtime state visible before adding behavior.
//
// Why this matters for C-Kernel-Engine style code:
//   CPU kernels should receive clean pointers and ranges. The runtime around the
//   kernel should own dispatch, barriers, stop flags, and scheduling state.

#include <pthread.h>
#include <stdio.h>

// Every dispatched job uses the same function shape.
// ith = this worker index, nth = total participating workers, arg = user payload.
typedef void (*work_fn_t)(int ith, int nth, void *arg);

typedef struct {
    pthread_t *threads;      // worker handles created once and joined at shutdown
    int nthreads;            // total workers including main thread 0

    pthread_mutex_t mu;      // protects every mutable field below
    pthread_cond_t has_work; // workers sleep here until a new generation appears
    pthread_cond_t done;     // main thread sleeps here until workers finish

    work_fn_t fn;            // current dispatched function
    void *arg;               // current dispatched payload

    int generation;          // increments once per dispatch to avoid missed work
    int completed;           // number of background workers done this generation
    int stop;                // shutdown flag broadcast to sleeping workers
} threadpool_t;

typedef struct {
    threadpool_t *pool;
    int ith;
} worker_arg_t;

int main(void) {
    printf("stage 2: define the runtime state before writing worker behavior\n");
    printf("threadpool_t stores workers, synchronization, dispatch state, and stop state.\n");
    printf("sizeof(threadpool_t) = %zu bytes\n", sizeof(threadpool_t));
    printf("sizeof(worker_arg_t) = %zu bytes\n", sizeof(worker_arg_t));
    return 0;
}
