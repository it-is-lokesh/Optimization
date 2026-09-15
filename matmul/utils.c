#define _POSIX_C_SOURCE 199309L
#include "utils.h"
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int *init_mat(int P, int Q, int type) {
    int *A = (int *)malloc(P * Q * sizeof(int));
    switch (type) {
        case ZERO:
            for(int i=0;i<P;i++){
                for(int j=0;j<Q;j++){
                    A[i*Q + j] = 0;
                }
            }
            break;
        case SUM:
            for(int i=0;i<P;i++){
                for(int j=0;j<Q;j++){
                    A[i*Q + j] = i + j;
                }
            }
            break;
        case PRODUCT:
            for(int i=0;i<P;i++){
                for(int j=0;j<Q;j++){
                    A[i*Q + j] = i * j;
                }
            }
            break;
        default:
            printf("Not valid option \n");
            break;
    }
    return A;
}

void print_mat(int P, int Q, int *A) {
    for(int i=0;i<P;i++){
        for(int j=0;j<Q;j++){
            printf("%d ", A[i*Q + j]);
        }
        printf("\n");
    }
}

uint64_t get_time() {
    struct timespec _t;
    clock_gettime(CLOCK_MONOTONIC, &_t);
    uint64_t ret = _t.tv_sec * 1000000000 + _t.tv_nsec;
    return ret;
}

void iterator(int count, void *args, int type) {
    mat_args *obj = (mat_args *)args;
    int P = obj->P;
    int Q = obj->Q;
    int R = obj->R;
    int *A = obj->A;
    int *B = obj->B;
    int *C = obj->C;
    for(int i=0;i<count;i++){
        switch (type) {
            case STANDARD:
                matmul_standard(P, Q, R, A, B, C);
                break;
            case ITERATION_REORDER:
                matmul_iteration_reorder(P, Q, R, A, B, C);
                break;
            case CACHE_TILING:
                matmul_cache_tiling(P, Q, R, A, B, C);
                break;
            case CACHE_TILING_IJK:
                matmul_cache_tiling_ijk(P, Q, R, A, B, C);
                break;
            case MULTI_THREAD:
                matmul_multithread(P, Q, R, A, B, C);
                break;
            case REGISTER_TILING:
                // matmul_register_tiling(P, Q, R, A, B, C);
                matmul_register_tiling_custom(P, Q, R, A, B, C);
                break;
            case REGISTER_TILING_MT:
                matmul_register_tiling_mt(P, Q, R, A, B, C);
                break;
            default:
                printf("Not valid option \n");
                break;
        }
    }
}
