# Threadpool curriculum Makefile
#
# Demo rhythm:
#   make run-stage1  # raw pthread_create / pthread_join
#   make run-stage2  # inspect the runtime state structs
#   make run-stage3  # create persistent workers and shut them down
#   make run-stage4  # dispatch reusable work twice
#   make run-final   # row-partitioned dummy GEMV workload
#
# The -pthread flag matters: it enables pthread-aware compilation and linking.

CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra -pthread

STAGES := 01_pthreads_minimal 02_structures 03_pool_create 04_dispatch_minimal threadpool_demo 05_core_burn

all: $(STAGES)

01_pthreads_minimal: 01_pthreads_minimal.c
	$(CC) $(CFLAGS) $< -o $@

02_structures: 02_structures.c
	$(CC) $(CFLAGS) $< -o $@

03_pool_create: 03_pool_create.c
	$(CC) $(CFLAGS) $< -o $@

04_dispatch_minimal: 04_dispatch_minimal.c
	$(CC) $(CFLAGS) $< -o $@

threadpool_demo: threadpool_demo.c
	$(CC) $(CFLAGS) $< -o $@

05_core_burn: 05_core_burn.c
	$(CC) $(CFLAGS) $< -o $@

run-stage1: 01_pthreads_minimal
	./01_pthreads_minimal

run-stage2: 02_structures
	./02_structures

run-stage3: 03_pool_create
	./03_pool_create 4

run-stage4: 04_dispatch_minimal
	./04_dispatch_minimal 4

run-final: threadpool_demo
	./threadpool_demo 4

run-burn-1: 05_core_burn
	./05_core_burn 1 8

run-burn-4: 05_core_burn
	./05_core_burn 4 8

run-all: run-stage1 run-stage2 run-stage3 run-stage4 run-final

run: run-final

clean:
	rm -f $(STAGES)
