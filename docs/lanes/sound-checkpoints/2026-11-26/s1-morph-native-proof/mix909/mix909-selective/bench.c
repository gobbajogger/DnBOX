#include <stdint.h>
#include "fm1_time.h"
static inline uint32_t bits(float f){union{float f;uint32_t u;}x={.f=f};return x.u;}
static inline float fm_fabsf(float f){union{float f;uint32_t u;}x={.f=f};x.u&=0x7fffffffu;return x.f;}
#include "../mix909.inc"
enum {N=256,R=512};
static __attribute__((noinline)) float baseline(float a[4][N],const float g[6]);
static __attribute__((noinline)) float candidate(float a[4][N],const float g[6]){
 if(g[0]==1.f||g[1]==1.f||g[4]==1.f||g[5]==1.f) return baseline(a,g);
 return fm1_909_mix_settled(a[0],a[1],a[2],a[3],N,g,.73f,0);
}
static __attribute__((noinline)) float baseline(float a[4][N],const float g[6]){
 float m[N],pk=0;
 for(unsigned i=0;i<N;i++)m[i]=(a[0][i]+a[1][i])*.5f;
 for(unsigned j=0;j<4;j++){float c=g[j<2?j:j+2];if(c!=1.f)for(unsigned i=0;i<N;i++)a[j][i]*=c;}
 for(unsigned j=2;j<4;j++)if(g[j]!=0.f)for(unsigned i=0;i<N;i++)a[j][i]+=m[i]*g[j];
 for(unsigned i=0;i<N;i++){float x=fm_fabsf(m[i])*.73f;if(x>pk)pk=x;}
 return pk;
}
static uint32_t data(unsigned mode,int tested,uint32_t *elapsed) {
 const float g[4][6]={{1,1,0,0,1,1},{.785f,.709f,0,0,.72f,.82f},
                       {.81f,.72f,.23f,.55f,.78f,.91f},{1,1,.41f,.19f,1,1}};
 float a[4][N];uint32_t here=0,h=0x811c9dc5u;
 for(unsigned k=0;k<R;k++){
  for(unsigned j=0;j<4;j++)for(unsigned i=0;i<N;i++)a[j][i]=(float)(int32_t)(i+k+j*7u)*.001f;
  uint32_t t=fm1_ticks();float pk=tested?candidate(a,g[mode]):baseline(a,g[mode]);here+=fm1_ticks()-t;
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
