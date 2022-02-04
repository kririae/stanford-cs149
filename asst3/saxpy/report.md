The result is

```
---------------------------------------------------------
Found 1 CUDA devices
Device 0: NVIDIA GeForce GTX 1660 Ti with Max-Q Design
   SMs:        24
   Global mem: 5945 MB
   CUDA Cap:   7.5
---------------------------------------------------------
Running 3 timing tests:
Effective BW by CUDA saxpy: 399.980 ms		[2.794 GB/s]
Effective BW by CUDA saxpy: 431.404 ms		[2.591 GB/s]
Effective BW by CUDA saxpy: 430.732 ms		[2.595 GB/s]

```

while the result of `bandwisthTest --memory=pageable` is

```
[CUDA Bandwidth Test] - Starting...
Running on...

 Device 0: NVIDIA GeForce GTX 1660 Ti with Max-Q Design
 Quick Mode

 Host to Device Bandwidth, 1 Device(s)
 PAGEABLE Memory Transfers
   Transfer Size (Bytes)	Bandwidth(MB/s)
   33554432			3215.3

 Device to Host Bandwidth, 1 Device(s)
 PAGEABLE Memory Transfers
   Transfer Size (Bytes)	Bandwidth(MB/s)
   33554432			2998.0

 Device to Device Bandwidth, 1 Device(s)
 PAGEABLE Memory Transfers
   Transfer Size (Bytes)	Bandwidth(MB/s)
   33554432			26704.3

Result = PASS
```

NOTE: The CUDA Samples are not meant for performance measurements. Results may vary when GPU Boost is enabled.

```
==23731== Profiling result:
Type Time(%) Time Calls Avg Min Max Name
61.35% 773.59ms 6 128.93ms 118.44ms 149.71ms [CUDA memcpy HtoD]
28.16% 355.13ms 3 118.38ms 118.29ms 118.50ms [CUDA memcpy DtoH]
10.49% 132.24ms 3 44.079ms 44.061ms 44.103ms saxpy_kernel(int, float, float*, float*, float\*)
```
