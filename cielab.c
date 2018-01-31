#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>

#include "cielab.h"

enum mode {norm,lab,srgb};

#define XnD50 96.6797
#define YnD50 100.000
#define ZnD50 82.5188

#define XnD65 95.047
#define YnD65 100.000
#define ZnD65 108.883

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


void matMult(float m2[3], float m1[3][3], float r[3]) {
  int c, d, k;
  for (c = 0; c < 3; c++) {
    r[c] = 0;
    for (d = 0; d < 1; d++) {
      for (k = 0; k < 3; k++) {
	r[c] += m1[c][k] * m2[k];
      }
    }
  }
}

void xyz2lab(int isD50, float x, float y, float z, float *l, float *a, float *b) {
  float Xn=XnD50, Yn=YnD50, Zn=ZnD50;
  if (!isD50) {
    Xn=XnD65;
    Yn=YnD65;
    Zn=ZnD65;
  }
  *l = (float)116 * f(y / Yn) - 16;
  *a = (float)500 * (f(x / Xn) - f(y / Yn));
  *b = (float)200 * (f(y / Yn) - f(z / Zn));
}

void normalizeXYZ(float x, float y, float z, float *Xn, float *Yn, float *Zn) {
  float C = (float)1/((float)80 - 0.2);
  *Xn = C * (x - 0.1901);
  *Yn = C * (y - 0.2);
  *Zn = C * (z - 0.2178);
}

void xyz2rgb(float x, float y, float z, float *r, float *g, float *b) {
  float m1[3];
  float m2[3][3] = {
    {3.1338561, -1.6168667, -0.4906146},
    {-0.9787684, 1.9161415, 0.0334540},
    {0.0719453, -0.2289914, 1.4052427}
  };
  float res[3];
  m1[0] = x;
  m1[1] = y;
  m1[2] = z;
  matMult(m1, m2, res);
  *r = res[0];
  *g = res[1];
  *b = res[2];
}

float applyGamma_sub(float v) {
  if (v <= 0.0031308) return v*12.92;
  else return powf(v * 1.055, 1/2.4) - 0.055;
}

void applyGamma(float r0, float g0, float b0, float *r, float *g, float *b) {
  *r = applyGamma_sub(r0);
  *g = applyGamma_sub(g0);
  *b = applyGamma_sub(b0);
}

int main(int argc, char *argv[]) {
  float x, y, z, Xn, Yn, Zn, v1, v2, v3;
  int isD50 = 1, byte=0;
  int o;
  enum mode action = srgb;
  while ((o=getopt(argc, argv, "Dm:bx:y:z:"))!=-1) {
    switch (o) {
    case 'D':
      isD50=0;
      break;
    case 'm':
      if (!strcasecmp(optarg, "lab")) {
	action = lab;
      } else if (!strcasecmp(optarg, "norm")) {
	action = norm;
      } else if (!strcasecmp(optarg, "srgb")) {
	action = srgb;
      }
      break;
    case 'b':
      byte=1;
      break;
    case 'x':
      sscanf(optarg, "%f", &x);
      break;
    case 'y':
      sscanf(optarg, "%f", &y);
      break;
    case 'z':
      sscanf(optarg, "%f", &z);
      break;
    }
  }

  switch (action) {
  case norm:
    normalizeXYZ(x, y, z, &v1, &v2, &v3);
    break;
  case lab:
    xyz2lab(isD50, x, y, z, &v1, &v2, &v3);
    break;
  case srgb:
    if (!isD50) {
      fprintf(stderr, "D65 XYZ conversion to sRGB not supported yet.\n");
      return -2;
    }
    normalizeXYZ(x, y, z, &Xn, &Yn, &Zn);
    xyz2rgb(Xn, Yn, Zn, &v1, &v2, &v3);
    if (v1 < 0) v1=0;
    if (v1 > 1) v1=1;
    if (v2 < 0) v2=0;
    if (v2 > 1) v2=1;
    if (v3 < 0) v3=0;
    if (v3 > 1) v3=1;
    applyGamma(v1, v2, v3, &v1, &v2, &v3);
    break;
  }
  if (byte) {
    v1 = floorf(v1 * (float)256);
    v2 = floorf(v2 * (float)256);
    v3 = floorf(v3 * (float)256);
  }
  printf("%g %g %g\n", v1, v2, v3);
  return 0;
}
