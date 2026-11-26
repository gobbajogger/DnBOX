#include SRC
#include "fm1_time.h"
#define AT ((volatile unsigned*)0x01c08000)
#define OUT ((volatile unsigned*)0x01c20000)
enum {N=512};
static unsigned in[N],ow[N];
static float angle[N];
static float unword(unsigned u){float f;__builtin_memcpy(&f,&u,4);return f;}
void bench(void){
 fm1_time_init();AT[0]=0x43554231;
 for(unsigned p=0;p<4;p++){
  for(unsigned i=0;i<N;i++){
   const unsigned v=(i*3185141u+2297u)&0x7fffffu;
   unsigned a=p==0?0u:p==1?v:p==2?0x800000u|v:0x3e000000u|v;
   in[i]=a|((i&1u)<<31);
   angle[i]=.021f+(.578f-.021f)*(float)((i*213u)&511)/511.f;
  }
  unsigned t=fm1_ticks();
  for(unsigned i=0;i<N;i++)ow[i]=ph_word(ph_cosprod(unword(in[i]),angle[i],0));
  AT[4+p]=fm1_ticks()-t;
  for(unsigned i=0;i<N;i++){OUT[p*N+i]=ow[i];OUT[4*N+p*N+i]=in[i];}
 }
 AT[1]=0x600d600d;for(;;)__asm__ volatile("" ::: "memory");
}
