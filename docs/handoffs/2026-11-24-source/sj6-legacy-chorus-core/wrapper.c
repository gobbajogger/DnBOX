/* Classic Build9 scoped host wrapper: real legacy musical modes I and II only. */
#include "wrapper.h"
#include <math.h>
#include <string.h>

#define WRAPPER_PI2 6.283185307179586
#define WRAPPER_RATE 44100.
static float ramper(classic_ramper *r,uint64_t at)
{
    if(r->pending!=r->target){
        r->target=r->pending;r->start=at;r->initial=r->current;
    }
    if(r->current==r->target)return r->current;
    uint64_t dt=at-r->start;
    if(dt>=r->duration){r->current=r->target;r->start=0;}
    else if(at>=r->start)r->current=fmaf(r->target-r->initial,(float)dt/(float)r->duration,r->initial);
    return r->current;
}
void classic_wrapper_prepare(classic_mode_i *s,const float incoming[4],unsigned retained_ramp_frames)
{
    memset(s,0,sizeof *s);
    for(unsigned i=0;i<4;++i){
        classic_ramper *r=s->p+i;
        r->pending=r->target=r->current=r->initial=incoming[i];
        r->duration=retained_ramp_frames;
    }
}
void classic_wrapper_start(classic_mode_i *s,int started)
{
    /* Actual start/stop virtuals only store DSP byte +41. */
    s->started=!!started;
}
int classic_wrapper_command(classic_mode_i *s,unsigned id,float u)
{
    if(!(u>=0.f&&u<=1.f))return 0;
    switch(id){
      case 42:s->p[0].pending=(float)(16.*pow((double)u,3.33));return 1;
      case 43:s->p[1].pending=(float)(.97*(double)u);return 1;
      case 44:s->p[3].pending=u;return 1;
      default:return 0;
    }
}
void classic_wrapper_process(classic_mode_i *s,uint64_t at,float *l,float *r,size_t n)
{
    if(!s->started)return; /* Original disabled branch copies dry unchanged. */
    for(size_t off=0;off<n;off+=8){
        const uint64_t t=at+off;
        const float rate=ramper(s->p,t),depth=ramper(s->p+1,t);
        const float feedback=ramper(s->p+2,t),mix=ramper(s->p+3,t);
        const float increment=(float)((double)rate/WRAPPER_RATE);
        const double omega=(double)rate*WRAPPER_PI2;
        const float a=sinf((float)(omega*(double)t/WRAPPER_RATE));
        const float b=sinf((float)(omega*(double)((float)t+.2f)/WRAPPER_RATE));
        const float width=s->mode_ii ? .02f : .015f;
        const float fb[]={feedback+fmaf(a,.05f,1.f)*width,
                          feedback+fmaf(b,.05f,1.f)*-width};
        size_t end=off+8<n ? off+8 : n;
        for(size_t i=off;i<end;++i) {
            float y[2];classic_i_frame(&s->core,l[i],r[i],depth,increment,fb,mix,y);
            /* Legacy app booster after active TempoChorus; OFF is unity. */
            l[i]=y[0]*(float)1.707;r[i]=y[1]*(float)1.707;
        }
    }
}
void classic_wrapper_enter_mode_ii(classic_mode_i *s)
{
    // Original musical Mode II setter: start legacy graph, select ±.02,
    // read back known .863 Hz, project through inverse/Float32/forward,
    // and reapply authored default depth/mix. Keep current phase/ramps.
    const float u=(float)pow(.863/16.,1./3.33);
    s->mode_ii=1;
    classic_wrapper_start(s,1);
    (void)classic_wrapper_command(s,42,u);
    (void)classic_wrapper_command(s,43,(float)(.97*.202));
    (void)classic_wrapper_command(s,44,(float).505);
}
