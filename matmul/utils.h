#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>

// #define restrict 

enum mat_init_type {
    SUM,
    PRODUCT,
    ZERO
};

enum mat_mul_type {
    STANDARD,
    ITERATION_REORDER,
    CACHE_TILING,
    CACHE_TILING_IJK,
    MULTI_THREAD,
    REGISTER_TILING,
    REGISTER_TILING_MT
};

typedef struct {
    int P;
    int Q;
    int R;
    int *A;
    int *B;
    int *C;
} mat_args;

int *init_mat(int P, int Q, int type);

void print_mat(int P, int Q, int *A);

uint64_t get_time();

void iterator(int count, void *args, int type);

void matmul_standard(int P, int Q, int R, int *restrict A, int *restrict B, int *restrict C);

void matmul_iteration_reorder(int P, int Q, int R, int *restrict A, int *restrict B, int *restrict C);

void matmul_cache_tiling(int P, int Q, int R, int *restrict A, int *restrict B, int *restrict C);

void matmul_cache_tiling_ijk(int P, int Q, int R, int *restrict A, int *restrict B, int *restrict C);

void matmul_multithread(int P, int Q, int R, int *restrict A, int *restrict B, int *restrict C);

void matmul_register_tiling(int P, int Q, int R, int *restrict A, int *restrict B, int *restrict C);

void matmul_register_tiling_custom(int P, int Q, int R, int *restrict A, int *restrict B, int *restrict C);

void matmul_register_tiling_mt(int P, int Q, int R, int *restrict A, int *restrict B, int *restrict C);

#endif // UTILS_H