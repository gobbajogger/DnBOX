#include <stdint.h>
#include <stdio.h>
#define X0X_DRUM_TABLES_DEFINE
#include "drum909_dsp.h"
static float old(float x,float d,float g) {
 const float u=d*x; const float z=d9_tanh_table(fm_fabsf(u)*X0X_TANH_SCALE);
 return (u<0.f?-z:z)*g;
}
static float candidate(float x) {
 fm_bits_t u={.f=(1.6f * X0X_TANH_SCALE)*x};
 const uint32_t sign=u.u&0x80000000u; u.u&=0x7fffffffu;
 fm_bits_t t={.f=d9_tanh_table(u.f)}; t.u|=sign;
 return t.f*1.0849887132644653f;
}
static int run(float folded) {
 const float x=folded*(1.f/1073741824.f)-1.f;
 const fm_bits_t a={.f=old(x,1.6f,1.0849887132644653f)},b={.f=candidate(x)};
 return a.u!=b.u;
}
int main(void) {
 uint32_t n=0;
 for(uint32_t i=0;i<(1u<<24);i++) { if(run((float)i)) return 1; ++n; }
 for(unsigned e=24;e<31;++e) {
   const uint32_t step=1u<<(e-23);
   for(uint32_t i=1u<<e;i<(1u<<(e+1));i+=step) { if(run((float)i)) return 1; ++n; }
 }
 if(run((float)0x7fffffff)) return 1;
 printf("PASS: %u distinct rounded folded phase values at fixed drive 1.6\n",n+1);
}
