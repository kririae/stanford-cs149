#include "atom.h"

export void saxpy_ispc(uniform int N, uniform float scale, uniform struct Atom in[]) {
  foreach (i = 0 ... N) { in[i].res = scale * in[i].x + in[i].y; }
}

// task void saxpy_ispc_task(uniform int N, uniform int span, uniform float scale,
//                           uniform float X[], uniform float Y[],
//                           uniform float result[]) {
//   uniform int indexStart = taskIndex * span;
//   uniform int indexEnd   = min(N, indexStart + span);

//   foreach (i = indexStart... indexEnd) { result[i] = scale * X[i] + Y[i]; }
// }

// export void saxpy_ispc_withtasks(uniform int N, uniform float scale,
//                                  uniform float X[], uniform float Y[],
//                                  uniform float result[]) {
//   uniform int span = N / 128;  // 64 tasks

//   launch[N / span] saxpy_ispc_task(N, span, scale, X, Y, result);
// }

// export void saxpy_ispc_withtasks_soa(uniform int N, uniform float scale,
//                                      uniform float X[], uniform float Y[],
//                                      uniform float result[]) {
//   uniform int span = N / 128;  // 64 tasks

//   launch[N / span] saxpy_ispc_task(N, span, scale, X, Y, result);
// }