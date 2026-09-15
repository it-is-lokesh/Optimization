// gcc -O3 -pthread no_volatile.c -o no_volatile
// gcc -O3 -S no_volatile.c -o no_volatile.s
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdatomic.h>

volatile int flag = 0;

void *worker(void *arg)
{
    sleep(2);

    printf("Worker: setting flag = 1\n");
    flag = 1;

    return NULL;
}

int main()
{
    pthread_t tid;

    pthread_create(&tid, NULL, worker, NULL);

    printf("Main: waiting for flag...\n");

    while (flag == 0) {
        // Busy wait
        // asm volatile("" ::: "memory");
    }

    printf("Main: flag detected!\n");

    pthread_join(tid, NULL);

    return 0;
}