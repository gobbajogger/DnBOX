#include "dnbx_core.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static dnbx_part src, old;
static float a[128],b[128],c[128],d[128];
static void dial(dnbx_part *s,unsigned leg,unsigned rate,float pan,float resonance,float sub)
{
    assert(dnbx_init_superj6(s,(float)rate)==DNBX_OK);
    for(unsigned j=0;j<2;j++){
        dnbx_layer_params x;
        assert(dnbx_get_layer(s,j,&x)==DNBX_OK);
        x.gain=1.f;x.pan=j==leg?pan:0.f;
        x.cutoff_hz=20000.f;x.resonance=j==leg?resonance:0.f;
        x.filter_env_hz=0.f;x.sub_gain=j==leg?sub:0.f;x.waveform=0.f;
        x.oscillator_mode=DNBX_OSC_SUPERJ6;
        x.amp.attack_s=.002f;x.amp.decay_s=.002f;x.amp.sustain=1.f;x.amp.release_s=.008f;
        assert(dnbx_set_layer(s,j,&x)==DNBX_OK);
    }
#if X0X_SJ6_NOISE_SUB_NATIVE
    s->sj6_noise_gain=s->sj6_sub_gain=0.f;
#endif
}
static void arm(dnbx_part *s)
{
#if defined(X0X_HOST) && DNBX_HOST_SJ6_MONO_SOURCE_ROUTE
    /* Real controls above succeeded. No public knob maps to this flag. */
    s->sj6_host_mono_source=1;
#else
    (void)s;
#endif
}
static float one(const dnbx_part *s){(void)s;return 0.808f;}
static int go(dnbx_part *s,unsigned leg,float *l,float *r,float oth,float bend,float vel)
{
    float g=one(s);
    return dnbx_render_pitch_osc_levels_velocity(s,l,r,128,bend,leg?oth:g,leg?g:oth,vel);
}
static void same(unsigned leg,float oth,float bend,float vel,unsigned blocks)
{
    for(unsigned j=0;j<blocks;j++){
        int g=go(&src,leg,a,b,oth,bend,vel);
        assert(g==go(&old,leg,c,d,oth,bend,vel));
        assert(g==DNBX_OK);
        assert(memcmp(a,c,sizeof a)==0&&memcmp(b,d,sizeof b)==0);
    }
}
static void single(unsigned l)
{
    dial(&src,l,44100,0.f,0.f,0.f);old=src;
    assert(dnbx_note_on(&src,69,87)==DNBX_OK && dnbx_note_on(&old,69,87)==DNBX_OK);
    same(l,0,1,0,1); /* default unarmed render */
    arm(&src);
    double newer=0,legacy=0;
    float max=0;
    for(unsigned t=0;t<80;t++){
        assert(go(&src,l,a,b,0,1,0)==DNBX_OK && go(&old,l,c,d,0,1,0)==DNBX_OK);
        for(unsigned k=0;k<128;k++){
            assert(isfinite(a[k])&&a[k]==b[k]);
#if defined(X0X_HOST) && DNBX_HOST_SJ6_MONO_SOURCE_ROUTE
            assert(fabsf(a[k]-12.f*c[k])<5e-7f);
#else
            assert(a[k]==c[k]);
#endif
            if(t>20){if(fabsf(a[k])>max)max=fabsf(a[k]);newer+=(double)a[k]*a[k];legacy+=(double)c[k]*c[k];}
        }
    }
    assert(max<.406f && max>0.f);
    printf("OSC%u centre isolated | level delta %.9f dB | peak %.9f\n",l+1,10.*log10(newer/legacy),max);
    assert(dnbx_note_off(&src,69)==DNBX_OK && dnbx_note_off(&old,69)==DNBX_OK);
    for(unsigned t=0;t<1500 && dnbx_active_voices(&old);t++){
       assert(go(&src,l,a,b,0,1,0)==DNBX_OK && go(&old,l,c,d,0,1,0)==DNBX_OK);
#if defined(X0X_HOST) && DNBX_HOST_SJ6_MONO_SOURCE_ROUTE
       for(unsigned k=0;k<128;k++) assert(fabsf(a[k]-12.f*c[k])<5e-7f);
#endif
    }
    assert(dnbx_active_voices(&src)==0 && dnbx_active_voices(&old)==0);
    same(l,0,1,0,2);
    for(unsigned k=0;k<128;k++)assert(a[k]==0.f&&b[k]==0.f);
}
static void no_effect_checks(void)
{
    for(unsigned kind=0;kind<11;kind++){
        dial(&src,0,kind==5?48000:44100,kind==0?.12f:0.f,kind==1?.1f:0.f,kind==2?.03f:0.f);
        assert(dnbx_note_on(&src,69,87)==DNBX_OK);
        if(kind==6)assert(dnbx_store_raw(&src,136,.5f)==DNBX_RAW_STORED_ONLY);
        if(kind==7)assert(dnbx_store_raw(&src,131,.5f)==DNBX_RAW_STORED_ONLY);
        if(kind==8)src.voice[0].layer[1].filter_first=.25f;
        if(kind==9){assert(dnbx_all_off(&src)==DNBX_OK);}
#if X0X_SJ6_NOISE_SUB_NATIVE
        if(kind==10)src.sj6_noise_gain=.01f;
#else
        if(kind==10)src.layer[0].params.waveform=.5f;
#endif
        old=src;arm(&src);
        same(0,kind==3?.23f:0.f,kind==4?1.1f:1.f,0,kind==8?1:6);
    }
    dial(&src,0,44100,0.f,0.f,0.f);assert(dnbx_note_on(&src,69,87)==DNBX_OK);
    old=src;arm(&src);
    for(unsigned t=0;t<4;t++){
        assert(dnbx_render_pitch_osc_levels_velocity(&src,a,b,128,1.f,0.f,0.f,0.f)==DNBX_OK);
        assert(dnbx_render_pitch_osc_levels_velocity(&old,c,d,128,1.f,0.f,0.f,0.f)==DNBX_OK);
        assert(memcmp(a,c,sizeof a)==0&&memcmp(b,d,sizeof b)==0);
        for(unsigned k=0;k<128;k++)assert(a[k]==0.f&&b[k]==0.f);
    }
    assert(dnbx_render_pitch_osc_levels(&src,a,b,128,1.f,1.01f,0.f)==DNBX_E_RANGE);
}
static void poly(void)
{
    for(unsigned sync=0;sync<2;sync++){
        dial(&src,0,44100,0.f,0.f,0.f);
        const unsigned keys[]={48,52,55,60,64,67};
        for(unsigned i=0;i<6;i++) {
            assert(dnbx_note_on(&src,(uint8_t)keys[i],127)==(int)i);
            assert(src.voice[i].note==keys[i]);
        }
        assert(dnbx_active_voices(&src)==6);
        /* Independent deterministic coherent safety negative control. Keep all
         * six distinct note identities, but align their oscillator clocks. */
        if(sync)for(unsigned i=1;i<6;i++)
            src.voice[i].layer[0].increment=src.voice[0].layer[0].increment;
        old=src;arm(&src);
        float peak=0;
        for(unsigned i=0;i<80;i++){
            same(0,0,1,0,1);
            for(unsigned k=0;k<128;k++)if(fabsf(a[k])>peak)peak=fabsf(a[k]);
        }
        assert(peak<1);
        if(sync)assert(peak*12.f>1.f);
        printf("six distinct notes %s | fallback peak %.9f | hypothetical x12 %.9f\n",
               sync?"coherent":"natural",peak,peak*12.f);
    }
}
static void clip(void)
{
    float peak=0;
    for(unsigned l=0;l<2;l++)for(unsigned note=0;note<128;note++){
        dial(&src,l,44100,0.f,0.f,0.f);arm(&src);
        assert(dnbx_note_on(&src,(uint8_t)note,127)==DNBX_OK);
        for(unsigned j=0;j<8;j++){
            assert(go(&src,l,a,b,0,1,0)==DNBX_OK);
            for(unsigned k=0;k<128;k++){
                assert(isfinite(a[k])&&a[k]==b[k]);
                if(fabsf(a[k])>peak)peak=fabsf(a[k]);
            }
        }
    }
    assert(peak<.42f);
    printf("all 256 one-Sine 127-velocity fixtures single-path peak %.9f; no clipping\n",peak);
}
static void transition(void)
{
    const unsigned keys[]={48,52,55,60,64};
    const unsigned counts[]={1,3,6,3,1};
    unsigned layers=1;
    dial(&src,0,44100,0.f,0.f,0.f);
    assert(dnbx_note_on(&src,69,87)==0);
    old=src;arm(&src);
    for(unsigned w=0;w<5;w++){
        const unsigned want=counts[w];
        while(layers<want){
            assert(dnbx_note_on(&src,keys[layers-1],87)==(int)layers);
            assert(dnbx_note_on(&old,keys[layers-1],87)==(int)layers);
            ++layers;
        }
        while(layers>want){
            --layers;
            assert(dnbx_note_off(&src,keys[layers-1])==DNBX_OK);
            assert(dnbx_note_off(&old,keys[layers-1])==DNBX_OK);
        }
        unsigned settle=0;
        while(dnbx_active_voices(&src)!=want && settle++<1500){
            assert(go(&src,0,a,b,0,1,0)==DNBX_OK);
            assert(go(&old,0,c,d,0,1,0)==DNBX_OK);
        }
        assert(dnbx_active_voices(&src)==want);
        assert(src.voice[0].gate&&src.voice[0].note==69);
        assert(go(&src,0,a,b,0,1,0)==DNBX_OK);
        assert(go(&old,0,c,d,0,1,0)==DNBX_OK);
        float scale=1.f;
#if defined(X0X_HOST) && DNBX_HOST_SJ6_MONO_SOURCE_ROUTE
        if(want==1)scale=12.f;
#endif
        double ss=0,oo=0;
        for(unsigned k=0;k<128;k++){
            assert(fabsf(a[k]-scale*c[k])<5e-7f);
            ss+=(double)a[k]*a[k];oo+=(double)c[k]*c[k];
        }
        printf("continuously held key69, total=%u | delta=%.8f dB | release settle blocks=%u\n",
               want,10*log10(ss/oo),settle);
    }
}
int main(void){
    single(0);single(1);no_effect_checks();poly();clip();transition();
    puts("centre, both OSCs, release, muted, panned/nonzero/alternate refusals, six voice, clip checks PASS");
}
