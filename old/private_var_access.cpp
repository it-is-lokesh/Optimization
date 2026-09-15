#include <iostream>
using namespace std;

class A {
private:
    int x;
    virtual int func() { return x * 2; }
    virtual int func1() { return y * 2; }
    virtual int func2() { return x*y * 2; }
    int y;
public:
    A() : x(10), y(20) {}
    int *getXPtr() { return &x; }
    int *getYPtr() { return &y; }
};

int main() {
    A a;
    // Print size of A
    cout << "Size of A: " << sizeof(A) << endl;
    int *xPtr = a.getXPtr();
    int *yPtr = a.getYPtr();
    uint64_t xOffset = (uint64_t)xPtr - (uint64_t)&a;
    uint64_t yOffset = (uint64_t)yPtr - (uint64_t)&a;
    cout << "Offset of x: " << xOffset << endl;
    cout << "Offset of y: " << yOffset << endl;
    // Print values of x and y using offsets
    int *xPtr2 = (int *)((uint64_t)&a + xOffset);
    int *yPtr2 = (int *)((uint64_t)&a + yOffset);
    cout << "Value of x: " << *xPtr2 << endl;
    cout << "Value of y: " << *yPtr2 << endl;
    return 0;
}