/* SPDX-License-Identifier: GPL-3.0-only */
/* Build 9 J60 V2 core control and BBD topology. Keep its 96 kHz phase,
 * cascaded input biquads and dual-channel delay/interpolation independent
 * from Legacy Chorus. At the device rate use a bounded interpolating bridge,
 * not the AU's large, 64-tap arbitrary-rate converter. */
#if X0X_SJ6_NATIVE_J60
void juno6_j60_config(juno6_chorus_t *c,const uint8_t b[4])
{
    sj6_j60_t *s=&c->j60;
    if(!b[0]) {s->selected=0;return;}
    if(!s->selected) {
        memset(s,0,sizeof *s);
#if X0X_SJ6_NATIVE_CLASSIC
        /* The other version may next resume its controls and phase, but
         * must not read 96 kHz BBD samples as the Classic input signal. */
        c->classic_native.filled=c->classic_native.pos=0;
#endif
        s->selected=1;
        s->rate=s->depth=s->mix=1.f;
    }
    s->target[0]=b[1] ? (float)(b[1]-1u)*(2.f/127.f):1.f;
    s->target[1]=b[2] ? (float)(b[2]-1u)*(2.f/127.f):1.f;
    s->target[2]=b[3] ? (float)(b[3]-1u)/127.f:1.f;
}
static inline int16_t j60_quantize(float x)
{
    /* Internal ring scale accommodates the wet analog feedback excursion. */
    return (int16_t)(fm_clampf(x,-7.999f,7.999f)*4096.f);
}
static inline float j60_ring(const juno6_chorus_t *c,const sj6_j60_t *s,unsigned ch,float delay)
{
    unsigned d=(unsigned)delay;
    if(d>s->filled)return 0.f;
    float a=c->mode_history[ch][(s->write-d)&1023u];
    float b=d<s->filled?c->mode_history[ch][(s->write-d-1u)&1023u]:0.f;
    return (a+(b-a)*(delay-(float)d))*(1.f/4096.f);
}
static inline float j60_biquad(float x,float z[2])
{
    float y=.0792125556f*x+z[0];
    z[0]=.158425111f*x+1.21644445f*y+z[1];
    z[1]=.0792125556f*x-.533294672f*y;
    return y;
}
static void j60_core(juno6_chorus_t *c,sj6_j60_t *s,unsigned mode,const float in[2])
{
    const float ramp=1.f/1920.f; /* source 20 ms dezipper */
    s->rate+=(s->target[0]-s->rate)*ramp;
    s->depth+=(s->target[1]-s->depth)*ramp;
    s->mix+=(s->target[2]-s->mix)*ramp;
    const float hz=mode==3u?9.2f:mode==2u?.82f:.48f;
    float phase=s->phase+s->rate*(2.f*hz+(s->phase<0.f?-.048f:.048f))*(1.f/96000.f);
    if(phase>1.f)phase-=2.f;
    s->phase=phase;
    const float center=mode==3u?314.799988f:142.f;
    s->center+=(center-s->center)*.0000304996283f;
    float displacement=mode==3u?28.8f:384.f;
    for(unsigned ch=0;ch<2u;ch++) {
        float f=j60_biquad(j60_biquad(in[ch],s->lp[ch][0]),s->lp[ch][1]);
        float h=.9990192f*(f-s->hp_x[ch])+.9980384f*s->hp_y[ch];
        s->hp_x[ch]=f;s->hp_y[ch]=h;
        c->mode_history[ch][s->write]=j60_quantize(h);
        float ph=phase+((!ch && mode!=3u)?1.f:0.f);
        if(ph>1.f)ph-=2.f;
        float d=4.f+s->center+s->depth*displacement*.967567563f*fm_fabsf(ph);
        float p=j60_ring(c,s,ch,d)-1.53846154f*s->post0[ch]-s->post1[ch];
        s->post0[ch]+=.533294672f*p;
        s->post1[ch]+=.533294672f*s->post0[ch];
        s->wet[ch]=s->post1[ch];
    }
    s->write=(uint16_t)((s->write+1u)&1023u);
    if(s->filled<1023u)++s->filled;
}
void juno6_j60_process(juno6_chorus_t *c,float *left,float *right,unsigned frames)
{
    sj6_j60_t *s=&c->j60;
    unsigned m=c->factory_mode;
    if(m<JUNO6_CHORUS_I||m>JUNO6_CHORUS_BOTH){s->reset=1;return;}
    const float ratio=96000.f/c->sample_rate;
    if(s->reset){
        s->filled=s->write=0;s->phase=0.f;s->bridge=0.f;
        memset(s->lp,0,sizeof s->lp);
        for(unsigned q=0;q<2u;++q)s->hp_x[q]=s->hp_y[q]=s->post0[q]=s->post1[q]=s->wet[q]=s->prev[q]=0.f;
        s->reset=0;
    }
    for(unsigned k=0;k<frames;++k){
        const float exact[2]={left[k],right[k]};
        float next=1.f-s->bridge;
        while(next<=ratio){
            float fraction=next/ratio;
            float lerp[2]={s->prev[0]+(exact[0]-s->prev[0])*fraction,
                           s->prev[1]+(exact[1]-s->prev[1])*fraction};
            j60_core(c,s,m-JUNO6_CHORUS_OFF,lerp);
            next+=1.f;
        }
        s->bridge=ratio-next+1.f;
        s->prev[0]=exact[0];s->prev[1]=exact[1];
        if(s->target[2]==0.f && s->mix<.00001f)s->mix=0.f;
        if(s->mix!=0.f){
            left[k]=1.3f*exact[0]+1.17f*s->mix*s->wet[0];
            right[k]=1.3f*exact[1]+1.17f*s->mix*s->wet[1];
        }
    }
}
#endif
