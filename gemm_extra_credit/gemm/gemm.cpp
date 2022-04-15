
// gemm -- general double precision dense matrix-matrix multiplication.
//
// implement: C = alpha * A x B + beta * C, for matrices A, B, C
// Matrix C is M x N  (M rows, N columns)
// Matrix A is M x K
// Matrix B is K x N
//
// Your implementation should make no assumptions about the values contained in
// any input parameters.

#include <assert.h>
#include <math.h>
#include <stdio.h>

#define BLOCKSIZE (6)  // 62 * 62(N) * 8(bytes) \approx = 32KBytes

void __gemm_naive(int m, int n, int k, double *A, double *B, double *C,
                  double alpha, double beta);
void __gemm_blocking(int m, int n, int k, double *A, double *B, double *C,
                     double alpha, double beta);

void gemm(int m, int n, int k, double *A, double *B, double *C, double alpha,
          double beta) {
  // __gemm_naive(m, n, k, A, B, C, alpha, beta);
  __gemm_blocking(m, n, k, A, B, C, alpha, beta);
}

__attribute__((always_inline)) size_t __get_idx(int m, int n, int nc) {
  return m * nc + n;
}

__attribute__((always_inline)) static void __gemm_block_L2_mul_naive(
    int si, int sj, int sk, int m, int n, int k, double *A, double *B,
    double *C, double alpha, double beta) {
  // Matrix C is M x N  (M rows, N columns)
  // Matrix A is M x K
  // Matrix B is K x N

  // starting from (si, sj, sk), mul 2*2 matrix through vectorization
  // add the result to C
  // A: starting from A[si, sk]
  // B: startingf rom B[sk, sj]
  // C: starting from C[si, sj]
  // an 2 x 2 matrix

  // Will be optimized out
  register double A11, A12, A21, A22, B11, B12, B21, B22;
  A11 = A[__get_idx(si, sk, k)];
  A12 = A[__get_idx(si, sk + 1, k)];
  A21 = A[__get_idx(si + 1, sk, k)];
  A22 = A[__get_idx(si + 1, sk + 1, k)];
  B11 = B[__get_idx(sk, sj, n)];
  B12 = B[__get_idx(sk, sj + 1, n)];
  B21 = B[__get_idx(sk + 1, sj, n)];
  B22 = B[__get_idx(sk + 1, sj + 1, n)];

  C[__get_idx(si, sj, n)] += alpha * (A11 * B11 + A12 * B21);
  C[__get_idx(si, sj + 1, n)] += alpha * (A11 * B12 + A12 * B22);
  C[__get_idx(si + 1, sj, n)] += alpha * (A21 * B11 + A22 * B21);
  C[__get_idx(si + 1, sj + 1, n)] += alpha * (A21 * B12 + A22 * B22);
}

__attribute__((always_inline)) static void __gemm_block_L4_mul(
    int si, int sj, int sk, int m, int n, int k, double *A, double *B,
    double *C, double alpha, double beta) {
  // Matrix C is M x N  (M rows, N columns)
  // Matrix A is M x K
  // Matrix B is K x N

  // starting from (si, sj, sk), mul 2*2 matrix through vectorization
  // add the result to C
  // A: starting from A[si, sk]
  // B: startingf rom B[sk, sj]
  // C: starting from C[si, sj]
  // an 2 x 2 matrix

  int i, j, kk;
  // printf("running gemm_naive: (m=%d, n=%d, k=%d)\n", m, n, k);
  for (i = 0; i < m; i++) {
    for (j = 0; j < n; j++) {
      double inner_prod = 0;
      for (kk = 0; kk < k; kk++) inner_prod += A[i * k + kk] * B[kk * n + j];
      C[i * n + j] = alpha * inner_prod + beta * C[i * n + j];
    }
  }
}

inline static void __gemm_block_L1_mul(int si, int sj, int sk, int m, int n,
                                       int k, double *A, double *B, double *C,
                                       double alpha, double beta) {
#define MIN_BLOCK 1
  int ei = fmin(si + BLOCKSIZE, m);
  int ej = fmin(sj + BLOCKSIZE, n);
  int ek = fmin(sk + BLOCKSIZE, k);
  int i, j, kk;

  assert(m % MIN_BLOCK == 0);
  assert(n % MIN_BLOCK == 0);
  assert(k % MIN_BLOCK == 0);

#if MIN_BLOCK == 1
  for (i = si; i < ei; i++) {
    for (j = sj; j < ej; j++) {
      double inner_prod = 0;
      for (kk = sk; kk < ek; kk++) inner_prod += A[i * k + kk] * B[kk * n + j];
      C[i * n + j] += alpha * inner_prod;
    }
  }
  return;
#elif MIN_BLOCK == 2
  for (i = si; i < ei; i += MIN_BLOCK)
    for (j = sj; j < ej; j += MIN_BLOCK)
      for (kk = sk; kk < ek; kk += MIN_BLOCK)
        __gemm_block_L2_mul_naive(i, j, kk, m, n, k, A, B, C, alpha, beta);
  return;
#elif MIN_BLOCK == 4
  for (i = si; i < ei; i += MIN_BLOCK)
    for (j = sj; j < ej; j += MIN_BLOCK)
      for (kk = sk; kk < ek; kk += MIN_BLOCK)
        __gemm_block_L4_mul(i, j, kk, m, n, k, A, B, C, alpha, beta);
  return;
#else
  static_assert("MIN_BLOCK size unsupported");
  return;
#endif
}

void __gemm_blocking(int m, int n, int k, double *A, double *B, double *C,
                     double alpha, double beta) {
  int i, j, kk;
  static_assert(BLOCKSIZE % 2 == 0, "BLOCKSIZE % 2 must be 0");
  // printf("running gemm_blocking: (m=%d, n=%d, k=%d)\n", m, n, k);

  assert(m % 4 == 0);
  assert(n % 4 == 0);
  assert(k % 4 == 0);

  for (i = 0; i < m; ++i) {
    for (j = 0; j < n; ++j) {
      C[i * n + j] *= beta;
    }
  }

  for (kk = 0; kk < k; kk += BLOCKSIZE) {
#pragma omp parallel for schedule(static)
    for (i = 0; i < m; i += BLOCKSIZE) {
      for (j = 0; j < n; j += BLOCKSIZE) {
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
      for (kk = 0; kk < k; kk++) inner_prod += A[i * k + kk] * B[kk * n + j];
      C[i * n + j] = alpha * inner_prod + beta * C[i * n + j];
    }
  }
}
