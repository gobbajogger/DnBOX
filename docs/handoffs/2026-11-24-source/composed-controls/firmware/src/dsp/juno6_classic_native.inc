/* SPDX-License-Identifier: GPL-3.0-only */
/* Build 9 classic TempoChorus role: true stereo, 3.518..24 ms
 * quadrature modulation, retained stopped state and active 1.707 booster. */
#if X0X_SJ6_NATIVE_CLASSIC
void juno6_classic_native_config(juno6_chorus_t *c,const uint8_t b[3])
{
    sj6_classic_native_t *s=&c->classic_native;
    unsigned m=c->factory_mode;
    s->typed=1;
    if(m<JUNO6_CHORUS_I || m>JUNO6_CHORUS_BOTH)return; /* OFF: freeze, do not reapply targets. */
    if(s->mode==m && !memcmp(s->code,b,3))return;
    static const float mode_rate[]={.513f,.863f,1.726f};
    float target[3]={
        b[0]?16.f*fm_powf((float)(b[0]-1u)/127.f,3.33f):mode_rate[m-JUNO6_CHORUS_I],
        b[1]?.97f*((float)(b[1]-1u)/127.f):(.97f*.97f*.202f),
        b[2]?((float)(b[2]-1u)/127.f):(m==JUNO6_CHORUS_BOTH?.555f:.505f)
    };
    memcpy(s->code,b,3);
    for(unsigned i=0;i<3;++i){
        if(!s->mode)s->current[i]=target[i];
        if(s->target[i]!=target[i]){
            s->target[i]=target[i];
            s->step[i]=(target[i]-s->current[i])*(1.f/1250.f);
            s->left[i]=1250u;
        }
    }
    s->mode=(uint8_t)m;
}
static inline float classic_native_tap(const juno6_chorus_t *c,const sj6_classic_native_t *s,unsigned ch,float lag)
{
    const unsigned n=(unsigned)lag;
    if(n>s->filled)return 0.f;
    int index=(int)s->pos-(int)n;
    if(index<0)index+=SJ6_CLASSIC_NATIVE_RING;
    const float a=c->mode_history[ch][index], b=n==s->filled?0.f:
        c->mode_history[ch][index?index-1:SJ6_CLASSIC_NATIVE_RING-1];
    return (a+(b-a)*(lag-(float)n))*(1.f/4096.f);
}
static inline float classic_native_lfo(float p)
{
    if(p>=1.f)p-=1.f;
    float x=p*256.f;
    unsigned i=(unsigned)x;
    float a=fm_sinf((float)i*(TWO_PI/256.f));
    float b=fm_sinf((float)((i+1u)&255u)*(TWO_PI/256.f));
    return a+(b-a)*(x-(float)i);
}
void juno6_classic_native_process(juno6_chorus_t *c,float *l,float *r,unsigned frames)
{
    sj6_classic_native_t *s=&c->classic_native;
    unsigned m=c->factory_mode;
    if(m<JUNO6_CHORUS_I || m>JUNO6_CHORUS_BOTH)return;
    const float ms=c->sample_rate*.001f, inv_sr=1.f/c->sample_rate;
    for(unsigned i=0;i<frames;++i){
        if(!s->quantum){
            for(unsigned k=0;k<3u;++k)if(s->left[k])
                s->current[k]=--s->left[k]?s->current[k]+s->step[k]:s->target[k];
            s->quantum=8;
        }
        --s->quantum;
        const float phase=s->phase, mix=s->current[2];
        const float amp=(24.f-3.518f)*.5f*s->current[1], center=(24.f+3.518f)*.5f;
        const float a=classic_native_tap(c,s,0,(center+amp*classic_native_lfo(phase))*ms);
        const float b=classic_native_tap(c,s,1,(center+amp*classic_native_lfo(phase+.25f))*ms);
        const float x=l[i], y=r[i];
        /* Corrected source uses feedback *= 1 + .05*sin(...). At the
         * typed Init native carrier raw feedback is exactly zero; the
         * original shadow mode-offset assignments are not injected. */
        c->mode_history[0][s->pos]=(int16_t)(fm_clampf(x,-7.999f,7.999f)*4096.f);
        c->mode_history[1][s->pos]=(int16_t)(fm_clampf(y,-7.999f,7.999f)*4096.f);
        if(++s->pos>=SJ6_CLASSIC_NATIVE_RING)s->pos=0;
        if(s->filled<SJ6_CLASSIC_NATIVE_RING-1u)++s->filled;
        s->phase+=s->current[0]*inv_sr;
        if(s->phase>=1.f)s->phase-=1.f;
        l[i]=1.707f*(x+(a-x)*mix);
        r[i]=1.707f*(y+(b-y)*mix);
    }
}
#endif
