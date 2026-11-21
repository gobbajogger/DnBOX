#include <stdio.h>
#include <string.h>
#include <math.h>
#include "fastmath.h"
static unsigned word(float f){unsigned u;memcpy(&u,&f,4);return u;}
int main(void){
 float times[]={.05f,.005f,.2f,.3f,.5f,.0005f,.004f};
 puts("time,attack,oracle,fm_exp_double_x,fm_exp_float_x,fm_exp2_double_x,fm_exp2_float_x");
 for(unsigned i=0;i<sizeof(times)/sizeof(times[0]);i++)for(int a=0;a<2;a++){
  float t=times[i],scale=a?.6f:1.f;
  double x=-1./((44100.*(double)t)*(a?.6:1.));
  float xf=-1.f/((44100.f*t)*scale);
  printf("%.10g,%d,%08x,%08x,%08x,%08x,%08x\n",t,a,word((float)exp(x)),word(fm_expf((float)x)),word(fm_expf(xf)),word(fm_exp2f((float)(x*1.4426950408889634))),word(fm_exp2f(xf*FM_LOG2E)));
 }
}
