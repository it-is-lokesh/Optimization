#include <chrono>
#include <cstring>
#include <iostream>

using namespace std;

constexpr int N = 1024;

struct Matrix {
    int rows;
    int cols;
    int* data;

    Matrix(int r, int c) {
        rows = r;
        cols = c;

        data = new int[(size_t)rows * cols];
    }

    ~Matrix() { delete[] data; }

    inline int& operator()(int r, int c) { return data[(size_t)r * cols + c]; }
};

void fill_random(Matrix& m) {
    for (size_t i = 0; i < (size_t)m.rows * m.cols; i++)
        m.data[i] = (int)(rand() % 10);
}

void zero_matrix(Matrix& m) {
    memset(m.data, 0, sizeof(int) * (size_t)m.rows * m.cols);
}

void matmul_ijk(const Matrix& A, const Matrix& B, Matrix& C) {
    zero_matrix(C);

    for (int i = 0; i < A.rows; i++) {
        for (int j = 0; j < B.cols; j++) {
            int sum = 0;

            for (int k = 0; k < A.cols; k++) {
                sum += A.data[(size_t)i * A.cols + k] *
                       B.data[(size_t)k * B.cols + j];
            }

            C.data[(size_t)i * C.cols + j] = sum;
        }
    }
}

void matmul_ikj(const Matrix& A, const Matrix& B, Matrix& C) {
    zero_matrix(C);

    for (int i = 0; i < A.rows; i++) {
        for (int k = 0; k < A.cols; k++) {
            int aval = A.data[(size_t)i * A.cols + k];

            const int* brow = &B.data[(size_t)k * B.cols];

            int* crow = &C.data[(size_t)i * C.cols];

            for (int j = 0; j < B.cols; j++) {
                crow[j] += aval * brow[j];
            }
        }
    }
}

constexpr int BLOCK = 64;

void matmul_blocked(const Matrix& A, const Matrix& B, Matrix& C) {
    zero_matrix(C);

    for (int ii = 0; ii < A.rows; ii += BLOCK) {
        for (int kk = 0; kk < A.cols; kk += BLOCK) {
            for (int jj = 0; jj < B.cols; jj += BLOCK) {
                int iend = min(ii + BLOCK, A.rows);

                int kend = min(kk + BLOCK, A.cols);

                int jend = min(jj + BLOCK, B.cols);

                for (int i = ii; i < iend; i++) {
                    for (int k = kk; k < kend; k++) {
                        int aval = A.data[(size_t)i * A.cols + k];

                        const int* bptr = &B.data[(size_t)k * B.cols + jj];

                        int* cptr = &C.data[(size_t)i * C.cols + jj];

                        for (int j = jj; j < jend; j++) {
                            cptr[j - jj] += aval * bptr[j - jj];
                        }
                    }
                }
            }
        }
    }
}

void matmul_register_blocked(const Matrix& A, const Matrix& B, Matrix& C) {
    zero_matrix(C);

    for (int ii = 0; ii < A.rows; ii += BLOCK) {
        for (int kk = 0; kk < A.cols; kk += BLOCK) {
            for (int jj = 0; jj < B.cols; jj += BLOCK) {
                int iend = min(ii + BLOCK, A.rows);

                int kend = min(kk + BLOCK, A.cols);

                int jend = min(jj + BLOCK, B.cols);

                for (int i = ii; i < iend; i += 4) {
                    for (int j = jj; j < jend; j += 4) {
                        int c00 = 0, c01 = 0, c02 = 0, c03 = 0;
                        int c10 = 0, c11 = 0, c12 = 0, c13 = 0;
                        int c20 = 0, c21 = 0, c22 = 0, c23 = 0;
                        int c30 = 0, c31 = 0, c32 = 0, c33 = 0;

                        for (int k = kk; k < kend; k++) {
                            int a0 = A.data[(size_t)(i + 0) * A.cols + k];

                            int a1 = A.data[(size_t)(i + 1) * A.cols + k];

                            int a2 = A.data[(size_t)(i + 2) * A.cols + k];

                            int a3 = A.data[(size_t)(i + 3) * A.cols + k];

                            const int* b = &B.data[(size_t)k * B.cols + j];

                            int b0 = b[0];
                            int b1 = b[1];
                            int b2 = b[2];
                            int b3 = b[3];

                            c00 += a0 * b0;
                            c01 += a0 * b1;
                            c02 += a0 * b2;
                            c03 += a0 * b3;

                            c10 += a1 * b0;
                            c11 += a1 * b1;
                            c12 += a1 * b2;
                            c13 += a1 * b3;

                            c20 += a2 * b0;
                            c21 += a2 * b1;
                            c22 += a2 * b2;
                            c23 += a2 * b3;

                            c30 += a3 * b0;
                            c31 += a3 * b1;
                            c32 += a3 * b2;
                            c33 += a3 * b3;
                        }

                        C(i + 0, j + 0) += c00;
                        C(i + 0, j + 1) += c01;
                        C(i + 0, j + 2) += c02;
                        C(i + 0, j + 3) += c03;

                        C(i + 1, j + 0) += c10;
                        C(i + 1, j + 1) += c11;
                        C(i + 1, j + 2) += c12;
                        C(i + 1, j + 3) += c13;

                        C(i + 2, j + 0) += c20;
                        C(i + 2, j + 1) += c21;
                        C(i + 2, j + 2) += c22;
                        C(i + 2, j + 3) += c23;

                        C(i + 3, j + 0) += c30;
                        C(i + 3, j + 1) += c31;
                        C(i + 3, j + 2) += c32;
                        C(i + 3, j + 3) += c33;
                    }
                }
            }
        }
    }
}

template <typename Func>
void benchmark(const char* name, Func fn, Matrix& A, Matrix& B, Matrix& C) {
    auto start = chrono::high_resolution_clock::now();

    fn(A, B, C);

    auto end = chrono::high_resolution_clock::now();

    auto us = chrono::duration_cast<chrono::microseconds>(end - start).count();

    cout << name << " : " << us / 1000.0 << " ms\n";
}

int main() {
    Matrix A(N, N);
    Matrix B(N, N);
    Matrix C(N, N);

    fill_random(A);
    fill_random(B);

    benchmark("IJK", matmul_ijk, A, B, C);
    benchmark("IKJ", matmul_ikj, A, B, C);
    benchmark("BLOCKED", matmul_blocked, A, B, C);
    benchmark("REGISTER_BLOCKED", matmul_register_blocked, A, B, C);
}
