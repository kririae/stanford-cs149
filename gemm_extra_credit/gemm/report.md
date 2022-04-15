# Performance Measurement

without any optimization(calling `__gemm_naive`):

```
[Intel MKL]:            [5.673] ms      [5.509] GB/s    [378.56] GFLOPS
[Student GEMM]:         [4201.226] ms   [0.007] GB/s    [0.51] GFLOPS
[Ref ISPC GEMM]:        [29.492] ms     [1.060] GB/s    [72.82] GFLOPS
Total squared error student sol: 0.000000
Total squared error ref ispc: 0.000000
```

with blocking(calling `__gemm_blocking`), with `BLOCKSIZE=8`:

```
[Intel MKL]:            [5.604] ms      [5.576] GB/s    [383.18] GFLOPS
[Student GEMM]:         [775.963] ms    [0.040] GB/s    [2.77] GFLOPS
[Ref ISPC GEMM]:        [30.361] ms     [1.029] GB/s    [70.73] GFLOPS
```

however, if we experiment with different blocksize:

| BLOCKSIZE | GFLOPS |
| 2         | 1.08   |
| 4         | 2.58   |
| 6         | 3.13   |
| 8         | 2.78   |
| 16        | 1.58   |
| 32        | 0.86   |

We see that when `BLOCKSIZE=6`, it reaches its maximum speedup.
And the speedup is 6.13.

I'll investigate its vectorization rate in vtune on my friend's intel machine.
But yet there's much space for improvement.

