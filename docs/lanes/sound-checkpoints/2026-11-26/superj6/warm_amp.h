/* Host source parity seam for the older SJ6 sampler's smooth (type 1) Main
 * AHDSHR. Time inputs are post-receiver seconds, before CoreSampler's /16
 * conversion. No preset identity, sample offsets, or phase/filter operations.
 * Caller owns actual render block boundaries and voice/legato policy.
 * Compile IEEE Float32/Float64 with no fast math. */
#ifndef SJ6_WARM_AMP_H
#define SJ6_WARM_AMP_H
#include <math.h>
#include <stdbool.h>

typedef struct {
    double from,to,bend;
    int ticks;
} sj6_amp_segment;
typedef struct {
    sj6_amp_segment s[8];
    double y,to,add,mul;
    float out,endpoint,delta;
    unsigned remain;
    int stage,count,ticks;
    bool up,hold,linear;
} sj6_warm_amp;
static inline void sj6_amp_config(sj6_warm_amp *a,float sr,float attack,float hold,
                                float decay,float sustain,float sustain_release_hold,float release)
{
    const float r=sr/16.0f;
    *a=(sj6_warm_amp){0};
    a->s[0]=(sj6_amp_segment){0,0,0,-1};
    a->s[1]=(sj6_amp_segment){1,0,0,(int)((double)r*.01)};
    a->s[2]=(sj6_amp_segment){0,1,0x1.c8f87724b5c1dp-3,(int)(attack*r)};
    a->s[3]=(sj6_amp_segment){1,1,0,(int)(hold*r)};
    a->s[4]=(sj6_amp_segment){1,sustain,0x1.d037e1b1c8ff7p-8,(int)(decay*r)};
    a->s[5]=(sj6_amp_segment){sustain,sustain,0,-1};
    a->s[6]=(sj6_amp_segment){sustain,sustain,0,(int)(sustain_release_hold*r)};
    a->s[7]=(sj6_amp_segment){sustain,0,0x1.d037e1b1c8ff7p-8,(int)(release*r)};
    a->hold=a->linear=true;
    a->ticks=-1;
}
/* Original application panel transfer (0x1004902a8). Call only with
 * valid normalized [0,1] Float32 controls; keep multiply and add separate
 * in the build, e.g. -ffp-contract=off. These are not linearized MIDI words. */
static inline float sj6_amp_panel(float raw,double low,double high,double exponent)
{
    const double scaled=(high-low)*pow((double)raw,exponent);
    return (float)(low+scaled);
}
static inline void sj6_amp_config_panel(sj6_warm_amp *a,float sr,float attack,float decay,
                                       float sustain,float release)
{
    sj6_amp_config(a,sr,sj6_amp_panel(attack,.002,16,3.33),0,
        sj6_amp_panel(decay,.002,24,3.35),sj6_amp_panel(sustain,0,1,2.5),0,
        sj6_amp_panel(release,.002,24,3.35));
}
static inline void sj6_amp_segment_set(sj6_warm_amp *a,int n,double from)
{
    const sj6_amp_segment d=a->s[n];
    a->stage=n;
    a->to=d.to;
    a->y=d.ticks>0 ? from : d.to;
    a->up=d.to>from;
    a->hold=from==d.to;
    a->linear=d.bend<=0.0;
    if (a->hold) {a->count=0; a->ticks=d.ticks; return;}
    if (a->linear) {a->mul=d.ticks>0 ? (d.to-from)/(double)d.ticks : 0.0;return;}
    if (!d.ticks) {a->mul=0; a->add=d.to;return;}
    a->mul=exp(-log((d.bend+fabs(d.to-from))/d.bend)/(double)d.ticks);
    a->add=(d.to+(a->up ? d.bend : -d.bend))*(1-a->mul);
}
static inline void sj6_amp_notify(sj6_warm_amp *a,int n,bool carry)
{
    while(n<8&&!a->s[n].ticks) ++n;
    if(n>=8)n=0;
    const sj6_amp_segment d=a->s[n];
    sj6_amp_segment_set(a,n,carry&&d.from!=d.to ? a->y : d.from);
}
static inline void sj6_amp_on(sj6_warm_amp *a) {sj6_amp_notify(a,2,false);}
static inline void sj6_amp_restart(sj6_warm_amp *a) {sj6_amp_notify(a,1,true);}
static inline void sj6_amp_off(sj6_warm_amp *a)
{
    a->s[6].from=a->s[6].to=a->s[7].from=(double)(float)(a->hold ? a->to : a->y);
    sj6_amp_notify(a,6,true);
}
static inline bool sj6_amp_cross(sj6_warm_amp *a)
{
    if(a->hold) return a->ticks>=0 && ++a->count>=a->ticks;
    a->y=a->linear ? a->y+a->mul : fma(a->mul,a->y,a->add);
    return a->up ? a->y>=a->to : a->y<=a->to;
}
/* Return true only on the actual stage-1 completion boundary. */
static inline bool sj6_amp_prep(sj6_warm_amp *a,unsigned frames)
{
    const int started=a->stage;
    double end=a->to;
    bool crossed=sj6_amp_cross(a);
    if(crossed){
        a->y=a->to;    /* original clamps, then initializes one next segment */
        const int n=started<7 ? started+1 : 0;
        sj6_amp_segment_set(a,n,a->s[n].from);
        if(started==1){
            /* This prep also performs the first new attack attempt. */
            if(sj6_amp_cross(a)){
                end=a->to; a->y=end;
                sj6_amp_segment_set(a,3,a->s[3].from);
            } else end=a->hold ? a->to : a->y;
        }
    } else if(!a->hold)end=a->y;
    if(a->stage==5)end=a->s[5].to;
    a->endpoint=(float)end;
    a->remain=frames;
    a->delta=frames ? (a->endpoint-a->out)/(float)frames : 0.0f;
    if (!frames) a->out=a->endpoint;
    return started==1 && crossed;
}
static inline float sj6_amp_sample(sj6_warm_amp *a)
{
    if(a->remain){a->out+=a->delta; --a->remain;}
    return a->out;
}
#endif
