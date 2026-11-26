#include "../../../firmware/hal/fm1_time.h"
struct sj6_route_cue_receipt { uint32_t magic, owner_size, part_size, out[4][4]; };
volatile struct sj6_route_cue_receipt sj6_route_cue_receipt;
void sg6_zero_signed_cue(original_runtime_part *p,original_runtime_part *other,float cue[4][256])
{
    for(unsigned f=0;f<4;f++){
        juno6_chorus_t *a=&p->dsp.juno.chorus,*b=&other->dsp.juno.chorus;
        juno6_chorus_init(a,44100);juno6_chorus_init(b,44100);
        a->factory_mode=b->factory_mode=JUNO6_CHORUS_BOTH;
        uint32_t max=0,sum=0,d=2166136261u, m=0;
        for(unsigned tick=0;tick<9;tick++){
            unsigned x=0x82478849u;
            for(unsigned i=0;i<256;i++){
                x=x*1664525u+1013904223u;
                const float noise=(float)((int32_t)(x>>16)-32768)*0x1p-15f;
                const unsigned off=i&31u;
                float z=0.25f*noise;
                if(f==0) z=3.7f*noise;
                if(f==1) z=z*(float)(256-i)*(1.f/256.f);
                if(f==2 && i==31u) z=1.25f;
                if(f==3 && off>=25u) z=1.25f*(1u<<(off-25u));
                cue[0][i]=cue[1][i]=cue[2][i]=cue[3][i]=z;
            }
            uint32_t start=fm1_ticks();
            juno6_chorus_process(a,cue[0],cue[1],256);
            juno6_chorus_process(b,cue[2],cue[3],256);
            uint32_t dt=fm1_ticks()-start;
            if(tick){sum+=dt;if(dt>max)max=dt;if(!m||dt<m)m=dt;}
            for(unsigned i=0;i<256;i++){
                union {float f;uint32_t u;} w={.f=cue[0][i]};d=(d^w.u)*16777619u;w.f=cue[2][i];d=(d^w.u)*16777619u;
            }
        }
        sj6_route_cue_receipt.out[f][0]=sum;sj6_route_cue_receipt.out[f][1]=max;
        sj6_route_cue_receipt.out[f][2]=m;sj6_route_cue_receipt.out[f][3]=d;
    }
    sj6_route_cue_receipt.owner_size=sizeof(juno6_chorus_t);
    sj6_route_cue_receipt.part_size=sizeof(original_runtime_part);
    sj6_route_cue_receipt.magic=0x535a4658u;
}
