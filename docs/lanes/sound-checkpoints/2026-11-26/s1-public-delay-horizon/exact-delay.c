/* Host sidecar for per-sample Synth One original delay topology. */
#include "exact-delay.h"
#include "soundpipe.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
struct dnbox_s1_exact_delay {
    sp_data *sp;
    sp_vdelay *line[4];
    unsigned physical_at[4];
    sp_moogladder *lp[2];
    sp_crossfade *mix[2];
};
dnbox_s1_exact_delay *dnbox_s1_delay_new(unsigned rate) {
    dnbox_s1_exact_delay *p=calloc(1,sizeof *p);
    if(!p)return 0;
    sp_create(&p->sp); p->sp->sr=rate; p->sp->nchan=2;
    for(unsigned i=0;i<4;i++){
        sp_vdelay_create(&p->line[i]);
        const float history_seconds=i<2 ? (5.f+8.f/(float)rate) : (2.5f+8.f/(float)rate);
        sp_vdelay_init(p->sp,p->line[i],history_seconds);
    }
    for(unsigned i=0;i<2;i++){
        sp_moogladder_create(&p->lp[i]); sp_moogladder_init(p->sp,p->lp[i]);
        sp_crossfade_create(&p->mix[i]); sp_crossfade_init(p->sp,p->mix[i]);
    }
    return p;
}
void dnbox_s1_delay_free(dnbox_s1_exact_delay *p) {
    if(!p)return;
    for(unsigned i=0;i<4;i++)sp_vdelay_destroy(&p->line[i]);
    for(unsigned i=0;i<2;i++){sp_moogladder_destroy(&p->lp[i]);sp_crossfade_destroy(&p->mix[i]);}
    sp_destroy(&p->sp); free(p);
}
void dnbox_s1_delay_reset(dnbox_s1_exact_delay *p) {
    for(unsigned i=0;i<4;i++)sp_vdelay_reset(p->sp,p->line[i]);
    for(unsigned i=0;i<4;i++)p->physical_at[i]=0;
}
size_t dnbox_s1_delay_history_bytes(const dnbox_s1_exact_delay *p) {
    size_t n=0;
    for(unsigned i=0;i<4;i++)n+=p->line[i]->buf.size;
    return n;
}
/* Match original Soundpipe interpolation including its unusual integer-age
 * branch. Keep the original 10s cursor as logical state; only the actual
 * full-rate physical ring addresses are reduced. */
static void physical_vdelay(dnbox_s1_exact_delay *p,unsigned k,float in,float *out)
{
    sp_vdelay *l=p->line[k];
    const int32_t limit=(uint32_t)(l->maxdel*p->sp->sr);
    const int32_t logical=(uint32_t)(10.f*p->sp->sr);
    const int32_t index=(int32_t)p->physical_at[k];
    float *buf=l->buf.ptr;
    buf[index]=in+l->prev*l->feedback;
    float f=l->del*(-1.0*p->sp->sr);
    int32_t shift=(int32_t)f;
    f-=(float)shift;
    if((int32_t)l->left+shift<0 || f<0.f){f++;shift--;}
    int32_t b=index+shift;
    if(b<0)b+=limit;
    if(b<0 || b>=limit)abort();
    const int32_t a=b==0?limit-1:b-1;
    const int32_t c=b==limit-1?0:b+1;
    const int32_t d=c==limit-1?0:c+1;
    float w,x,y,z;
    z=f*f;z--;z*=0.1666666667;
    y=f;y++;w=(y*=0.5);w--;
    x=3.0*z;y-=x;w-=z;x-=f;
    *out=(w*buf[a]+x*buf[b]+y*buf[c]+z*buf[d])*f+buf[b];
    l->prev=*out;
    if(++l->left==logical)l->left=0;
    p->physical_at[k]=index+1==limit?0:index+1;
}
void dnbox_s1_delay_tap(dnbox_s1_exact_delay *p,float t,float fb,float l,float r,float *left,float *right) {
    if(!(t>=0.f && t<=2.5f))abort(); /* diagnostic API contract, never clamp controls */
    for(unsigned i=0;i<4;i++){p->line[i]->del=i<2?t*2.f:t;p->line[i]->feedback=fb;}
    float o=0, fill=0;
    physical_vdelay(p,0,l,left);
    physical_vdelay(p,1,r,&o);
    physical_vdelay(p,2,r,&fill);
    physical_vdelay(p,3,o,right);
    *right+=fill;
}
void dnbox_s1_delay_insert(dnbox_s1_exact_delay *p,float t,float fb,float ft,float cutoff,float ratio,
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
    dnbox_s1_delay_tap(p,t,fb,x,y,&a,&b);
    p->mix[0]->pos=p->mix[1]->pos=mix*enable;
    sp_crossfade_compute(p->sp,p->mix[0],&l,&a,left);
    sp_crossfade_compute(p->sp,p->mix[1],&r,&b,right);
}
