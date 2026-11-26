#include "../../../firmware/hal/fm1_time.h"
struct sj6_route_cue_receipt {uint32_t magic,layersize,partsize, test[4][4];};
volatile struct sj6_route_cue_receipt sj6_route_cue_receipt;
static void scope_prepare(juno6_t *s)
{
    juno6_init(s,44100.f);
    s->osc1_level_code=128;s->osc2_level_code=1;
    juno6_set_sj6_hpf(s,0.f);
    for(unsigned l=0;l<2;l++){
        dnbx_layer_params p;
        (void)dnbx_get_layer(&s->core,l,&p);
        p.cutoff_hz=20000.f;
        p.resonance=0.f;
        p.pulse_width=.5f;
        p.waveform=0.f;
        p.amp.attack_s=.002f;
        p.amp.decay_s=.002f;
        p.amp.sustain=1.f;
        p.amp.release_s=.002f;
        p.filter_env_hz=0.f;
        p.oscillator_mode=DNBX_OSC_SUPERJ6;
        (void)dnbx_set_layer(&s->core,l,&p);
    }
#if DNBX_HOST_SJ6_FILTER_MODE
    s->core.sj6_mode_test.armed=1;
    s->core.sj6_mode_test.step=1.f/(44100.f*.005f);
    s->core.sj6_mode_test.gain=1.f;
    s->core.sj6_mode_test.b0=0x1.851d9c7caa4aap-1;
    s->core.sj6_mode_test.a1=0x1.7cb2883c57ccap+0;
    s->core.sj6_mode_test.a2=0x1.1b116179f9916p-1;
#endif
}
static void scope_coeff(juno6_t *s,unsigned low)
{
    /* Actual current Normal coefficients at common tracking zero, scoped to
     * this private identical two-receiver fixture. Not live coefficient law. */
    for(unsigned l=0;l<2;l++)s->core.layer[l].params.cutoff_hz=low?300.f:20000.f;
#if DNBX_HOST_SJ6_FILTER_MODE
    s->core.sj6_mode_test.b0=low?0x1.cb5f067432c00p-12:0x1.851d9c7caa4aap-1;
    s->core.sj6_mode_test.a1=low?-0x1.ea91fe81f06abp+0:0x1.7cb2883c57ccap+0;
    s->core.sj6_mode_test.a2=low?0x1.d609ac871aeedp-1:0x1.1b116179f9916p-1;
#endif
}
void sg6_zero_signed_cue(original_runtime_part *p,original_runtime_part *other,float cue[4][256])
{
    juno6_t *a=&p->dsp.juno.synth, *b=&other->dsp.juno.synth;
    for(unsigned mode=0;mode<4;mode++){
        scope_prepare(a);scope_prepare(b);
        unsigned count=mode>=2?6:1;
        for(unsigned l=0;l<count;l++){
            if(dnbx_note_on(&a->core,57+l,87)<0||dnbx_note_on(&b->core,69+l,87)<0)return;
        }
        if(dnbx_active_voices(&a->core)!=count || dnbx_active_voices(&b->core)!=count)return;
        for(unsigned i=0;i<8;i++){
            if(!juno6_render_stereo(a,cue[0],cue[1],256)||!juno6_render_stereo(b,cue[2],cue[3],256))return;
        }
        uint32_t sum=0,mx=0,d=2166136261u, edges=0;
        unsigned n=mode==1?16:256;
        unsigned limit=mode==1?64:8;
        unsigned pm=0;
        for(unsigned i=0;i<limit;i++){
            if(mode==1){
                if(i==0||i==6)dnbx_set_sj6_ota(&a->core,1),dnbx_set_sj6_ota(&b->core,1);
                if(i==4||i==40)dnbx_set_sj6_ota(&a->core,0),dnbx_set_sj6_ota(&b->core,0);
                if(i==28||i==56)scope_coeff(a,i==28),scope_coeff(b,i==28);
            }
            if(mode==3&&i==0)scope_coeff(a,1),scope_coeff(b,1);
            uint32_t t=fm1_ticks();
            if(!juno6_render_stereo(a,cue[0],cue[1],n)||!juno6_render_stereo(b,cue[2],cue[3],n))return;
            t=fm1_ticks()-t;
            if (pm!=a->core.sj6_ota){pm=a->core.sj6_ota;edges=(edges<<8)|i;}
            if (a->core.sj6_ota!=b->core.sj6_ota)return;
            sum+=t;if(t>mx)mx=t;
            for(unsigned q=0;q<4;q++)for(unsigned k=0;k<n;k++){
                union {float f;uint32_t u;} u={.f=cue[q][k]};
                d=(d^u.u)*16777619u;
            }
        }
        sj6_route_cue_receipt.test[mode][0]=sum;
        sj6_route_cue_receipt.test[mode][1]=mx;
        sj6_route_cue_receipt.test[mode][2]=d;
        sj6_route_cue_receipt.test[mode][3]=edges;
    }
    sj6_route_cue_receipt.layersize=sizeof(dnbx_layer_voice);
    sj6_route_cue_receipt.partsize=sizeof(original_runtime_part);
    sj6_route_cue_receipt.magic=0x535a4658u;
}
