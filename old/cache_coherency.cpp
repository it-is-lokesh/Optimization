// #define _GNU_SOURCE

#include <pthread.h>
#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ITERATIONS 1000000000ULL

struct shared_bad {
    volatile uint32_t counter1; // 4 bytes
    volatile uint32_t counter3; // 4 bytes
    volatile uint32_t counter2; // 4 bytes
    volatile uint32_t counter4; // 4 bytes
// };
} __attribute__((aligned(64)));

struct shared_good {
    volatile uint32_t counter1; // 4 bytes
    volatile uint32_t counter3; // 4 bytes
    char pad[64-sizeof(uint32_t)*2]; // Pad to avoid false sharing
    volatile uint32_t counter2;
    volatile uint32_t counter4;
// };
} __attribute__((aligned(64)));

struct shared_bad bad;
struct shared_good good;

typedef struct {
    int cpu;
    int use_padding;
    volatile uint32_t *counter;
    volatile uint32_t *counter2;
} thread_arg_t;

static void pin_to_cpu(int cpu)
{
    cpu_set_t cpuset;

    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);

    pthread_setaffinity_np(
        pthread_self(),
        sizeof(cpu_set_t),
        &cpuset);
}

static void *worker(void *arg)
{
    thread_arg_t *t = (thread_arg_t *)arg;

    pin_to_cpu(t->cpu);

    for (uint64_t i = 0; i < ITERATIONS; i++) {
        (*t->counter)++;
        (*t->counter2)++;
    }

    return NULL;
}

double now_sec(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return ts.tv_sec + ts.tv_nsec / 1e9;
}

void run_test(int padded)
{
    pthread_t t1, t2;

    thread_arg_t a1;
    thread_arg_t a2;

    if (padded) {
        a1.counter = &good.counter1;
        a2.counter = &good.counter2;
        a1.counter2 = &good.counter3;
        a2.counter2 = &good.counter4;
    } else {
        a1.counter = &bad.counter1;
        a2.counter = &bad.counter2;
        a1.counter2 = &bad.counter3;
        a2.counter2 = &bad.counter4;
    }

    a1.cpu = 0;
    a2.cpu = 7;

    // Print counter addresses
    printf("Counter 1 address offset: %ld, Q: %ld\n", (unsigned long)a1.counter % 64, (unsigned long)a1.counter / 64);
    printf("Counter 2 address offset: %ld, Q: %ld\n", (unsigned long)a2.counter % 64, (unsigned long)a2.counter / 64);

    double start = now_sec();

    pthread_create(&t1, NULL, worker, &a1);
    pthread_create(&t2, NULL, worker, &a2);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    // print value of counters
    printf("Counter 1 final value: %u\n", *a1.counter);
    printf("Counter 2 final value: %u\n", *a2.counter);

    double end = now_sec();

    printf("%s : %.3f sec\n",
           padded ? "PADDED" : "FALSE_SHARING",
           end - start);
}

int main()
{
    printf("Iterations per thread: %llu\n",
           (unsigned long long)ITERATIONS);

    run_test(0);
    run_test(1);

    return 0;
}
