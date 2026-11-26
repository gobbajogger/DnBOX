#include "../../../firmware/hal/fm1_time.h"
#define fmaf __builtin_fmaf
#ifndef PACK
#define PACK 1
#endif
#include "hbp-proof.h"
struct sj6_route_cue_receipt { uint32_t magic, state_size, coeff_size, out[4][4]; };
volatile struct sj6_route_cue_receipt sj6_route_cue_receipt;
static float old_ap(float x,float a,float *h){float y=*h+a*x;*h=x-a*y;return y;}
static float old_2x(float a,float b,float *h){
    b=old_ap(b,.09110601f,h);a=old_ap(a,.31684510f,h+1);
    b=old_ap(b,.58723765f,h+2);a=old_ap(a,.85663281f,h+3);
    return .5f*(a+b);
}
static __attribute__((noinline)) float leaf_cb(unsigned m,compact_halfband *s,float *old,float a,float b,float c,float d)
{
    if(m&1u){const float x[4]={a,b,c,d};return compact_decimate(s,x);}
    return old_2x(a,c,old);
}
void sg6_zero_signed_cue(original_runtime_part *p,original_runtime_part *other,float cue[4][256])
{
    (void)p;(void)other;
    for(unsigned mode=0;mode<4;mode++){
      uint32_t max=0,sum=0,d=2166136261u,mn=UINT32_MAX;
      for(unsigned shot=0;shot<9;shot++){
        uint32_t elapsed=0;
        unsigned seed=0xc7484457u;
        for(unsigned lane=0;lane<24;lane++){
           for(unsigned i=0;i<256;i++)for(unsigned q=0;q<4;q++){
              seed=seed*1664525u+1013904223u;
              cue[q][i]=(float)((int32_t)(seed>>16)-32768)*0x1p-17f;
           }
           compact_halfband s={0};float h[4]={0};
           uint32_t start=fm1_ticks();
           for(unsigned i=0;i<256;i++)
             cue[0][i]=leaf_cb(mode,&s,h,cue[0][i],cue[1][i],cue[2][i],cue[3][i]);
           elapsed+=fm1_ticks()-start;
           for(unsigned i=0;i<256;i++)
             {union {float f;uint32_t u;} w={.f=cue[0][i]};d=(d^w.u)*16777619u;}
        }
        if(shot){sum+=elapsed;if(elapsed>max)max=elapsed;if(elapsed<mn)mn=elapsed;}
      }
      sj6_route_cue_receipt.out[mode][0]=sum;sj6_route_cue_receipt.out[mode][1]=max;
      sj6_route_cue_receipt.out[mode][2]=mn;sj6_route_cue_receipt.out[mode][3]=d;
    }
    sj6_route_cue_receipt.state_size=sizeof(compact_halfband);
    sj6_route_cue_receipt.coeff_size=sizeof(float[4]);
    sj6_route_cue_receipt.magic=0x535a4658u;
}
