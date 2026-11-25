/* SPDX-License-Identifier: GPL-3.0-only
 * SuperJ6 Build 9 Unison (distinct post-instrument effect, not Widen).
 * The source graph fans out one stereo signal; the compact receiver shares
 * the stereo input history between otherwise independent pitch-shift taps.
 */
#if X0X_SJ6_NATIVE_UNISON
void juno6_unison_config(juno6_t *s, const uint8_t b[4], float hist[2][SJ6_UNISON_HISTORY])
{
    juno6_unison_t *u=&s->unison;
    if (u->history==hist && !memcmp(u->code,b,4)) return;
    if (u->history!=hist || !u->code[0]) {
        u->pos=u->filled=0;
        memset(u->head,0,sizeof u->head);
    }
    memcpy(u->code,b,4);
    u->history=hist;
    unsigned type=b[3], voices=type<4u ? type+2u : 6u;
    if(!b[0]) { u->taps=0; return; }
    static const float pan[]={1.f,-.95f,.5f,-.59f,.33f};
    static const float graph_gain[]={.55f,.86f,1.25f,1.65f,2.1f,2.4f};
    float detune=b[1] ? .5f*fm_powf((float)(b[1]-1u)/127.f,2.5f):.01f;
    float stereo=(float)b[2]/127.f;
    unsigned old=u->taps;
    u->taps=(uint8_t)(voices-1u);
    const float d=detune/(float)(voices-1u), x=detune*.02f*(float)(voices-1u);
    for(unsigned i=0;i<u->taps;++i) {
        float semitones=type==5u ? (d+3.f*x)*((i&1u)?-1.f:1.25f) :
                                               (d+x)*((i&1u)?-1.f:1.f);
        u->step[i]=1.f-fm_exp2f((float)(i+1u)*semitones*.0833333f);
        u->pan[i]=pan[i]*stereo;
        if(i>=old)u->head[i]=0.f;
    }
    u->wet=graph_gain[type]/(float)voices;
    /* Dry graph unity when disabled is intentionally the established firmware
     * bypass, rather than applying the original graph's always-on trim. */
    u->trim=(.5f+graph_gain[type]/3.f)*1.2f;
    u->dry=1.f-1.f/(float)voices;
}
static inline float unison_read(const juno6_unison_t *u,unsigned ch,float lag)
{
    unsigned l=(unsigned)lag;
    if(l>u->filled)return 0.f;
    const float a=u->history[ch][(u->pos-l)&(SJ6_UNISON_HISTORY-1u)];
    const float b=l>=u->filled?0.f:u->history[ch][(u->pos-l-1u)&(SJ6_UNISON_HISTORY-1u)];
    return a+(b-a)*(lag-(float)l);
}
void juno6_unison_process(juno6_t *s,float *l,float *r,unsigned n)
{
    juno6_unison_t *u=&s->unison;
    if(!u->taps || !u->history)return;
    for(unsigned k=0;k<n;++k) {
        const float in_l=l[k], in_r=r[k];
        u->history[0][u->pos]=in_l;
        u->history[1][u->pos]=in_r;
        float wet_l=0.f,wet_r=0.f;
        for(unsigned i=0;i<u->taps;++i) {
            float h=u->head[i]+u->step[i];
            if(h>=1000.f)h-=1000.f;
            if(h<0.f)h+=1000.f;
            u->head[i]=h;
            const float fade=h<10.f?h*.1f:1.f;
            float a=unison_read(u,0,h), b=unison_read(u,1,h);
            if(fade<1.f) {
                a=a*fade+unison_read(u,0,h+1000.f)*(1.f-fade);
                b=b*fade+unison_read(u,1,h+1000.f)*(1.f-fade);
            }
            const float pan=u->pan[i];
            wet_l+=a*(pan>0.f?1.f-pan:1.f);
            wet_r+=b*(pan<0.f?1.f+pan:1.f);
        }
        l[k]=(u->dry*in_l+u->wet*wet_l)*u->trim;
        r[k]=(u->dry*in_r+u->wet*wet_r)*u->trim;
        u->pos=(uint16_t)((u->pos+1u)&(SJ6_UNISON_HISTORY-1u));
        if(u->filled<SJ6_UNISON_HISTORY-1u)++u->filled;
    }
}
#endif
