#ifndef DNBX_HOST_SJ6_FILTER_ADSR_H
#define DNBX_HOST_SJ6_FILTER_ADSR_H
#include "warm_amp.h"
/* Separate source type 1 Filter EG. Six descriptors, unlike AMP AHDSHR.
 * This state emits the cutoff control at source prep boundaries rather
 * than interpolating gain across audio samples. */
typedef sj6_warm_amp sj6_filter_adsr;
static inline void sj6_filter_config(sj6_filter_adsr *a,float sr,
                                     float attack,float decay,float sustain,float release)
{
    const float r=sr/16.f;
    *a=(sj6_filter_adsr){0};
    a->s[0]=(sj6_amp_segment){0,0,0,-1};
    a->s[1]=(sj6_amp_segment){1,0,0,(int)((double)r*.01)};
    a->s[2]=(sj6_amp_segment){0,1,0x1.c8f87724b5c1dp-3,(int)(attack*r)};
    a->s[3]=(sj6_amp_segment){1,sustain,0x1.d037e1b1c8ff7p-8,(int)(decay*r)};
    a->s[4]=(sj6_amp_segment){sustain,sustain,0,-1};
    a->s[5]=(sj6_amp_segment){sustain,0,0x1.d037e1b1c8ff7p-8,(int)(release*r)};
    a->hold=a->linear=true;
    a->ticks=-1;
}
static inline void sj6_filter_notify(sj6_filter_adsr *a,int stage,bool carry)
{
    while(stage<6&&!a->s[stage].ticks)++stage;
    if(stage>=6)stage=0;
    const sj6_amp_segment d=a->s[stage];
    const double from=carry&&d.from!=d.to?a->y:d.from;
    sj6_amp_segment_set(a,stage,from);
}
static inline void sj6_filter_on(sj6_filter_adsr *a){sj6_filter_notify(a,2,false);}
static inline void sj6_filter_restart(sj6_filter_adsr *a){sj6_filter_notify(a,1,true);}
static inline void sj6_filter_off(sj6_filter_adsr *a)
{
    a->s[5].from=(double)(float)(a->hold?a->to:a->y);
    sj6_filter_notify(a,5,true);
}
static inline float sj6_filter_prep(sj6_filter_adsr *a)
{
    if(sj6_amp_cross(a)){
        const double end=a->to;
        a->y=end;
        sj6_filter_notify(a,a->stage+1,false);
        return (float)end;
    }
    return (float)(a->hold?a->to:a->y);
}
#endif
