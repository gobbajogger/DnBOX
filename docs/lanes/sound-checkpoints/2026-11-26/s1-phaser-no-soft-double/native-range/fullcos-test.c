#include "cos-range.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <float.h>
int main(void){
 const uint32_t edge[]={0,0x80000000,1,0x80000001,0x007fffff,0x00800000,0x3f1fffff,0x3f200000,0x3f200001,0x3f200005,0x3f490fd9,0x3f490fda,0x3f490fdb,0x3fc90fda,0x3fc90fdb,0x40490fdb,0x7f7fffff,0xff7fffff,0x7f800000,0xff800000,0x7fc00000};
 float max=0,arg=0;int mismatch=0;
 uint32_t u=0x1fa748eu;
 for(unsigned j=0;j<6000000u+sizeof(edge)/sizeof(edge[0]);j++){
  u^=u<<13;u^=u>>17;u^=u<<5;
  float x=fc_float(j<sizeof(edge)/sizeof(edge[0])?edge[j]:u);
  float c=fc_full(x);double q=cos((double)x); float e=(float)fabs(c-q);
  if(isfinite(x)){assert(isfinite(c)&&e<=2.0e-7f); if(e>max){max=e;arg=x;}}
  else assert(isnan(c));
  if(j<sizeof(edge)/sizeof(edge[0]))printf("%08x %a %a abs=%g\n",fc_bits(x),(double)c,q,(double)e);
  if(isfinite(x)){
   uint32_t a=u*0x9e3779b9u;float v=fc_float(a&0x81ffffffu);
   if(j%263==0)v=fc_float(a&0x807fffffu);
   const float z=fc_product(v,c), want=(float)((double)v*(double)c);
   if(fc_bits(z)!=fc_bits(want))mismatch++;
  }
 }
 printf("max abs cosine error %.12g at x=%a; <6M tiny/normal signed product mismatches %d\n", (double)max,(double)arg,mismatch);
 assert(!mismatch);
}
