#!/usr/bin/env python3
"""Inject original-source room downstream of actual DnBOX voice+delay, no recorded feedback input."""
from pathlib import Path
import sys
base=Path(sys.argv[1])
def exact(s,a,b):
    assert s.count(a)==1,a
    return s.replace(a,b)
p=base/'dsp/s1c/s1c.h';s=p.read_text()
s=exact(s,'    int32_t wpos, wlen;','    int32_t wpos, wlen;\n    unsigned host_width_pos;\n    float host_width[2205];\n#if S1_STEREO_EXACT\n    void *host_exact_stereo;\n#endif\n#if S1_ROOM_EXACT\n    void *host_exact_room;\n#endif\n#if S1_DELAY_EXACT\n    struct dnbox_s1_exact_delay *host_exact_delay;\n    float host_delay_mono;\n#endif')
s=exact(s,'    float lp[5], bq[2];','    float lp[5], bq[2];\n#if S1_AUX_EXACT\n    void *host_exact_aux;\n#endif\n#if S1_MORPH_EXACT\n    uint32_t host_morph_phase[2];\n#endif\n#if S1_FILTER_EXACT\n    void *host_exact_filter;\n#endif')
p.write_text(s)
p=base/'dsp/s1c/s1c.c';s=p.read_text()
s=exact(s,'static void room_tap(const float*a){','''#include "exact-room.h"
#include "exact-delay.h"
#include "exact-filter.h"
#include "exact-morph.h"
#include "exact-aux.h"
#include "exact-stereo.h"
static void room_tap(const float*a){''')
s=exact(s,' static FILE*f;if(!f)f=fopen(getenv("ROOM_TRACE"),"wb");',' static FILE*f; const char *trace=getenv("ROOM_TRACE"); if(!trace)return; if(!f)f=fopen(trace,"wb");')
a='        /* ping-pong delay with tracking low-pass on its input */\n'
s=exact(s,a,a+"""#if S1_DELAY_EXACT
        if(!s->host_exact_delay){
            s->host_exact_delay=dnbox_s1_delay_new((unsigned)s->sr);
            if(!s->host_exact_delay)abort();
        }
        const float mono_now=P(MONO);
        if(mono_now!=s->host_delay_mono){
            dnbox_s1_delay_reset(s->host_exact_delay);
            s->host_delay_mono=mono_now;
        }
        float mL,mR;
        dnbox_s1_delay_insert(s->host_exact_delay,P(DLT),P(DLFB),P(FTYPE),P(CUT),P(DICT),
                              P(DLMIX),P(DLON),L,R,&mL,&mR);
#else
""")
a='        /* reverb: high-pass, pre-gain, input compressor, network, wet compressor */\n'
s=exact(s,a,'#endif /* S1_DELAY_EXACT */\n'+a+'''#if S1_ROOM_EXACT
        if(!s->host_exact_room)s->host_exact_room=exact_room_new(s->sr);
        float hl=mL,hr=mR,wl=0.f,wr=0.f;
        exact_room_hp(s->host_exact_room,P(RVHP),&hl,&hr);
        exact_room_comp(s->host_exact_room,ROOM_INPUT_COMP,P(CRR),P(CRT),P(CRA),P(CRREL),P(CRG),&hl,&hr);
        const float initialL=hl,initialR=hr;
        exact_room_return(s->host_exact_room,P(RVFB),hl,hr,&wl,&wr);
        const float roomL=wl,roomR=wr;
        exact_room_comp(s->host_exact_room,ROOM_WET_COMP,P(CWR),P(CWT),P(CWA),P(CWREL),P(CWG),&wl,&wr);
#else
''')
a='        float rm = P(RVMIX) * P(RVON) * lfo10(s, P(LRVMIX));'
s=exact(s,a,'#endif /* S1_ROOM_EXACT */\n'+a)
a='        L = ((1.0f - rm) * mL + rm * wl) * 2.0f * P(MVOL);\n        R = ((1.0f - rm) * mR + rm * wr) * 2.0f * P(MVOL);'
s=exact(s,a,'''#if S1_ROOM_EXACT
        L=(mL*(1.f-rm)+wl*rm)*(2.f*P(MVOL));
        R=(mR*(1.f-rm)+wr*rm)*(2.f*P(MVOL));
#else
'''+a+'\n#endif')
a='        L = comp(s->cmp[0][0], L, &cm) * P(CMG); R = comp(s->cmp[0][1], R, &cm) * P(CMG);'
s=exact(s,a,'''#if S1_ROOM_EXACT && S1_ROOM_EXACT_MASTER
        exact_room_comp(s->host_exact_room,ROOM_MASTER_COMP,P(CMR),P(CMT),P(CMA),P(CMREL),P(CMG),&L,&R);
#else
'''+a+'\n#endif')
needle='    y *= v->amp * v->kkey;\n    float fo;'
s=exact(s,needle,needle.replace('    float fo;',"""#if S1_FILTER_EXACT
    if(!v->host_exact_filter)v->host_exact_filter=exact_voice_filter_new(s->sr);
    float cut=P(CUT)*lfo10(s,P(LCUT)), amount=P(FENV)*lfo10(s,P(LFENV));
    cut=clampf(cut-cut*amount*(1.f-v->fenv),64.f,22050.f);
    const float res=clampf(P(RES)*lfo10(s,P(LRES)),0.f,.98f);
    return exact_voice_filter(v->host_exact_filter,y,cut,res,P(FTYPE),P(FMIX));
#endif
    float fo;"""))
