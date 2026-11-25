/* SPDX-License-Identifier: GPL-3.0-only */
#if X0X_SJ6_NATIVE_LIMITER
#include "../dsp/fastmath.h"
static float sj6_limiter_unit(unsigned slot,unsigned code)
{
    /* Code 0 means not authored. Code 1..128 means direct detent 0..127. */
    static const float defaults[SJ6_LIM_COUNT]={.45f,0.f,0.f,.5f};
    return code ? (float)(code-1u)*(1.f/127.f) : defaults[slot];
}
unsigned sj6_limiter_position(unsigned slot,unsigned code)
{
    return code ? code-1u : (unsigned)(127.f*sj6_limiter_unit(slot,0)+.5f);
}
float sj6_limiter_value(unsigned slot,unsigned code)
{
    float u=sj6_limiter_unit(slot,code);
    switch(slot){
    case SJ6_LIM_THRESH:return -30.f+40.f*u;
    case SJ6_LIM_HEADROOM:return .1f+19.9f*u*fm_sqrtf(u);
    case SJ6_LIM_ATTACK:return .001f+.199f*u;
    default:return -10.f+22.f*u;
    }
}
void original_runtime_sj6_limiter(original_runtime_part *p,const uint8_t b[SJ6_LIM_COUNT])
{
    if(!p || p->kind!=ORIGINAL_SOUND_JUNO || p->status!=ORIGINAL_RUNTIME_OK)return;
    sj6_limiter_t *s=&p->dsp.juno.limiter;
    unsigned any=0;
    for(unsigned k=0;k<SJ6_LIM_COUNT;++k){if(b[k]>128u)return;any|=b[k];}
    if(!any){s->armed=0;return;}
    if(!s->armed)s->gain=1.f;
    s->armed=1;
    const float per_db=.16609640474f;
    s->threshold=fm_exp2f(per_db*sj6_limiter_value(SJ6_LIM_THRESH,b[SJ6_LIM_THRESH]));
    const float head=fm_exp2f(per_db*sj6_limiter_value(SJ6_LIM_HEADROOM,b[SJ6_LIM_HEADROOM]));
    s->inverse_knee=1.f/(s->threshold*(head-1.f));
    s->attack=1.f-fm_expf(-1.f/(44100.f*sj6_limiter_value(SJ6_LIM_ATTACK,b[SJ6_LIM_ATTACK])));
    s->output_gain=fm_exp2f(per_db*sj6_limiter_value(SJ6_LIM_GAIN,b[SJ6_LIM_GAIN]));
}
/* Build9 ID17 controls the same individual stage as Threshold and Gain,
 * but writes both via its own ranges and gates the effect at .01. */
static void sj6_rage_apply(sj6_limiter_t *s,float u)
{
    const float db=.16609640474f;
    const float width=s->threshold>0.f && s->inverse_knee>0.f?
        1.f/(s->threshold*s->inverse_knee):.01157945f;
    s->threshold=fm_exp2f(db*(-20.f+30.f*u));
    s->inverse_knee=1.f/(s->threshold*width);
    s->output_gain=fm_exp2f(db*(1.f+7.f*u));
    if(!(s->attack>0.f))s->attack=.02242f;
    if(!(s->gain>0.f))s->gain=1.f;
    s->armed=u>.01f;
}
void original_runtime_sj6_rage(original_runtime_part *p,unsigned position)
{
    if(p && p->kind==ORIGINAL_SOUND_JUNO && p->status==ORIGINAL_RUNTIME_OK && position<=127u)
        sj6_rage_apply(&p->dsp.juno.limiter,(float)position*(1.f/127.f));
}
static void sj6_limiter_process(sj6_limiter_t *s,float *l,float *r,unsigned frames)
{
    if(!s->armed)return;
    float gain=s->gain;
    for(unsigned i=0;i<frames;++i){
        const float peak=fm_maxf(fm_fabsf(l[i]),fm_fabsf(r[i]));
        float target=1.f;
        if(peak>s->threshold){
            const float over=peak-s->threshold;
            const float soft=s->threshold+over/(1.f+s->inverse_knee*over);
            target=soft/peak;
        }
        /* Stereo-linked gain changes, true attack, fixed compact recovery.
         * Original Bonus Release label invokes ATTACK, not this recovery. */
        gain+=(target-gain)*(target<gain?s->attack:.0004534119f);
        const float applied=s->output_gain*gain;
        l[i]*=applied;r[i]*=applied;
    }
    s->gain=gain;
}
#endif
