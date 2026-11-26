/* Host sidecar for per-sample Synth One original delay topology. */
#include "baseline-delay.h"
#include "soundpipe.h"
#include <math.h>
#include <stdlib.h>
struct source_s1_exact_delay {
    sp_data *sp;
    sp_vdelay *line[4];
    sp_moogladder *lp[2];
    sp_crossfade *mix[2];
};
source_s1_exact_delay *source_s1_delay_new(unsigned rate) {
    source_s1_exact_delay *p=calloc(1,sizeof *p);
    if(!p)return 0;
    sp_create(&p->sp); p->sp->sr=rate; p->sp->nchan=2;
    for(unsigned i=0;i<4;i++){
        sp_vdelay_create(&p->line[i]); sp_vdelay_init(p->sp,p->line[i],10.f);
    }
    for(unsigned i=0;i<2;i++){
        sp_moogladder_create(&p->lp[i]); sp_moogladder_init(p->sp,p->lp[i]);
        sp_crossfade_create(&p->mix[i]); sp_crossfade_init(p->sp,p->mix[i]);
    }
    return p;
}
void source_s1_delay_free(source_s1_exact_delay *p) {
    if(!p)return;
    for(unsigned i=0;i<4;i++)sp_vdelay_destroy(&p->line[i]);
    for(unsigned i=0;i<2;i++){sp_moogladder_destroy(&p->lp[i]);sp_crossfade_destroy(&p->mix[i]);}
    sp_destroy(&p->sp); free(p);
}
void source_s1_delay_reset(source_s1_exact_delay *p) {
    for(unsigned i=0;i<4;i++)sp_vdelay_reset(p->sp,p->line[i]);
}
void source_s1_delay_tap(source_s1_exact_delay *p,float t,float fb,float l,float r,float *left,float *right) {
    for(unsigned i=0;i<4;i++){p->line[i]->del=i<2?t*2.f:t;p->line[i]->feedback=fb;}
    float o=0, fill=0;
    sp_vdelay_compute(p->sp,p->line[0],&l,left);
    sp_vdelay_compute(p->sp,p->line[1],&r,&o);
    sp_vdelay_compute(p->sp,p->line[2],&r,&fill);
    sp_vdelay_compute(p->sp,p->line[3],&o,right);
    *right+=fill;
}
void source_s1_delay_insert(source_s1_exact_delay *p,float t,float fb,float ft,float cutoff,float ratio,
     float mix,float enable,float l,float r,float *left,float *right) {
    float x=l,y=r,a,b;
    if(ft==0.f) {
        const float lo=log2f(1024.f),hi=log2f(22050.f);
        float pos=log2f(cutoff);
        if(pos<lo)pos=lo;if(pos>hi)pos=hi;
        const float norm=(pos-lo)/(hi-lo);
        const float factor=1.f+norm*(ratio-1.f);
        float hz=cutoff*factor;
        if(hz<64.f)hz=64.f;if(hz>22050.f)hz=22050.f;
        p->lp[0]->freq=p->lp[1]->freq=hz;
        p->lp[0]->res=p->lp[1]->res=0.f;
        sp_moogladder_compute(p->sp,p->lp[0],&l,&x);
        sp_moogladder_compute(p->sp,p->lp[1],&r,&y);
    }
    source_s1_delay_tap(p,t,fb,x,y,&a,&b);
    p->mix[0]->pos=p->mix[1]->pos=mix*enable;
    sp_crossfade_compute(p->sp,p->mix[0],&l,&a,left);
    sp_crossfade_compute(p->sp,p->mix[1],&r,&b,right);
}
