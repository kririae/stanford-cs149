#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <algorithm>
#include <fstream>
#include <iostream>

#include "CycleTimer.h"

#undef MKL_INSTALLED
#if MKL_INSTALLED
#include "mkl.h"
#endif

#include "gemm_ispc.h"
#include "ref_gemm_ispc.h"

#define N_ITERS 1 // how many times to run implementaions for timing

// implement: C = alpha * A x B + beta * C
extern void gemm(int m, int n, int k, double *A, double *B, double *C,
                 double alpha, double beta);

static float toBW(uint64_t bytes, float sec) {
  return static_cast<float>(bytes) / (1024. * 1024. * 1024.) / sec;
}

static float toGFLOPS(uint64_t ops, float sec) {
  return static_cast<float>(ops) / 1e9 / sec;
}

// Useful if you want to print a whole matrix
void printMat(const char *name, double *A, int m, int n) {
  printf("--- %s ---\n", name);
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      printf("%.2lf ", A[i * n + j]);
    }
    printf("\n");
  }
  printf("\n");
  return;
}

// Allocate and populate matrices
int allocMatrices(int m, int n, int k, double **A, double **B, double **C) {
#if MKL_INSTALLED
  // mkl_malloc aligns allocated memory on 64-byte boundaries for performance
  *A = (double *)mkl_malloc(m * k * sizeof(double), 64);
  *B = (double *)mkl_malloc(k * n * sizeof(double), 64);
  *C = (double *)mkl_malloc(m * n * sizeof(double), 64);
  if (*A == NULL || *B == NULL || *C == NULL) {
    // Could not allocate memory; abort
    return 1;
  }
#else
  *A = (double *)malloc(m * k * sizeof(double));
  *B = (double *)malloc(k * n * sizeof(double));
  *C = (double *)malloc(m * n * sizeof(double));
  if (*A == NULL || *B == NULL || *C == NULL) {
    return 1;
  }
#endif
  return 0;
}

void fillMatrices(int m, int n, int k, double **A, double **B, double **C) {
  // Populate the matrices with some data
  int i;
  for (i = 0; i < (m * k); i++) {
    (*A)[i] = ((double)rand() / (double)RAND_MAX);
  }

  for (i = 0; i < (k * n); i++) {
    (*B)[i] = ((double)rand() / (double)RAND_MAX);
  }

  for (i = 0; i < (m * n); i++) {
    (*C)[i] = ((double)rand() / (double)RAND_MAX);
  }
}

// Compute C=alpha*A*B+beta*C using Intel MKL and your implementation
int main(int argc, char *argv[]) {
  // Problem size calculations
  int m, n, k;
  int size = atoi(argv[1]);
  m = size, k = size, n = size;
  const uint64_t TOTAL_BYTES = (m * k + k * n + 2 * m * n) * sizeof(double);
  const uint64_t TOTAL_FLOPS = 2 * m * ((uint64_t)n * k);

  double alpha, beta;
  alpha = 1.0;
  beta = 1.0;

  // Prepare matrices
  double *A1, *B1, *C1; // for MKL library implementation

  if (allocMatrices(m, n, k, &A1, &B1, &C1) == 1) {
    return 1;
  }

  double startTime, endTime;

  // Fill input matrices with random data
  fillMatrices(m, n, k, &A1, &B1, &C1);

  // Run your matrix multiply implementation.
  printf("Running student GEMM... ");
  startTime = CycleTimer::currentSeconds();
  // ispc::gemm_ispc(m, n, k, A2, B2, C2, alpha, beta);
  gemm(m, n, k, A1, B1, C1, alpha, beta);
  endTime = CycleTimer::currentSeconds();
  printf("%.2lfms\n", (endTime - startTime) * 1000);
  double minGEMM = endTime - startTime;

  printf("[Student GEMM]:\t\t[%.3f] ms\t[%.3f] GB/s\t[%.2f] GFLOPS\n",
         minGEMM * 1000, toBW(TOTAL_BYTES, minGEMM),
         toGFLOPS(TOTAL_FLOPS, minGEMM));

  free(A1);
  free(B1);
  free(C1);

  return 0;
}
