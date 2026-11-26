/* Host sidecar for per-sample Synth One original delay topology. */
#include "exact-delay.h"
#include "soundpipe.h"
#include <math.h>
#include <stdlib.h>
struct dnbox_s1_exact_delay {
    sp_data *sp;
    sp_vdelay *line[4];
    sp_moogladder *lp[2];
    sp_crossfade *mix[2];
};
dnbox_s1_exact_delay *dnbox_s1_delay_new(unsigned rate) {
    dnbox_s1_exact_delay *p=calloc(1,sizeof *p);
    if(!p)return 0;
    if(!rate){free(p);return 0;}
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
void dnbox_s1_delay_free(dnbox_s1_exact_delay *p) {
    if(!p)return;
    for(unsigned i=0;i<4;i++)sp_vdelay_destroy(&p->line[i]);
    for(unsigned i=0;i<2;i++){sp_moogladder_destroy(&p->lp[i]);sp_crossfade_destroy(&p->mix[i]);}
    sp_destroy(&p->sp); free(p);
}
void dnbox_s1_delay_reset(dnbox_s1_exact_delay *p) {
    for(unsigned i=0;i<4;i++)sp_vdelay_reset(p->sp,p->line[i]);
}
/* The original four lines always advance together and have the same
 * size, with exactly two independent delay ages. Reuse only their control
 * geometry; preserve each feedback write and the signed right crossfeed. */
typedef struct { int a,b,c,d; float w,x,y,z,f; } tap_geom;
static __attribute__((noinline)) void geometry(tap_geom *gp,float t,int sr,int at,int n)
{
    tap_geom g;
    /* The integer rate is exactly binary32; binary64 represents the full
       product of the two binary32 operands before the original narrowing. */
    float f=t*(-(float)sr);
    int i=(int)f;
    f-=(float)i;
    i+=at;
    if(i<0 || f<0.f){
        f++;i--;
        while(i<0)i+=n;
    }else while(i>=n)i-=n;
    int c=i==n-1?0:i+1;
    g.a=i==0?n-1:i-1;g.b=i;g.c=c;g.d=c==n-1?0:c+1;
    float w,x,y,z;
    z=f*f;z--;z*=0.1666666667;
    y=f;y++;w=(y*=0.5);w--;
    x=3.0*z;y-=x;w-=z;x-=f;
    g.w=w;g.x=x;g.y=y;g.z=z;g.f=f;
    *gp=g;
}
static __attribute__((noinline)) float lane(sp_vdelay *p,const tap_geom *gp,int at,float in,float fb)
{
    const tap_geom g=*gp;
    float *buf=p->buf.ptr;
    buf[at]=in+p->prev*fb;
    float a=buf[g.a],b=buf[g.b],c=buf[g.c],d=buf[g.d];
    const float o=(g.w*a+g.x*b+g.y*c+g.z*d)*g.f+b;
    p->prev=o;
    return o;
}
void dnbox_s1_delay_tap(dnbox_s1_exact_delay *p,float t,float fb,float l,float r,float *left,float *right) {
    const int at=p->line[0]->left;
    const int n=(uint32_t)(p->line[0]->maxdel*p->sp->sr);
    tap_geom twice,once; geometry(&twice,t*2.f,p->sp->sr,at,n);
    geometry(&once,t,p->sp->sr,at,n);
    *left=lane(p->line[0],&twice,at,l,fb);
    const float o=lane(p->line[1],&twice,at,r,fb);
    const float fill=lane(p->line[2],&once,at,r,fb);
    *right=lane(p->line[3],&once,at,o,fb);
    *right+=fill;
    p->line[0]->left=at+1==n?0:at+1;
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
