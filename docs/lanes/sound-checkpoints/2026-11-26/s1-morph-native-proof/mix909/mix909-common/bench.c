#include <stdint.h>
#include "fm1_time.h"
static inline uint32_t bits(float f){union{float f;uint32_t u;}x={.f=f};return x.u;}
static inline float fm_fabsf(float f){union{float f;uint32_t u;}x={.f=f};x.u&=0x7fffffffu;return x.f;}
#define GLIDE_A .0022651f
#include "../mix909.inc"
enum {N=256,R=512};
static __attribute__((noinline)) float baseline(float a[4][N],const float g[6],float target);
static __attribute__((noinline)) float candidate(float a[4][N],const float g[6],float target){
 float c=g[0];
 if ((c==1.f&&target==1.f)||g[1]!=c||g[4]!=c||g[5]!=c||g[2]!=0.f||g[3]!=0.f) return baseline(a,g,target);
 return fm1_909_mix_common(a[0],a[1],a[2],a[3],N,&c,target,.73f,0);
}
static __attribute__((noinline)) float baseline(float a[4][N],const float g[6],float target){
 float m[N],pk=0;
 for(unsigned i=0;i<N;i++)m[i]=(a[0][i]+a[1][i])*.5f;
 for(unsigned j=0;j<4;j++){
  float c=g[j<2?j:j+2];
  if(c!=target)for(unsigned i=0;i<N;i++){c+=(target-c)*GLIDE_A;a[j][i]*=c;}
  else if(c!=1.f)for(unsigned i=0;i<N;i++)a[j][i]*=c;
 }
 for(unsigned j=2;j<4;j++)if(g[j]!=0.f)for(unsigned i=0;i<N;i++)a[j][i]+=m[i]*g[j];
 for(unsigned i=0;i<N;i++){float x=fm_fabsf(m[i])*.73f;if(x>pk)pk=x;}
 return pk;
}
static uint32_t data(unsigned mode,int tested,uint32_t *elapsed) {
 const float g[4][6]={{1,1,0,0,1,1},{.7569f,.7569f,0,0,.7569f,.7569f},
                       {.7568869f,.7568869f,0,0,.7568869f,.7568869f},{.7568869f,.79f,.41f,.19f,.7568869f,.7568869f}};
 const float goal[4]={1.f,.7569f,.7569f,.7569f};
 float a[4][N];uint32_t here=0,h=0x811c9dc5u;
 for(unsigned k=0;k<R;k++){
  for(unsigned j=0;j<4;j++)for(unsigned i=0;i<N;i++)a[j][i]=(float)(int32_t)(i+k+j*7u)*.001f;
  uint32_t t=fm1_ticks();float pk=tested?candidate(a,g[mode],goal[mode]):baseline(a,g[mode],goal[mode]);here+=fm1_ticks()-t;
  for(unsigned j=0;j<4;j++)for(unsigned i=0;i<N;i++)h=(h^bits(a[j][i]))*16777619u;
  h=(h^bits(pk))*16777619u;
 }
 *elapsed=here;return h;
}
void bench_start(void){
 volatile uint32_t*out=(volatile uint32_t*)0x01c10000;
 fm1_time_init();
 for(unsigned m=0;m<4;m++)for(unsigned k=0;k<2;k++){
  uint32_t t,h=data(m,(int)k,&t);out[1+m*4+k]=t;out[3+m*4+k]=h;
 }
 out[0]=0x3930394du;
}
