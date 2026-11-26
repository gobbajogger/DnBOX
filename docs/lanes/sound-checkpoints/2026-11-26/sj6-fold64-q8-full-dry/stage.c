/* Standalone clean diagnostic stage, not a firmware route. */
#include <assert.h>
#include "selector.h"
#include "halfband.h"
#include "anchor-generated.private.h"

typedef struct {
    sj6_band band;
    unsigned shape;
} owner;
static inline float lookup(unsigned s,unsigned row,unsigned p,unsigned q,float f){
    float x=TBL[s][row][p],y=TBL[s][row][q];
    return x+f*(y-x);
}
static inline float shape_lookup(float phase,float unused,void *v){
    (void)unused;
    owner *o=v;
    float x=phase*TBL_N;
    unsigned i=(unsigned)x;
    float f=x-i;
    i&=TBL_N-1;
    unsigned j=(i+1)&(TBL_N-1);
    float a=lookup(o->shape,o->band.row,i,j,f);
    if(o->band.blend>0.f){
        float b=lookup(o->shape,o->band.row+1u,i,j,f);
        a+=o->band.blend*(b-a);
    }
    float natural=o->shape?.495f*1.17878819f*(8.f/(3.14159265358979323846f*3.14159265358979323846f)):.5f*1.2377909421920776f;
    return a*(natural*TBL_GAIN[o->shape]);
}
/* Normal is deliberately separate from the stage blend. */
void sj6_anchor_run(compact_halfband *s,unsigned shape,float increment,float *output,unsigned count){
    assert(shape<2);
    owner o={sj6_wavetable_band(increment),shape};
    for(unsigned i=0;i<count;i++){
        float x[4],p=s->phase,step=.25f*increment;
        for(unsigned j=0;j<4;++j){
            x[j]=shape_lookup(p,step,&o);
            p+=step;if(p>=1.f)p-=1.f;
        }
        s->phase=p;output[i]=compact_decimate(s,x);
    }
}
