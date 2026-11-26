#include <math.h>
#include "normal_coeff.h"
void coeff(double rate,double hz,double *out) {
  sj6_normal_coeff c=sj6_normal_coefficients(rate,hz,1.0);
  out[0]=c.b0;out[1]=c.b1;out[2]=c.b2;out[3]=c.a1;out[4]=c.a2;
}
