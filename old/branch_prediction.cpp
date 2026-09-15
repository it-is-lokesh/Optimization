#include <bits/stdc++.h>
using namespace std;

#define NUM_ITERATIONS 100
#define VECTOR_SIZE 16*1024*32
// #define

struct alignas(64) AlignedArray {
    int data[VECTOR_SIZE];
};

AlignedArray result;


void PredictableBranch(int *a, int *b, int *c, int *out, size_t size) {
    for(int i=0;i<VECTOR_SIZE;i++){
        if(i%2==0){
            out[i] = a[i] + b[i];
        }else{
            out[i] = a[i] - b[i];
        }
    }
}

void UnpredictableBranch(int *a, int *b, int *c, int *out, size_t size) {
    for(int i=0;i<VECTOR_SIZE;i++){
        if(c[i]&1){
            out[i] = a[i] + b[i];
        }else{
            out[i] = a[i] - b[i];
        }
    }
}

void Branchless(int *a, int *b, int *c, int *out, size_t size) {
    // for(int i=0;i<VECTOR_SIZE;i++){
    //     bool temp = rand()&1;
    //     out[i] = temp * (a[i] + b[i]) + (!temp) * (a[i] - b[i]);
    // }

    int sign[2] = {1, -1};

    for(int i=0;i<VECTOR_SIZE;i++){
        int temp = c[i]&1;
        out[i] = a[i] + sign[temp] * b[i];
    }
}

int main() {
    AlignedArray a, b, c;

    {
        uint64_t duration=0;
        for (int i = 0; i < NUM_ITERATIONS; ++i) {
            for (size_t i = 0; i < VECTOR_SIZE; ++i) {
                a.data[i] = rand() % 100;
                b.data[i] = rand() % 100;
                c.data[i] = rand() % 100;
            }
            uint64_t start = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count();
            PredictableBranch(a.data, b.data, c.data, result.data, VECTOR_SIZE);
            uint64_t end = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count();
            duration += end - start;
        }

        cout << "Time taken for " << NUM_ITERATIONS << " iterations: " << duration << " ms" << endl;
    }

    // Print the values of result to verify correctness
    for (size_t i = 0; i < 10; ++i) {
        printf("%4d ", result.data[i]);
    }
    printf("\n");

    {
        uint64_t duration = 0;
        for (int i = 0; i < NUM_ITERATIONS; ++i) {
            for (size_t i = 0; i < VECTOR_SIZE; ++i) {
                a.data[i] = rand() % 100;
                b.data[i] = rand() % 100;
                c.data[i] = rand() % 100;
            }
            uint64_t start = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count();
            UnpredictableBranch(a.data, b.data, c.data, result.data, VECTOR_SIZE);
            uint64_t end = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count();
            duration += end - start;
        }

        cout << "Time taken for " << NUM_ITERATIONS << " iterations: " << duration << " ms" << endl;
    }

    // Print the values of result to verify correctness
    for (size_t i = 0; i < 10; ++i) {
        printf("%4d ", result.data[i]);
    }

    printf("\n");

    {
        uint64_t duration = 0;
        for (int i = 0; i < NUM_ITERATIONS; ++i) {
            for (size_t i = 0; i < VECTOR_SIZE; ++i) {
                a.data[i] = rand() % 100;
                b.data[i] = rand() % 100;
                c.data[i] = rand() % 100;
            }
            uint64_t start = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count();
            Branchless(a.data, b.data, c.data, result.data, VECTOR_SIZE);
            uint64_t end = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count();
            duration += end - start;
        }

        cout << "Time taken for " << NUM_ITERATIONS << " iterations: " << duration << " ms" << endl;
    }

    // Print the values of result to verify correctness
    for (size_t i = 0; i < 10; ++i) {
        printf("%4d ", result.data[i]);
    }
    printf("\n");
    return 0;
}
