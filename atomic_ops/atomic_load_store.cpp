#include <stdio.h>
#include <stdatomic.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_THREADS 4
#define NUM_ITERATIONS 1000000

typedef struct {
    int x;
    atomic_int y;
} AppObj;

void *thread_func(void *arg) {
    AppObj *obj = (AppObj *)arg;
    for (int i=0; i < NUM_ITERATIONS; i++) {
        obj->x++;
        // atomic_fetch_add(&obj->y, 1);
        obj->y++;
    }
    return NULL;
}

int main() {
    AppObj obj;
    obj.x = 0;
    atomic_init(&obj.y, 0);
    pthread_t threads[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        obj.x = i;
        pthread_create(&threads[i], NULL, (void *(*)(void *))thread_func, &obj);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    if(obj.x == NUM_THREADS * NUM_ITERATIONS) {
        printf("obj.x is correct: %d\n", obj.x);
    } else {
        printf("obj.x is incorrect: %d\n", obj.x);
    }
    if(atomic_load(&obj.y) == NUM_THREADS * NUM_ITERATIONS) {
        printf("obj.y is correct: %d\n", atomic_load(&obj.y));
    } else {
        printf("obj.y is incorrect: %d\n", atomic_load(&obj.y));
    }

    return 0;
}