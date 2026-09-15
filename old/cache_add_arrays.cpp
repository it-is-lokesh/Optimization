#include <bits/stdc++.h>
using namespace std;

#define NUM_ITERATIONS 100
#define VECTOR_SIZE 16*1024*32
// #define

struct alignas(64) AlignedArray {
    int data[VECTOR_SIZE];
};

AlignedArray result;

void addVectors(int *a, int *b, int *out, size_t size) {
    for (size_t i = 0; i < VECTOR_SIZE; ++i) {
        out[i] = a[i] + b[i];
    }
}

void addVectorsLoopUnrolled(int *a, int *b, int *out, size_t size) {
    for (size_t i = 0; i < VECTOR_SIZE; i += 16) {
        __builtin_prefetch(&a[i + 16], 0, 1);
        out[i] = a[i] + b[i];
        out[i + 1] = a[i + 1] + b[i + 1];
        out[i + 2] = a[i + 2] + b[i + 2];
        out[i + 3] = a[i + 3] + b[i + 3];
        out[i + 4] = a[i + 4] + b[i + 4];
        out[i + 5] = a[i + 5] + b[i + 5];
        out[i + 6] = a[i + 6] + b[i + 6];
        out[i + 7] = a[i + 7] + b[i + 7];
        out[i + 8] = a[i + 8] + b[i + 8];
        out[i + 9] = a[i + 9] + b[i + 9];
        out[i + 10] = a[i + 10] + b[i + 10];
        out[i + 11] = a[i + 11] + b[i + 11];
        out[i + 12] = a[i + 12] + b[i + 12];
        out[i + 13] = a[i + 13] + b[i + 13];
        out[i + 14] = a[i + 14] + b[i + 14];
        out[i + 15] = a[i + 15] + b[i + 15];
    }
}

int main() {
    AlignedArray a, b;

    {
        uint64_t duration=0;
        uint64_t start = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count();
        for (int i = 0; i < NUM_ITERATIONS; ++i) {
            for (size_t i = 0; i < VECTOR_SIZE; ++i) {
                a.data[i] = rand() % 100;
                b.data[i] = rand() % 100;
            }
            addVectors(a.data, b.data, result.data, VECTOR_SIZE);
        }
        uint64_t end = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count();
        duration += end - start;

        cout << "Time taken for " << NUM_ITERATIONS << " iterations: " << duration << " ms" << endl;
    }

    // Print the values of result to verify correctness
    for (size_t i = 0; i < 10; ++i) {
        printf("%4d ", result.data[i]);
    }
    printf("\n");

    {
        uint64_t duration = 0;
        uint64_t start = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count();
        for (int i = 0; i < NUM_ITERATIONS; ++i) {
            for (size_t i = 0; i < VECTOR_SIZE; ++i) {
                a.data[i] = rand() % 100;
                b.data[i] = rand() % 100;
            }
            addVectorsLoopUnrolled(a.data, b.data, result.data, VECTOR_SIZE);
        }
        uint64_t end = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count();
        duration += end - start;

        cout << "Time taken for " << NUM_ITERATIONS << " iterations: " << duration << " ms" << endl;
    }

    // Print the values of result to verify correctness
    for (size_t i = 0; i < 10; ++i) {
        printf("%4d ", result.data[i]);
    }
    printf("\n");
    return 0;
}