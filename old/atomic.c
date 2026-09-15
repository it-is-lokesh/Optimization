// atomic_demo.c
// gcc -O3 atomic.c -o atomic

#define _GNU_SOURCE

#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>
#include <unistd.h>
#include <sched.h>

// int data = 0;
// atomic_int flag = 0;
// int flag = 0;
// volatile flag = 0;

struct separate_cache_line {
    int data;
    char pad[64];
    // atomic_int flag;
    volatile int flag;
};

struct separate_cache_line shared;
pthread_mutex_t mux;

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
    sleep(1);

    // pthread_mutex_lock(&mux);

    printf("Producer: writing data=123\n");

    shared.data = 123;

    // printf("Producer: setting flag=1\n");

    // atomic_store(&shared.flag, 1);
    shared.flag = 1;

    // pthread_mutex_unlock(&mux);

    return NULL;
}

void *consumer(void *arg)
{
    pin_to_core(7);
    // pthread_mutex_lock(&mux);
    printf("Consumer: waiting...\n");

    while (shared.flag == 0){
        // asm volatile("" ::: "memory");
    }
    // while (atomic_load(&shared.flag) == 0);

    printf("Consumer: flag observed\n");
    printf("Consumer: data=%d\n", shared.data);
    // pthread_mutex_unlock(&mux);

    return NULL;
}

int main()
{
    pthread_t t1, t2;

    pthread_create(&t1, NULL, producer, NULL);
    pthread_create(&t2, NULL, consumer, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    return 0;
}
