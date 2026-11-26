#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define SR 44100.f
static const float min_phase=100.f,max_phase=800.f;
static float val(unsigned pos){return pos==5u?12.f:1.f+(299.f/127.f)*(float)pos;}
int main(void){
 for(unsigned j=0;j<2;j++)for(unsigned pos=1;pos<128;pos++){
  float bpm=val(pos),q=(0.10472f/SR)*bpm,a2=q*q;
  float si=j?q+q*a2*(-0x1.555556p-3f+a2*0x1.111112p-7f):(float)sin((double)q);
  float co=j?1.f+a2*(-.5f+a2*0x1.555556p-5f):(float)cos((double)q);
  if((double)si*si+(double)co*co<=1.)continue;
  float s=0,c=0,l=0,r=0, coeff=(1.f/SR)*5.0625f,z=6.28319f*min_phase,span=.5f*(6.28319f*max_phase-z);
  for(unsigned i=0;i<26460000;i++){
   float sn=si*c+co*s;
   float cn=(1.f+(co*c+(0.f-si)*s))-(float)(i!=0);
   s=sn;c=cn;
   l=coeff*(span*(1.f-s)+z);r=coeff*(span*(1.f-c)+z);
   if(fabsf(l)>.625f||fabsf(r)>.625f){
     printf("path %u pos %u bpm %a angle %a sin %a cos %a frame %u secs %.6f left %a right %a state %a %a\n",j,pos,(double)bpm,(double)q,(double)si,(double)co,i,(double)i/44100.,(double)l,(double)r,(double)s,(double)c);
     break;
   }
  }
 }
}