a="""    float o1 = morph(&v->ph[0], v->kd1, v->ki1, P(I1), bl) * v->vamp * P(VOL1);
    float o2 = morph(&v->ph[1], v->kd2, v->ki2, P(I2), bl) * v->vamp * P(VOL2);"""
s=exact(s,a,"""#if S1_MORPH_EXACT
    const float base=P(MONO)>0.f?s->monos:v->freq;
    const float plfo=1.f+lfo01(s,P(LPITCH))*0.0594630944f;
    const float mul=P(DETM)*plfo;
    float hz1=base*(float)exp2((int)P(SEMI1)/12.f);
    float hz2=base*(float)exp2((int)P(SEMI2)/12.f);
    hz1=clampf(hz1*mul,0.f,.5f*s->sr);
    hz2*=mul;
    const float dscale=v->freq/261.6255653006f;
    hz2+=((int)P(LDET)?lfo01(s,P(LDET)):1.f)*P(DET2)*dscale;
    hz2=clampf(hz2,0.f,.5f*s->sr);
    const float o1=(exact_morph_sample(&v->host_morph_phase[0],hz1,P(I1),bl)*v->vamp)*P(VOL1);
    const float o2=(exact_morph_sample(&v->host_morph_phase[1],hz2,P(I2),bl)*v->vamp)*P(VOL2);
#else
"""+a+"""
#endif""")
a="static const uint8_t smooth_bits[19]"
s=exact(s,a,"""#if S1_LFO_EXACT
static void source_lfo_tick(s1u_lfo *l,float rate,float shape,float amount,float c2,float sr){
    float raw=l->ph;
    l->ph+=rate*(1.f/sr);
    if(l->ph>=1.f)l->ph-=1.f;
    else if(l->ph<0.f)l->ph+=1.f;
    if(shape==0.f)raw=(float)sin((double)raw*3.14159265358979323846*2.0);
    else if(shape==1.f)raw=raw>.5f?1.f:-1.f;
    else if(shape==2.f)raw=(raw-.5f)*2.f;
    else if(shape==3.f)raw=(.5f-raw)*2.f;
    l->sm=(1.f-c2)*raw+c2*l->sm;
    l->o01=.5f*(1.f+l->sm)*amount;
    l->o10=1.f-(.5f*(1.f+-l->sm)*amount);
}
#endif
"""+a)
a='    s->lfo_c2 = powf(0.5f, 1.0f / (0.0053125f * sr));'
s=exact(s,a,"""#if S1_LFO_EXACT
    s->lfo_c2=(float)pow(0.5,(double)((1.f/sr)/.0053125f));
#else
"""+a+"\n#endif")
a="""        s1u_lfo_tick(&s->lfo[0], P(L1R), (int)P(L1I), P(L1A), lc2, s->sr);
        s1u_lfo_tick(&s->lfo[1], P(L2R), (int)P(L2I), P(L2A), lc2, s->sr);"""
