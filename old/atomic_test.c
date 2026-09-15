// gcc -O3 atomic_test.c -o atomic_test
#define _GNU_SOURCE

#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <sched.h>

#define ITERATIONS 10000000

int data = 0;

/* Producer -> Consumer */
volatile int flag = 0;

/* Consumer -> Producer */
atomic_int ack = 0;

void pin_to_core(int core)
{
    cpu_set_t cpuset;

    CPU_ZERO(&cpuset);
    CPU_SET(core, &cpuset);

    pthread_setaffinity_np(
        pthread_self(),
        sizeof(cpu_set_t),
        &cpuset
    );
}

void *producer(void *arg)
{
    pin_to_core(0);

    for (int iter = 1; iter <= ITERATIONS; iter++)
    {
        data = iter;

        // atomic_store(&flag, iter);
        flag = iter;
        // printf("Producer: set data=%d flag=%d\n", data, flag);
        while (atomic_load(&ack) != iter);

        if ((iter % 100000) == 0)
        {
            printf("Producer reached iteration %d\n", iter);
        }
    }

    return NULL;
}

void *consumer(void *arg)
{
    pin_to_core(2);   /* choose a different physical core */

    for (int expected = 1; expected <= ITERATIONS; expected++)
    {
        // printf("Consumer: waiting for flag=%d\n", expected);
        // while (atomic_load(&flag) != expected);
        while(flag != expected);

        int observed = data;

        if (observed != expected)
        {
            printf(
                "BUG DETECTED: expected=%d observed=%d\n",
                expected,
                observed
            );
        }

        atomic_store(&ack, expected);
    }

    return NULL;
}

int main()
{
    pthread_t prod;
    pthread_t cons;

    pthread_create(&prod, NULL, producer, NULL);
    pthread_create(&cons, NULL, consumer, NULL);

    pthread_join(prod, NULL);
    pthread_join(cons, NULL);

    printf("Finished %d iterations\n", ITERATIONS);

    return 0;
}