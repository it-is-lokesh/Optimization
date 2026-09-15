// sb_test.c: store buffering test
// gcc -O3 -pthread sb_test.c -o sb_test

#include <stdio.h>
#include <pthread.h>

volatile int start;

int x, y;
int r1, r2;

void *t1(void *arg)
{
    while(!start){}

    x = 1;
    // r1 = y;
    if (y == 0) r1 = 1;

    return NULL;
}

void *t2(void *arg)
{
    while(!start){}

    y = 1;
    // r2 = x;
    if (x == 0) r2 = 1;

    return NULL;
}

int main()
{
    long long count = 0;

    while(1)
    {
        pthread_t a,b;

        x = y = 0;
        r1 = r2 = 0;
        start = 0;

        pthread_create(&a,NULL,t1,NULL);
        pthread_create(&b,NULL,t2,NULL);

        start = 1;

        pthread_join(a,NULL);
        pthread_join(b,NULL);

        count++;

        if(r1 == 1 && r2 == 1)
        {
            printf("Observed after %lld iterations\n",count);
            break;
        }
    }
}
