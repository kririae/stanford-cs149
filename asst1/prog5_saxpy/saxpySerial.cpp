#include "atom.h"

void saxpySerial(int N, float scale, Atom in[]) {
  for (int i = 0; i < N; i++) {
    in[i].res = scale * in[i].x + in[i].y;
  }
}
