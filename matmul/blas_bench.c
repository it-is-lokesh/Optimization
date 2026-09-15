// Standalone BLAS benchmark (float sgemm + double dgemm) for comparison against
// the hand-written kernels in matmul.c.
//
// NOTE: BLAS has no integer GEMM, so this uses float/double. Compare using
// GFLOP/s (printed below), not raw ns, since your kernels are int32.
//
// Requires OpenBLAS + cblas.h. Install one of:
//   sudo apt install libopenblas-dev        (Debian/Ubuntu)
//   conda install -c conda-forge openblas    (then add -I/-L $CONDA_PREFIX)
//
// Build:
//   gcc -O3 -march=native blas_bench.c -o blas_bench -lopenblas -lm
// Run (tune threads for your 12900H P-cores):
//   OMP_NUM_THREADS=12 OPENBLAS_NUM_THREADS=12 ./blas_bench

#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <cblas.h>

static uint64_t now_ns(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (uint64_t)t.tv_sec * 1000000000ull + t.tv_nsec;
}

int main(void) {
    const int N = 1024;              // same size as your matmul
    const int iters = 10;            // same iteration count as run() in main.c
    const double flops = 2.0 * (double)N * N * N;   // 2*N^3 per matmul

    float  *Af = malloc((size_t)N*N*sizeof(float));
    float  *Bf = malloc((size_t)N*N*sizeof(float));
    float  *Cf = malloc((size_t)N*N*sizeof(float));
    double *Ad = malloc((size_t)N*N*sizeof(double));
    double *Bd = malloc((size_t)N*N*sizeof(double));
    double *Cd = malloc((size_t)N*N*sizeof(double));

    for (size_t i = 0; i < (size_t)N*N; i++) {
        Af[i] = (float)(i % 7);  Bf[i] = (float)(i % 3);  Cf[i] = 0.0f;
        Ad[i] = (double)(i % 7); Bd[i] = (double)(i % 3); Cd[i] = 0.0;
    }

    printf("N=%d  iters=%d  BLAS threads=%s\n\n",
           N, iters, getenv("OPENBLAS_NUM_THREADS") ? getenv("OPENBLAS_NUM_THREADS") : "default");

    // ---- warm-up (spins up OpenBLAS thread pool, pages memory) ----
    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
                N, N, N, 1.0f, Af, N, Bf, N, 0.0f, Cf, N);

    // ---- SGEMM (float) ----
    uint64_t t0 = now_ns();
    for (int it = 0; it < iters; it++)
        cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
                    N, N, N, 1.0f, Af, N, Bf, N, 1.0f, Cf, N);
    uint64_t t1 = now_ns();

    // ---- DGEMM (double) ----
    uint64_t t2 = now_ns();
    for (int it = 0; it < iters; it++)
        cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans,
                    N, N, N, 1.0, Ad, N, Bd, N, 1.0, Cd, N);
    uint64_t t3 = now_ns();

    double sgemm_total = (t1 - t0);
    double dgemm_total = (t3 - t2);
    double sgemm_each  = sgemm_total / iters;
    double dgemm_each  = dgemm_total / iters;

    printf("%-14s total: %12.0f ns   per-iter: %11.0f ns   %8.2f GFLOP/s\n",
           "openblas_sgemm", sgemm_total, sgemm_each, flops / sgemm_each);
    printf("%-14s total: %12.0f ns   per-iter: %11.0f ns   %8.2f GFLOP/s\n",
           "openblas_dgemm", dgemm_total, dgemm_each, flops / dgemm_each);

    printf("\nCompare your kernels: GFLOP/s = %.3g / (your_ns_for_10_iters / 10)\n", flops);

    free(Af); free(Bf); free(Cf); free(Ad); free(Bd); free(Cd);
    return 0;
}
