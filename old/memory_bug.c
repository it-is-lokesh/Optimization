// reorder_test.c
// gcc -O3 -pthread memory_bug.c -o memory_bug

#define _GNU_SOURCE

#include <stdio.h>
#include <pthread.h>
#include <sched.h>
#include <stdatomic.h>

#define ITERATIONS 10000

int data;
volatile int flag;

atomic_int start_epoch = 0;
atomic_int done_count = 0;

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

    int local_epoch = 0;

    while (1)
    {
        // printf("Producer: waiting for epoch %d\n", local_epoch + 1);
        while (atomic_load(&start_epoch) == local_epoch)
        {
        }

        local_epoch++;

        data = local_epoch;
        flag = 1;

        atomic_fetch_add(&done_count, 1);
        if(local_epoch % 1000000 == 0)printf("Producer reached epoch %d\n", local_epoch);
    }

    return NULL;
}

void *consumer(void *arg)
{
    pin_to_core(1);

    int local_epoch = 0;

    while (1)
    {
        // printf("Consumer: waiting for epoch %d\n", local_epoch + 1);
        while (atomic_load(&start_epoch) == local_epoch)
        {
        }

        local_epoch++;

        while (flag == 0)
        {
        }

        if (data != local_epoch)
        {
            printf(
                "BUG DETECTED "
                "expected=%d data=%d flag=%d\n",
                local_epoch,
                data,
                flag
            );
        }

        atomic_fetch_add(&done_count, 1);
    }

    return NULL;
}

int main()
{
    pthread_t prod;
    pthread_t cons;

    pthread_create(&prod, NULL, producer, NULL);
    pthread_create(&cons, NULL, consumer, NULL);

    for (int iter = 1; iter <= ITERATIONS; iter++)
    {
        flag = 0;
        data = 0;

        atomic_store(&done_count, 0);

        atomic_store(&start_epoch, iter);

        while (atomic_load(&done_count) != 2)
        {
        }

        // if ((iter % 100000) == 0)
        // {
        //     printf("Iteration %d completed\n", iter);
        // }
    }
    printf("Finished %d iterations\n", ITERATIONS);

    pthread_join(prod, NULL);
    pthread_join(cons, NULL);

    printf("Finished\n");

    return 0;
}