s=exact(s,a,"""#if S1_LFO_EXACT
        source_lfo_tick(&s->lfo[0],P(L1R),P(L1I),P(L1A),lc2,s->sr);
        source_lfo_tick(&s->lfo[1],P(L2R),P(L2I),P(L2A),lc2,s->sr);
#else
"""+a+"\n#endif")
a='        /* bitcrush (sample-and-hold), tremolo */\n'
s=exact(s,a,a+"""#if S1_CRUSH_EXACT
        /* Source steps the sample-index accumulator in two ordered
         * binary32 operations, including when there is no LFO. */
        float rate_log=(float)log2((double)P(BCSR));
        rate_log+=4.f*lfo01(s,P(LBC));
        const float native_rate=clampf((float)exp2((double)rate_log),2048.f,48000.f);
        s1u_crush *cr=&s->crush;
        cr->inc=s->sr/native_rate;
        if(cr->inc<1.f)cr->inc=1.f;
        if(cr->idx<=cr->n){
            cr->v=y;cr->idx+=cr->inc;cr->idx-=cr->n;cr->n=0.f;
        }else y=cr->v;
        cr->n+=1.f;
#else
""")
a='        y *= lfo10(s, P(LTREM));'
s=exact(s,a,'#endif /* S1_CRUSH_EXACT */\n'+a)
a="""    float sub = sine_t(v->ph[2]);
    v->ph[2] += v->kis;
    y += P(SUBSQ) != 0.0f ? (sub > 0.0f ? P(SUBV) : -P(SUBV)) : sub * v->vamp * P(SUBV) * 3.0f;
    if (P(FMV) != 0.0f) {
        float mod = sine_t(v->mph) * v->kff * v->kidx;
        v->mph += v->kif;
        y += sine_t(v->cph) * v->vamp * P(FMV);
        v->cph += ph_inc((v->kff + mod) / s->sr);
    }"""
s=exact(s,a,"""#if S1_AUX_EXACT
    if(!v->host_exact_aux)v->host_exact_aux=exact_aux_new();
    const float ab=P(MONO)>0.f?s->monos:v->freq;
    const float lf=(1.f+lfo01(s,P(LPITCH))*0.0594630944f);
    const float subf=clampf(ab*(P(DETM)/(2.f*(1.f+P(SUBOCT)))*lf),0.f,.5f*s->sr);
    const float fmf=clampf(ab*(P(DETM)*lf),0.f,.5f*s->sr);
    y+=exact_aux_sample(v->host_exact_aux,subf,fmf,v->vamp,P(SUBSQ),P(SUBV),
                        clampf(P(FMA)*lfo10(s,P(LFM)),0.f,15.f),P(FMV));
#else
"""+a+"""
#endif""")
marker='        /* autopan, equal power */\n        float L, R;'
s=exact(s,marker,marker+"""
#if S1_STEREO_EXACT
        if(!s->host_exact_stereo)s->host_exact_stereo=exact_stereo_new();
        exact_stereo(s->host_exact_stereo,y,P(PANF),P(PANA),P(PHMIX),P(PHRATE),P(PHW),P(PHFB),&L,&R);
#else""")
marker='        s1u_phaser_run(&s->ph, &L, &R, P(PHMIX), P(PHRATE), P(PHW), P(PHFB), s->sr);'
s=exact(s,marker,marker+'\n#endif /* S1_STEREO_EXACT */')
old_noise="""    if (P(NOISEV) != 0.0f) {
        s->rnd = s->rnd * 1664525u + 1013904223u;
        y += ((float)(int32_t)s->rnd * (1.0f / 2147483648.0f)) * v->vamp * P(NOISEV) * lfo10(s, P(LNOISE));
    }"""
