extern float fmaf(float,float,float);
#define fmaf __builtin_fmaf
#include "leaf.h"
void normal_span_f32(nf_hist *h, const nf_coef *c, float *x,unsigned n)
{
    for(unsigned i=0;i<n;i++)x[i]=normal_f32(h,c,x[i]);
}
