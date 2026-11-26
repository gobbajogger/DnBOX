#include "cos-range.h"
#include "fm1_time.h"
#if BASE
#include <math.h>
#endif
#define AT ((volatile uint32_t*)0x01c08000)
#define O ((volatile uint32_t*)0x01c20000)
static uint32_t s=0x1fa748eu;
enum{N=512};
static float x[N],v[N],y[N];
static uint32_t rng(void){s^=s<<13;s^=s>>17;s^=s<<5;return s;}
void bench(void){
 fm1_time_init();AT[0]=0x43554231;
 for(unsigned p=0;p<4;p++){
  for(unsigned i=0;i<N;i++){
   uint32_t u=rng();
   if(p==0)x[i]=.021f+.556f*(float)(u&511u)*(1.f/511.f);
   else if(p==1)x[i]=.62499f+1.726f*(float)(u&511u)*(1.f/511.f);
   else if(p==2)x[i]=fc_float(u%0x7f800000u|((u&1u)<<31));
   else x[i]=fc_float((u&0x807fffffu)|0x7f000000u);
   v[i]=fc_float((rng()&0x807fffffu)|(i&1u?0x3f000000u:0u));
  }
  unsigned t=fm1_ticks();
  for(unsigned i=0;i<N;i++){
#if BASE
   y[i]=(float)((double)v[i]*cos((double)x[i]));
#else
   y[i]=fc_product(v[i],fc_full(x[i]));
#endif
  }
  AT[4+p]=fm1_ticks()-t;
  for(unsigned i=0;i<N;i++){O[p*N+i]=fc_bits(y[i]);O[(p+4)*N+i]=fc_bits(x[i]);O[(p+8)*N+i]=fc_bits(v[i]);}
 }
 AT[1]=0x600d600d;for(;;)__asm__ volatile("" ::: "memory");
}
