Note that `<sys/sysctl.h>` is removed in glibc 2.32, change it to `<linux/sysctl.h>`.

`uniform` qualifier indicates that the variable has the same value for all concurrently-executing program instances.

When the scene is switched to `--view 2`, the performance reduced from 5.8x to 4.8x.

If the core is 4c8t, consider there's only one SIMD unit in one core actually, the speedup is at most 4 \* 8 = 32.

However, in the `ispc` implementation, the code is:

```cpp
foreach (j = 0 ... height, i = 0 ... width) {
  float x = x0 + i * dx;
  float y = y0 + j * dy;

  int index     = j * width + i;
  output[index] = mandel(x, y, maxIterations);
}
```

The variable `j` is paralleled yet. In SIMD, sequential access can significantly promoted the performance. So we could modify it to,

foreach (j = 0 ... height, i = 0 ... width) {
float x = x0 + i _ dx;
float y = y0 + j _ dy;

int index = j \* width + i;
output[index] = mandel(x, y, maxIterations);
}
