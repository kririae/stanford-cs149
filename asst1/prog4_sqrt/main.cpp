#include <stdio.h>
#include <algorithm>
#include <pthread.h>
#include <math.h>
#include <assert.h>

#include "CycleTimer.h"
#include "sqrt_ispc.h"

using namespace ispc;

extern void sqrtSerial(int N, float startGuess, float* values, float* output);
extern void sqrtIntrinsics(int N, float initialGuess, float values[],
                           float output[]);

static void verifyResult(int N, float* result, float* gold) {
  for (int i = 0; i < N; i++) {
    if (fabs(result[i] - gold[i]) > 1e-4) {
      printf("Error: [%d] Got %f expected %f\n", i, result[i], gold[i]);
    }
  }
}

int main() {
  const unsigned int N            = 20 * 1000 * 1000;
  const float        initialGuess = 1.0f;

  // float* values = new float[N];
  const unsigned int _N = N + N % 8;
  float* values = (float*)aligned_alloc(8 * sizeof(float), _N * sizeof(float));
  float* output = (float*)aligned_alloc(8 * sizeof(float), _N * sizeof(float));
  float* gold   = (float*)aligned_alloc(8 * sizeof(float), _N * sizeof(float));

  // Random input
  for (unsigned int i = 0; i < N; i++) {
    // TODO: CS149 students.  Attempt to change the values in the
    // array here to meet the instructions in the handout: we want
    // to you generate best and worse-case speedups

    // starter code populates array with random input values
    values[i] = .001f + 2.998f * static_cast<float>(rand()) / RAND_MAX;
  }

  // Maximize speed up
  for (unsigned int i = 0; i < N; i++) {
    values[i] = 2.998f;
  }

  // // Minimize speed up
  // for (unsigned int i = 0; i < N; i++) {
  //   values[i] = i % 8 == 0 ? 2.998f : 1.0f;
  // }

  // generate a gold version to check results
  for (unsigned int i = 0; i < N; i++) gold[i] = sqrt(values[i]);

  //
  // And run the serial implementation 3 times, again reporting the
  // minimum time.
  //

  double minSerial = 1e30;
  for (int i = 0; i < 3; ++i) {
    double startTime = CycleTimer::currentSeconds();
    sqrtSerial(N, initialGuess, values, output);
    double endTime = CycleTimer::currentSeconds();
    minSerial      = std::min(minSerial, endTime - startTime);
  }

  printf("[sqrt serial]:\t\t[%.3f] ms\n", minSerial * 1000);

  verifyResult(N, output, gold);

  //
  // Compute the image using the ispc implementation; report the minimum
  // time of three runs.
  //
  double minISPC = 1e30;
  for (int i = 0; i < 3; ++i) {
    double startTime = CycleTimer::currentSeconds();
    sqrt_ispc(N, initialGuess, values, output);
    double endTime = CycleTimer::currentSeconds();
    minISPC        = std::min(minISPC, endTime - startTime);
  }

  printf("[sqrt ispc]:\t\t[%.3f] ms\n", minISPC * 1000);

  verifyResult(N, output, gold);

  // Clear out the buffer
  // for (unsigned int i = 0; i < N; ++i) output[i] = 0;
  memset(output, 0, _N * sizeof(float));

  double minIntrinsics = 1e30;
  for (int i = 0; i < 3; ++i) {
    double startTime = CycleTimer::currentSeconds();
    sqrtIntrinsics(N, initialGuess, values, output);
    double endTime = CycleTimer::currentSeconds();
    minIntrinsics  = std::min(minIntrinsics, endTime - startTime);
  }

  printf("[sqrt intrinsics]:\t[%.3f] ms\n", minIntrinsics * 1000);

  verifyResult(N, output, gold);

  printf("\t\t\t\t(%.2fx speedup from hand written Intrinsics)\n",
         minSerial / minIntrinsics);

  //
  // Tasking version of the ISPC code
  //
  double minTaskISPC = 1e30;
  for (int i = 0; i < 3; ++i) {
    double startTime = CycleTimer::currentSeconds();
    sqrt_ispc_withtasks(N, initialGuess, values, output);
    double endTime = CycleTimer::currentSeconds();
    minTaskISPC    = std::min(minTaskISPC, endTime - startTime);
  }

  printf("[sqrt task ispc]:\t[%.3f] ms\n", minTaskISPC * 1000);

  verifyResult(N, output, gold);

  printf("\t\t\t\t(%.2fx speedup from ISPC)\n", minSerial / minISPC);
  printf("\t\t\t\t(%.2fx speedup from task ISPC)\n", minSerial / minTaskISPC);

  free(values);
  free(output);
  free(gold);

  return 0;
}
