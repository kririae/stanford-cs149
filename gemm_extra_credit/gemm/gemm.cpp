
// gemm -- general double precision dense matrix-matrix multiplication.
//
// implement: C = alpha * A x B + beta * C, for matrices A, B, C
// Matrix C is M x N  (M rows, N columns)
// Matrix A is M x K
// Matrix B is K x N
//
// Your implementation should make no assumptions about the values contained in
// any input parameters.

#include <math.h>
#include <stdio.h>

#define BLOCKSIZE (6)  // 62 * 62(N) * 8(bytes) \approx = 32KBytes

void __gemm_naive(int m, int n, int k, double *A, double *B, double *C,
                   double alpha, double beta);
void __gemm_blocking(int m, int n, int k, double *A, double *B, double *C,
                   double alpha, double beta);

void gemm(int m, int n, int k, double *A, double *B, double *C,
                double alpha, double beta) {
  // __gemm_naive(m, n, k, A, B, C, alpha, beta);
  __gemm_blocking(m, n, k, A, B, C, alpha, beta);
}

inline static void __gemm_block_L1_mul(int si, int sj, int sk, int m, int n,
                                     int k, double *A, double *B, double *C,
                                     double alpha, double beta) {
  int ei = fmin(si + BLOCKSIZE, m);
  int ej = fmin(sj + BLOCKSIZE, n);
  int ek = fmin(sk + BLOCKSIZE, k);
  // int ei = si + BLOCKSIZE;
  // int ej = sj + BLOCKSIZE;
  // int ek = sk + BLOCKSIZE;
  int i, j, kk;

  for (i = si; i < ei; i++) {
    for (j = sj; j < ej; j++) {
      double inner_prod = 0;
      for (kk = sk; kk < ek; kk++)
        inner_prod += A[i * k + kk] * B[kk * n + j];
      C[i * n + j] += alpha * inner_prod;
    }
  }
}

void __gemm_blocking(int m, int n, int k, double *A, double *B, double *C,
                   double alpha, double beta) {
  int i, j, kk;
  static_assert(BLOCKSIZE % 2 == 0, "BLOCKSIZE % 2 must be 0");
  // printf("running gemm_blocking: (m=%d, n=%d, k=%d)\n", m, n, k);

  for (i = 0; i < m; ++i) {
    for (j = 0; j < n; ++j) {
      C[i * n + j] *= beta;
    }
  }

  for (i = 0; i < m; i += BLOCKSIZE) {
    for (j = 0; j < n; j += BLOCKSIZE) {
      for (kk = 0; kk < k; kk += BLOCKSIZE) {
        __gemm_block_L1_mul(i, j, kk, m, n, k, A, B, C, alpha, beta);
      }
    }
  }
}

void __gemm_naive(int m, int n, int k, double *A, double *B, double *C,
                double alpha, double beta) {
  int i, j, kk;
  // printf("running gemm_naive: (m=%d, n=%d, k=%d)\n", m, n, k);
  for (i = 0; i < m; i++) {
    for (j = 0; j < n; j++) {
      double inner_prod = 0;
      for (kk = 0; kk < k; kk++)
        inner_prod += A[i * k + kk] * B[kk * n + j];
      C[i * n + j] = alpha * inner_prod + beta * C[i * n + j];
    }
  }
}

