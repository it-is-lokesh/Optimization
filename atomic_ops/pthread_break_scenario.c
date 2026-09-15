#include <stdio.h>
#include <stdint.h>

void actual_function(double x)
{
    printf("actual_function received: %f\n", x);
}

int main(void)
{
    long val=123456;
    // Pretend actual_function has this completely different signature:
    void (*wrong_type)(int) = (void (*)(int))actual_function;

    wrong_type(val);
    printf("val in f: %f \n", *(double *)&val);

    return 0;
}