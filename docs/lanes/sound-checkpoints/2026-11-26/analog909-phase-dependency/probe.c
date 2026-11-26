#include <stdio.h>
#include "drum909.c"
int main(void){
 drum909_t d={0}; drum909_init(&d);
 drum909_trigger(&d,DR_SD,1.f); drum909_trigger(&d,DR_LT,1.f);
 d9_env_t e[3]={d.bt[DR_SD].pitch,d.tom[0].pitch,d.tom[0].pitch};
 const float r[3]={d.bt[DR_SD].tune2/d.bt[DR_SD].tune,1.5f,2.75f};
 const char *names[3]={"SD second shell","LT first partial","LT second partial"};
 for(unsigned k=0;k<3;k++){
  unsigned bad=0,first=0,oo=0,nn=0; float ff=0;
  for(unsigned i=0;i<1024;i++){
   if(!(i&127))d9_env_anchor(&e[k]);
   const float f=d9_env_tick(&e[k]);
   const uint32_t a=d9_inc(f*r[k]), b=(uint32_t)(f*(r[k]*D9_PHASE_PER_HZ));
   if (a!=b && !bad++) { first=i;ff=f;oo=a;nn=b; }
  }
  if(!bad)return 2;
  printf("%s: %u of 1024 phase increments differ; first frame %u hz=%a ratio=%a increment %u -> %u\n",names[k],bad,first,ff,r[k],oo,nn);
 }
}
