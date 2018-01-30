#include <stdio.h>
#include <math.h>

#define D50

#ifdef D50
#define Xn 96.6797
#define Yn 100.000
#define Zn 82.5188
#else
#define Xn 95.047
#define Yn 100.000
#define Zn 108.883
#endif

#define DELTA ((float)6/(float)29)
#define exp2(x) ((x)*(x))
#define exp3(x) ((x)*(x)*(x))
#define croot(x) (powf((x), (float)1/(float)3))

float f(float t) {
  if (t > exp3(DELTA)) {
    return croot(t);
  } else {
    return t / (3 * exp2(DELTA)) + ((float)4/(float)29);
  }
}

void xyz2lab(float x, float y, float z, float *l, float *a, float *b) {
  *l = (float)116 * f(y / Yn) - 16;
  *a = (float)500 * (f(x / Xn) - f(y / Yn));
  *b = (float)200 * (f(y / Yn) - f(z / Zn));
}

int main(int argc, char *argv[]) {
  float x, y, z, l, a, b;
  sscanf(argv[1], "%f", &x);
  sscanf(argv[2], "%f", &y);
  sscanf(argv[3], "%f", &z);
  xyz2lab(x, y, z, &l, &a, &b);
  printf("%g %g %g\n", l, a, b);
  return 0;
}
