#include <stddef.h>
#include "numeric.h"
typedef struct {float hp_1,hp_2,dc_1,dc_2;} ch_state;
_Static_assert(sizeof(ch_state)==16,"the owner uses 32 B of independent stereo mode storage");
static inline float hp_tpt(ch_state *p,float x)
{
    float v=x-p->hp_2;
    float d1=TPT_M*p->hp_1+TPT_A2*v;
    float d2=TPT_A2*p->hp_1+TPT_A3*v;
    float y=((x-TPT_K*(p->hp_1+d1))-p->hp_2)-d2;
    p->hp_1+=2.f*d1;
    p->hp_2+=2.f*d2;
    return y;
}
static inline float dc_lowpass(float *s,float x,float delta)
{
    float v=x-*s;
    *s+=delta*v;
    return v;
}
void mini_run(unsigned route,const float *in,float *out,size_t n,unsigned split)
{
    ch_state s={0};
    size_t i=0;
    while(i<n){
        size_t rest=n-i, lim=split ? 1u+(i%113u):rest;
        if(lim>rest)lim=rest;
        while(lim--){
            float y=in[i];
            if(route==0||route==2||route==3)y=hp_tpt(&s,y);
            if(route==1||route==2||route==3)y=dc_lowpass(&s.dc_1,y,DC1_DELTA);
            if(route==3)y=dc_lowpass(&s.dc_2,y,DC2_DELTA);
            out[i++]=y;
        }
    }
}
