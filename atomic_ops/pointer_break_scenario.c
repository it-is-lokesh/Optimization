#include <stdio.h>

int func(double *x) {
    printf("func received: %f\n", *x);
}

int main() {
    int val = 123.234;
    func(&val);
    return 0;
}
