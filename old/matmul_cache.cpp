#include <bits/stdc++.h>
using namespace std;

#define NUM_ITERATIONS 100
#define MATRIX_Q 16 * 64
#define MATRIX_P 16 * 64
#define MATRIX_R 16 * 64

struct Matrix
{
    int rows;
    int cols;
    int *data;
};

void matrix_multiplication_normal(const Matrix &a, const Matrix &b, Matrix &out)
{
    for (int i = 0; i < a.rows; i++)
    {
        for (int j = 0; j < b.cols; j++)
        {
            out.data[i * out.cols + j] = 0;
            for (int k = 0; k < a.cols; k++)
            {
                out.data[i * out.cols + j] += a.data[i * a.cols + k] * b.data[k * b.cols + j];
            }
        }
    }
}

void matrix_multiplication_blocked(const Matrix &a, const Matrix &b, Matrix &out, int block_size)
{
    for (int i = 0; i < a.rows; i++)
    {
        for (int j = 0; j < b.cols; j++)
        {
            out.data[i * out.cols + j] = 0;
        }
    }
    for (int i = 0; i < a.rows; i += block_size)
    {
        for (int j = 0; j < b.cols; j += block_size)
        {
            for (int k = 0; k < a.cols; k += block_size)
            {
                for (int ii = i; ii < min(i + block_size, a.rows); ii++)
                {
                    for (int jj = j; jj < min(j + block_size, b.cols); jj++)
                    {
                        for (int kk = k; kk < min(k + block_size, a.cols); kk++)
                        {
                            out.data[ii * out.cols + jj] += a.data[ii * a.cols + kk] * b.data[kk * b.cols + jj];
                        }
                    }
                }
            }
        }
    }
}

void matrix_multiplication_blocked_ikj(const Matrix &a, const Matrix &b, Matrix &out, int block_size)
{
    for (int i = 0; i < a.rows; i++)
    {
        for (int j = 0; j < b.cols; j++)
        {
            out.data[i * out.cols + j] = 0;
        }
    }
    for (int ii = 0; ii < a.rows; ii += block_size)
    {
        for (int kk = 0; kk < a.cols; kk += block_size)
        {
            for (int jj = 0; jj < b.cols; jj += block_size)
            {
                int i_end = min(ii + block_size, a.rows);
                int k_end = min(kk + block_size, a.cols);
                int j_end = min(jj + block_size, b.cols);
                for (int i = ii; i < i_end; i++)
                {
                    for (int k = kk; k < k_end; k++)
                    {
                        int a_val = a.data[i * a.cols + k];
                        for (int j = jj; j < j_end; j++)
                        {
                            out.data[i * out.cols + j] += a_val * b.data[k * b.cols + j];
                        }
                    }
                }
            }
        }
    }
}

int main()
{

    // {
    //     Matrix a, b, out;
    //     // Allocate memory for the matrix
    //     a.rows = MATRIX_P;
    //     a.cols = MATRIX_Q;
    //     a.data = new int[a.rows * a.cols];
    //     b.rows = MATRIX_Q;
    //     b.cols = MATRIX_R;
    //     b.data = new int[b.rows * b.cols];
    //     out.rows = MATRIX_P;
    //     out.cols = MATRIX_R;
    //     out.data = new int[out.rows * out.cols];
    //     int sum=0;
    //     uint64_t duration=0;
    //     for (int i = 0; i < NUM_ITERATIONS; ++i) {
    //         // Initialize matrices with random values
    //         for (size_t i = 0; i < MATRIX_P; ++i) {
    //             for (size_t j = 0; j < MATRIX_Q; ++j) {
    //                 a.data[i * a.cols + j] = rand() % 100;
    //             }
    //         }
    //         for (size_t i = 0; i < MATRIX_Q; ++i) {
    //             for (size_t j = 0; j < MATRIX_R; ++j) {
    //                 b.data[i * b.cols + j] = rand() % 100;
    //             }
    //         }
    //         uint64_t start = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count();
    //         matrix_multiplication_normal(a, b, out);
    //         uint64_t end = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count();
    //         duration += end - start;
    //     }

    //     cout << "Time taken for " << NUM_ITERATIONS << " iterations: " << duration << " ms" << " sum: " << sum << endl;
    // }

    // {
    //     Matrix a, b, out;
    //     // Allocate memory for the matrix
    //     a.rows = MATRIX_P;
    //     a.cols = MATRIX_Q;
    //     a.data = new int[MATRIX_P * MATRIX_Q];
    //     b.rows = MATRIX_Q;
    //     b.cols = MATRIX_R;
    //     b.data = new int[MATRIX_Q * MATRIX_R];
    //     out.rows = MATRIX_P;
    //     out.cols = MATRIX_R;
    //     out.data = new int[MATRIX_P * MATRIX_R];
    //     int sum=0;
    //     uint64_t duration=0;
    //     for (int i = 0; i < NUM_ITERATIONS; ++i) {
    //         // Initialize matrices with random values
    //         for (size_t i = 0; i < MATRIX_P; ++i) {
    //             for (size_t j = 0; j < MATRIX_Q; ++j) {
    //                 a.data[i * a.cols + j] = rand() % 100;
    //             }
    //         }
    //         for (size_t i = 0; i < MATRIX_Q; ++i) {
    //             for (size_t j = 0; j < MATRIX_R; ++j) {
    //                 b.data[i * b.cols + j] = rand() % 100;
    //             }
    //         }
    //         uint64_t start = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count();
    //         matrix_multiplication_blocked(a, b, out, 16);
    //         uint64_t end = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count();
    //         duration += end - start;
    //     }

    //     cout << "Time taken for " << NUM_ITERATIONS << " iterations: " << duration << " ms" << " sum: " << sum << endl;
    // }

    for (int bk_size = 8; bk_size <= 1024; bk_size *= 2)
    {
        Matrix a, b, out;
        // Allocate memory for the matrix
        a.rows = MATRIX_P;
        a.cols = MATRIX_Q;
        a.data = new int[MATRIX_P * MATRIX_Q];
        b.rows = MATRIX_Q;
        b.cols = MATRIX_R;
        b.data = new int[MATRIX_Q * MATRIX_R];
        out.rows = MATRIX_P;
        out.cols = MATRIX_R;
        out.data = new int[MATRIX_P * MATRIX_R];
        int sum = 0;
        uint64_t duration = 0;
        for (int i = 0; i < NUM_ITERATIONS; ++i)
        {
            // Initialize matrices with random values
            for (size_t i = 0; i < MATRIX_P; ++i)
            {
                for (size_t j = 0; j < MATRIX_Q; ++j)
                {
                    a.data[i * a.cols + j] = rand() % 100;
                }
            }
            for (size_t i = 0; i < MATRIX_Q; ++i)
            {
                for (size_t j = 0; j < MATRIX_R; ++j)
                {
                    b.data[i * b.cols + j] = rand() % 100;
                }
            }
            uint64_t start = chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count();
            matrix_multiplication_blocked_ikj(a, b, out, bk_size);
            uint64_t end = chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count();
            duration += end - start;
        }

        cout << "Time taken for " << NUM_ITERATIONS << " iterations: " << (1.0 * duration / NUM_ITERATIONS) << " us" << " block_size: " << bk_size << endl;
    }
}