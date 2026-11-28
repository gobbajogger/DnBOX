#define X0X_HOST 1
#include S1U_SOURCE
#include <stdio.h>
#include <stdlib.h>
typedef union { float f; uint32_t u; } Word;
static void check(uint32_t p) {
  float v=s1u_sin(p);
  Word sc={.f=v>0.f?0.f:-0.f},sn={.f=v*.8f*0.f*3.f};
  if(sc.u!=(p-1u<0x7fffffffu?0u:0x80000000u)||sn.u!=(p&0x80000000u)){
      fprintf(stderr,"failed phase %08x square %08x sine %08x\n",p,sc.u,sn.u);exit(1);
  }
}
int main(void) {
  uint32_t v=1;
  for(uint32_t i=0;i<1000000u;++i){
    check(i);check(0x80000000u+i);check(0x80000000u-i);check(0u-i);
    v=v*1664525u+1013904223u;check(v);
  }
  puts("5,000,000 sub oscillator phase cases: equal signed zeros");
}