assert s.count(old_noise)==2
s=s.replace(old_noise,"""#if S1_NOISE_EXACT
    /* This original generator clocks on every active voice, even at zero
     * noise volume. Reset to the same sp_data seed (zero) at engine init. */
    s->rnd=(1103515245u*s->rnd+12345u)&0x7fffffffu;
    const float unit=(float)s->rnd*(1.f/2147483648.f);
    const float noise=(unit*2.f-1.f)*v->vamp;
    y+=(noise*P(NOISEV))*lfo10(s,P(LNOISE));
#else
"""+old_noise+"""
#endif""")
s=exact(s,'    s->sr = sr; s->rnd = 0x1234567u;',
        '    s->sr=sr; s->rnd=S1_NOISE_EXACT ? 0u : 0x1234567u;')
start=s.index('        const float wsample[]={L,R,P(WIDEN)};')
end=s.index('        outl[f] = L; outr[f] = R;',start)
s=s[:start]+"""        if(s->sr!=44100.f)abort();
        const unsigned pos=s->host_width_pos;
        const float delayed=s->host_width[pos];
        s->host_width[pos]=R;
        s->host_width_pos=pos+1u==2205u?0u:pos+1u;
        const float width=P(WIDEN);
        R=width*delayed+(1.f-width)*R;
"""+s[end:]
p.write_text(s)
p=base/'part.c';s=p.read_text()
s='#include "exact-room.h"\n#include "exact-delay.h"\n#include "exact-filter.h"\n#include "exact-morph.h"\n#include "exact-aux.h"\n#include "exact-stereo.h"\n'+s
a='    load_init(e);'
# first unconditional program load, not the decode error branch (which matches a contextual line)
s=exact(s,a+'\n','''#if S1_ROOM_EXACT
    exact_room_drop(e->host_exact_room);
#endif
#if S1_DELAY_EXACT
    dnbox_s1_delay_free(e->host_exact_delay);
#endif
#if S1_FILTER_EXACT
    for(unsigned i=0;i<S1C_VOICES;++i)exact_voice_filter_drop(e->v[i].host_exact_filter);
#endif
#if S1_AUX_EXACT
    for(unsigned i=0;i<S1C_VOICES;++i)exact_aux_drop(e->v[i].host_exact_aux);
#endif
#if S1_STEREO_EXACT
    exact_stereo_drop(e->host_exact_stereo);
#endif
'''+a+'\n')
p.write_text(s)
p=base/'render.c';s=p.read_text()
s=exact(s,'    if(argc!=5)return 2;',"    if(argc!=5)return 2;\n#if S1_MORPH_EXACT\n    if(!exact_morph_load())return 6;\n#endif")
s='#include "exact-room.h"\n#include "exact-delay.h"\n#include "exact-filter.h"\n#include "exact-morph.h"\n#include "exact-aux.h"\n#include "exact-stereo.h"\n'+s
a='    free(pack);'
s=exact(s,a,'''#if S1_ROOM_EXACT
    exact_room_drop(part.e.host_exact_room);
#endif
#if S1_DELAY_EXACT
    dnbox_s1_delay_free(part.e.host_exact_delay);
#endif
#if S1_FILTER_EXACT
    for(unsigned i=0;i<S1C_VOICES;++i)exact_voice_filter_drop(part.e.v[i].host_exact_filter);
#endif
#if S1_AUX_EXACT
    for(unsigned i=0;i<S1C_VOICES;++i)exact_aux_drop(part.e.v[i].host_exact_aux);
#endif
#if S1_STEREO_EXACT
    exact_stereo_drop(part.e.host_exact_stereo);
#endif
'''+a)
p.write_text(s)
