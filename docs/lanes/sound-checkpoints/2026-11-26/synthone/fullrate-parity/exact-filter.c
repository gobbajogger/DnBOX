/* SPDX-License-Identifier: GPL-3.0-only */
#include "exact-filter.h"
#include "soundpipe.h"
#include <math.h>
#include <stdlib.h>
struct voice {
    sp_data *sp;
    sp_moogladder *lp;
    sp_butbp *bp;
    sp_buthp *hp;
};
void *exact_voice_filter_new(float rate) {
    struct voice *o=calloc(1,sizeof *o);if(!o||rate!=44100.f)abort();
    sp_create(&o->sp);if(!o->sp)abort();o->sp->sr=(int)rate;
    sp_moogladder_create(&o->lp);sp_moogladder_init(o->sp,o->lp);
    sp_butbp_create(&o->bp);sp_butbp_init(o->sp,o->bp);
    sp_buthp_create(&o->hp);sp_buthp_init(o->sp,o->hp);
    return o;
}
void exact_voice_filter_drop(void *state) {
    struct voice *o=state;
    if(!o)return;
    sp_moogladder_destroy(&o->lp);sp_butbp_destroy(&o->bp);sp_buthp_destroy(&o->hp);
    sp_destroy(&o->sp);free(o);
}
float exact_voice_filter(void *state,float input,float f,float res,float type,float mix) {
    struct voice *o=state;
    o->lp->freq=o->bp->freq=o->hp->freq=f;
    if(type==0.f)o->lp->res=res;
    else if(type==1.f)o->bp->bw=(float)(0.0625f*o->sp->sr*(-1.f+exp2((double)(1.f-res))));
    float a=0.f,b=0.f,c=0.f;
    sp_moogladder_compute(o->sp,o->lp,&input,&a);
    sp_butbp_compute(o->sp,o->bp,&input,&b);
    sp_buthp_compute(o->sp,o->hp,&input,&c);
    const float selected=type==0.f?a:type==1.f?b:type==2.f?c:0.f;
    return selected*mix+input*(1.f-mix);
}
