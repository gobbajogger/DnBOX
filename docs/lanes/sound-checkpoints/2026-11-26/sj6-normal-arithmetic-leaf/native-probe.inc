#include "../../../firmware/hal/fm1_time.h"
#define fmaf __builtin_fmaf
#include "normal-f32-leaf.h"
struct sj6_route_cue_receipt { uint32_t magic, state_size, coeff_size, out[4][4]; };
volatile struct sj6_route_cue_receipt sj6_route_cue_receipt;
static __attribute__((noinline)) void probe_normal(float *inout,const nf_coef *c)
{
    nf_hist h={0};
    for(unsigned k=0;k<256;k++)inout[k]=normal_plain(&h,c,inout[k]);
}
static __attribute__((noinline)) void probe_stock(float *inout,float a,float r)
{
    float x1=0.f,x2=0.f;
    for(unsigned k=0;k<256;k++){
        float x=inout[k];
        x1=fm_clampf(x1+a*(x-x1-r*.8f*x2),-4.f,4.f);
        x2=fm_clampf(x2+a*(x1-x2),-4.f,4.f);
        inout[k]=x2;
    }
}
void sg6_zero_signed_cue(original_runtime_part *p,original_runtime_part *other,float cue[4][256])
{
    const nf_coef c[2]={{0x1.5e21a6655439ap-4,0x1.5e21a6655439ap-3,0x1.5e21a6655439ap-4,-0x1.a9391e2967a0fp-1,0x1.6127c57046f6cp-3},
    {0x1.fe2d962d6ad16p-3,0x1.fe2d962d6ad16p-2,0x1.fe2d962d6ad16p-3,-0x1.d2ded2afeb253p-9,0x1.d4006af218705p-19}};
    (void)p;(void)other;
    for(unsigned mode=0;mode<4;mode++){
        uint32_t max=0,sum=0,d=2166136261u, mn=UINT32_MAX;
        for(unsigned tick=0;tick<9;tick++){
            unsigned x=0xc7484457u;
            uint32_t v=0;
            /* Timed aggregate: two independent owners, six voices, two layers.
             * No allocator/real routes; prepare same data for both outside timings. */
            for(unsigned pass=0;pass<6;pass++){
                for(unsigned i=0;i<256;i++)for(unsigned q=0;q<4;q++){
                    x=x*1664525u+1013904223u;
                    cue[q][i]=(float)((int32_t)(x>>16)-32768)*0x1p-17f;
                }
                uint32_t start=fm1_ticks();
                for(unsigned q=0;q<4;q++)
                    if(mode&1u)probe_normal(cue[q],c+(mode>>1));
                    else probe_stock(cue[q],mode?.15f:.75f,mode?.6f:0.f);
                v+=fm1_ticks()-start;
                for(unsigned q=0;q<4;q++)for(unsigned k=0;k<256;k++){
                    union {float f;uint32_t u;} w={.f=cue[q][k]};
                    d=(d^w.u)*16777619u;
                }
            }
            if(tick){sum+=v;if(v>max)max=v;if(v<mn)mn=v;}
        }
        sj6_route_cue_receipt.out[mode][0]=sum;sj6_route_cue_receipt.out[mode][1]=max;
        sj6_route_cue_receipt.out[mode][2]=mn;sj6_route_cue_receipt.out[mode][3]=d;
    }
    sj6_route_cue_receipt.state_size=sizeof(nf_hist);sj6_route_cue_receipt.coeff_size=sizeof(nf_coef);
    sj6_route_cue_receipt.magic=0x535a4658u;
}
