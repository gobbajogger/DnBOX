/* Generated Main voice/source DSP witness, not a firmware or Mac binary. */
#ifndef DNBX_SJ6_MAIN_4X_H
#define DNBX_SJ6_MAIN_4X_H
#include <math.h>

typedef struct {float phase; float section[2][4][2];} sj6_main_4x;
typedef float (*sj6_main_shape)(float phase,float step,void *ctx);
static inline float sj6_phase_add(float p,float q){
    float r=p+q;
    return r>=1.f?r-1.f:r;
}
static inline float sj6_hb_ap(float x,float a,float h[2]){
    const float y=fmaf(x-h[1],a,h[0]);
    h[0]=x; h[1]=y; return y;
}
static inline float sj6_hb(float even,float odd,float h[4][2]){
    odd=sj6_hb_ap(odd,.079866424202919f,h[0]);
    even=sj6_hb_ap(even,.28382933139801025f,h[1]);
    odd=sj6_hb_ap(odd,.5453536510467529f,h[2]);
    even=sj6_hb_ap(even,.8344119191169739f,h[3]);
    return .5f*(odd+even);
}
static inline float sj6_main_4x_sample(sj6_main_4x *s,float increment,sj6_main_shape shape,void *context){
    const float step=.25f*increment;
    float p=s->phase, x[4];
    for(unsigned j=0;j<4;j++){x[j]=shape(p,step,context);p=sj6_phase_add(p,step);}
    s->phase=p;
    const float a=sj6_hb(x[0],x[1],s->section[0]);
    const float b=sj6_hb(x[2],x[3],s->section[0]);
    return sj6_hb(a,b,s->section[1]);
}
#endif
