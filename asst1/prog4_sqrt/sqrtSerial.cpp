#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <immintrin.h>

void sqrtSerial(int N, float initialGuess, float values[], float output[]) {
  static const float kThreshold = 0.00001f;

  for (int i = 0; i < N; i++) {
    float x     = values[i];
    float guess = initialGuess;

    float error = fabs(guess * guess * x - 1.f);

    while (error > kThreshold) {
      guess = (3.f * guess - x * guess * guess * guess) * 0.5f;
      error = fabs(guess * guess * x - 1.f);
    }

    output[i] = x * guess;
  }
}

void _mm256_log(__m256 x) {
  float out[8];

  _mm256_store_ps(out, x);
  for (int i = 0; i < 8; ++i) printf("%f ", out[i]);
  printf("\n");
}

__always_inline __m256 _mm256_abs_ps(const __m256 a) {
  static const __m256 zeros = _mm256_set1_ps(0.0f);

  __m256 tmp = _mm256_sub_ps(zeros, a);
  return _mm256_max_ps(tmp, a);
}

void sqrtIntrinsics(int N, float initialGuess, float values[], float output[]) {
  static const __m256 kThreshold = _mm256_set1_ps(0.00001f);
  static const __m256 halfs      = _mm256_set1_ps(0.5f);
  static const __m256 ones       = _mm256_set1_ps(1.0f);
  static const __m256 threes     = _mm256_set1_ps(3.0f);

  __m256 x, guess, error, tmp1, tmp2;
  for (int i = 0; i < N; i += 8) {
    x     = _mm256_load_ps(values + i);
    guess = _mm256_set1_ps(initialGuess);

    tmp1  = _mm256_mul_ps(guess, guess);
    tmp1  = _mm256_mul_ps(tmp1, x);
    error = _mm256_abs_ps(_mm256_sub_ps(tmp1, ones));

    // while (error > kThreshold) {
    while (true) {
      tmp1 = _mm256_cmp_ps(error, kThreshold, _CMP_GT_OQ);
      if (_mm256_testz_ps(tmp1, tmp1)) break;

      // guess = (3.f * guess - x * guess * guess * guess) * 0.5f;
      tmp1 = _mm256_mul_ps(threes, guess);
      tmp2 = _mm256_mul_ps(x, guess);
      tmp2 = _mm256_mul_ps(tmp2, guess);
      tmp2 = _mm256_mul_ps(tmp2, guess);

      guess = _mm256_sub_ps(tmp1, tmp2);
      guess = _mm256_mul_ps(guess, halfs);

      tmp1  = _mm256_mul_ps(guess, guess);
      tmp1  = _mm256_mul_ps(tmp1, x);
      error = _mm256_abs_ps(_mm256_sub_ps(tmp1, ones));
    }

    tmp1 = _mm256_mul_ps(guess, x);
    _mm256_store_ps(output + i, tmp1);
  }
}