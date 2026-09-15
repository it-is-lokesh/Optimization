#include <stdio.h>

#define BLOCK_SIZE 32
#define MR 4   // register-tile rows
#define NR 4   // register-tile cols

void matmul_standard(int P, int Q, int R, int *restrict A, int *restrict B, int *restrict C) {
    for(int i=0;i<P;i++){
        for(int j=0;j<R;j++){
            for(int k=0;k<Q;k++){
                C[i*R + j] += A[i*Q + k] * B[k*R + j];
            }
        }
    }
}

void matmul_iteration_reorder(int P, int Q, int R, int *restrict A, int *restrict B, int *restrict C) {
    for(int i=0;i<P;i++){
        for(int k=0;k<Q;k++){
            int aik = A[i*Q + k];
            for(int j=0;j<R;j++){
                C[i*R + j] += aik * B[k*R + j];
            }
        }
    }
}

void matmul_cache_tiling(int P, int Q, int R, int *restrict A, int *restrict B, int *restrict C) {
    for(int ii=0;ii<P;ii+=BLOCK_SIZE) {
        for(int kk=0;kk<Q;kk+=BLOCK_SIZE) {
            for(int jj=0;jj<R;jj+=BLOCK_SIZE) {
                int i_end = (ii + BLOCK_SIZE > P) ? P : ii + BLOCK_SIZE;
                int k_end = (kk + BLOCK_SIZE > Q) ? Q : kk + BLOCK_SIZE;
                int j_end = (jj + BLOCK_SIZE > R) ? R : jj + BLOCK_SIZE;

                for(int i=ii;i<i_end;i++){
                    for(int k=kk;k<k_end;k++){
                        for(int j=jj;j<j_end;j++){
                            C[i*R + j] += A[i*Q + k] * B[k*R + j];
                        }
                    }
                }
            }
        }
    }
}

// Cache tiling applied to the ijk loop order. Here B is accessed column-wise
// (stride R), so without blocking every B access is a cache miss. Blocking the
// k/j dimensions keeps a BLOCK_SIZE x BLOCK_SIZE tile of B resident in cache and
// reuses it across the i-block, which is where tiling actually pays off.
void matmul_cache_tiling_ijk(int P, int Q, int R, int *restrict A, int *restrict B, int *restrict C) {
    for(int ii=0;ii<P;ii+=BLOCK_SIZE) {
        for(int jj=0;jj<R;jj+=BLOCK_SIZE) {
            for(int kk=0;kk<Q;kk+=BLOCK_SIZE) {
                int i_end = (ii + BLOCK_SIZE > P) ? P : ii + BLOCK_SIZE;
                int j_end = (jj + BLOCK_SIZE > R) ? R : jj + BLOCK_SIZE;
                int k_end = (kk + BLOCK_SIZE > Q) ? Q : kk + BLOCK_SIZE;

                for(int i=ii;i<i_end;i++){
                    for(int j=jj;j<j_end;j++){
                        int acc = C[i*R + j];
                        for(int k=kk;k<k_end;k++){
                            acc += A[i*Q + k] * B[k*R + j];
                        }
                        C[i*R + j] = acc;
                    }
                }
            }
        }
    }
}

// Multithreaded matmul (OpenMP). Uses the fast ikj order and parallelizes the
// outer i loop: each thread owns a distinct set of rows of C, so there are no
// write conflicts and no synchronization is needed. Compile with -fopenmp;
// without it the pragma is ignored and this runs single-threaded.
void matmul_multithread(int P, int Q, int R, int *restrict A, int *restrict B, int *restrict C) {
    #pragma omp parallel for schedule(static)
    for(int i=0;i<P;i++){
        for(int k=0;k<Q;k++){
            int aik = A[i*Q + k];
            for(int j=0;j<R;j++){
                C[i*R + j] += aik * B[k*R + j];
                // C[i*R + j] += A[i*Q + k] * B[k*R + j];
            }
        }
    }
}

// Register tiling (register blocking). Computes an MR x NR micro-tile of C in
// local accumulators that the compiler keeps in registers, while streaming
// across k. Each A/B value loaded from cache is reused NR/MR times straight
// from registers, and C is written only once per micro-tile instead of once
// per k. This is the last-level optimization on top of loop reorder + tiling.
void matmul_register_tiling(int P, int Q, int R, int *restrict A, int *restrict B, int *restrict C) {
    int i = 0;
    for (; i + MR <= P; i += MR) {
        int j = 0;
        for (; j + NR <= R; j += NR) {
            int c[MR][NR] = {{0}};
            for (int k = 0; k < Q; k++) {
                int a0 = A[(i+0)*Q + k];
                int a1 = A[(i+1)*Q + k];
                int a2 = A[(i+2)*Q + k];
                int a3 = A[(i+3)*Q + k];
                int b0 = B[k*R + j+0];
                int b1 = B[k*R + j+1];
                int b2 = B[k*R + j+2];
                int b3 = B[k*R + j+3];
                c[0][0]+=a0*b0; c[0][1]+=a0*b1; c[0][2]+=a0*b2; c[0][3]+=a0*b3;
                c[1][0]+=a1*b0; c[1][1]+=a1*b1; c[1][2]+=a1*b2; c[1][3]+=a1*b3;
                c[2][0]+=a2*b0; c[2][1]+=a2*b1; c[2][2]+=a2*b2; c[2][3]+=a2*b3;
                c[3][0]+=a3*b0; c[3][1]+=a3*b1; c[3][2]+=a3*b2; c[3][3]+=a3*b3;
            }
            for (int mi = 0; mi < MR; mi++)
                for (int ni = 0; ni < NR; ni++)
                    C[(i+mi)*R + (j+ni)] += c[mi][ni];
        }
        // leftover columns (R not a multiple of NR)
        for (; j < R; j++) {
            for (int mi = 0; mi < MR; mi++) {
                int acc = 0;
                for (int k = 0; k < Q; k++)
                    acc += A[(i+mi)*Q + k] * B[k*R + j];
                C[(i+mi)*R + j] += acc;
            }
        }
    }
    // leftover rows (P not a multiple of MR)
    for (; i < P; i++) {
        for (int j = 0; j < R; j++) {
            int acc = 0;
            for (int k = 0; k < Q; k++)
                acc += A[i*Q + k] * B[k*R + j];
            C[i*R + j] += acc;
        }
    }
}

void matmul_register_tiling_custom(int P, int Q, int R, int *restrict A, int *restrict B, int *restrict C) {
    int i = 0;
    for (; i+MR<P; i+=MR) {
        int j=0;
        for (; j+NR<R; j+=NR) {
            int c[MR][NR] = {{0}};
            for (int k = 0; k<Q; k++) {
                for (int ii = 0; ii<MR; ii++) {
                    int *Brow;
                    Brow = &B[k*R + j];
                    int a = A[(i+ii)*Q + k];
                    for (int jj = 0; jj<NR; jj++) {
                        c[ii][jj] += a*Brow[jj];
                    }
                }
            }
            for (int mi = 0; mi<MR; mi++) {
                for (int ni = 0; ni<NR; ni++) {
                    C[(i+mi)*R + (j+ni)] += c[mi][ni];
                }
            }
        }
        for(; j<R; j++) {
            for (int mi = 0; mi<MR; mi++) {
                int acc=0;
                for (int k = 0; k<Q; k++) {
                    acc += A[(i+mi)*Q + k] * B[k*R + j];
                }
                C[(i+mi)*R + j] += acc;
            }
        }
    }
    for (; i<P; i++) {
        for (int j=0; j<R; j++) {
            int acc=0;
            for (int k=0; k<Q; k++) {
                acc += A[i*Q + k] * B[k*R + j];
            }
            C[i*R + j] += acc;
        }
    }
}

// Register tiling + multithreading. Same MR x NR register micro-kernel as
// matmul_register_tiling, but the outer loop over row-blocks is parallelized
// with OpenMP. Each row-block (rows i..i+MR-1) is owned by one thread, so the
// distinct C rows are written without conflicts. This stacks the register-reuse
// speedup with core-level parallelism. Compile with -fopenmp.
void matmul_register_tiling_mt(int P, int Q, int R, int *restrict A, int *restrict B, int *restrict C) {
    int nblocks = P / MR;
    #pragma omp parallel for schedule(static)
    for (int ib = 0; ib < nblocks; ib++) {
        int i = ib * MR;
        int j = 0;
        for (; j + NR <= R; j += NR) {
            int c[MR][NR] = {{0}};
            for (int k = 0; k < Q; k++) {
                int a0 = A[(i+0)*Q + k];
                int a1 = A[(i+1)*Q + k];
                int a2 = A[(i+2)*Q + k];
                int a3 = A[(i+3)*Q + k];
                int b0 = B[k*R + j+0];
                int b1 = B[k*R + j+1];
                int b2 = B[k*R + j+2];
                int b3 = B[k*R + j+3];
                c[0][0]+=a0*b0; c[0][1]+=a0*b1; c[0][2]+=a0*b2; c[0][3]+=a0*b3;
                c[1][0]+=a1*b0; c[1][1]+=a1*b1; c[1][2]+=a1*b2; c[1][3]+=a1*b3;
                c[2][0]+=a2*b0; c[2][1]+=a2*b1; c[2][2]+=a2*b2; c[2][3]+=a2*b3;
                c[3][0]+=a3*b0; c[3][1]+=a3*b1; c[3][2]+=a3*b2; c[3][3]+=a3*b3;
            }
            for (int mi = 0; mi < MR; mi++)
                for (int ni = 0; ni < NR; ni++)
                    C[(i+mi)*R + (j+ni)] += c[mi][ni];
        }
        // leftover columns (R not a multiple of NR)
        for (; j < R; j++) {
            for (int mi = 0; mi < MR; mi++) {
                int acc = 0;
                for (int k = 0; k < Q; k++)
                    acc += A[(i+mi)*Q + k] * B[k*R + j];
                C[(i+mi)*R + j] += acc;
            }
        }
    }
    // leftover rows (P not a multiple of MR)
    for (int i = nblocks * MR; i < P; i++) {
        for (int j = 0; j < R; j++) {
            int acc = 0;
            for (int k = 0; k < Q; k++)
                acc += A[i*Q + k] * B[k*R + j];
            C[i*R + j] += acc;
        }
    }
}
