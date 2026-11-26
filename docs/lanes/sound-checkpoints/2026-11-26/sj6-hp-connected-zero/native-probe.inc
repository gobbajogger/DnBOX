/* Emulator-only boot cue: runs actual selected runtime FX using the two
 * genuine engine owner addresses before init makes those addresses live. */
#include "../../../firmware/hal/fm1_time.h"
struct sj6_route_cue_receipt { uint32_t magic,digest,ticks,ticks_min,n; };
volatile struct sj6_route_cue_receipt sj6_route_cue_receipt;
static uint32_t sj6_cue_word(float x)
{
    union {float f;uint32_t u;} w;w.f=x;return w.u;
}
void sg6_zero_signed_cue(original_runtime_part *p,original_runtime_part *other)
{
    const float evidence[]={10.f,0.f,.44387006759643555f,0.f,0.f,0.f,0.f,1.f};
    original_runtime_part *owners[2]={p,other};
    for(unsigned v=0;v<2u;++v){
        original_runtime_part *q=owners[v];
        q->kind=ORIGINAL_SOUND_JUNO;
        q->status=ORIGINAL_RUNTIME_OK;
        juno6_init(&q->dsp.juno.synth,44100.f);
        juno6_set_sj6_hpf(&q->dsp.juno.synth,0.f);
        juno6_chorus_init(&q->dsp.juno.chorus,44100.f);
        q->dsp.juno.chorus.factory_mode=JUNO6_CHORUS_OFF;
        q->dsp.juno.crush.step=1.f;q->dsp.juno.crush.active=0;
        if(sj6_zero_leaf_admit(q) || !engine_sj6_zero_pin(q,evidence))return;
    }
    sj6_zero_fx_channel *a=sj6_zero_leaf_admit(p),*b=sj6_zero_leaf_admit(other);
    if(!a||!b||a==b)return;
    float l[32],r[32];
    uint32_t x=0x48276db3u,digest=2166136261u,t=0,min=UINT32_MAX;
    for(unsigned block=0;block<32u;++block){
        for(unsigned i=0;i<32u;++i){
            x=x*1664525u+1013904223u;l[i]=(float)((int32_t)(x>>16)-32768)*0x1p-16f;
            x=x*1664525u+1013904223u;r[i]=(float)((int32_t)(x>>16)-32768)*0x1p-16f;
        }
        uint32_t start=fm1_ticks();
        sj6_run_effects(p,l,r,32,NULL);
        uint32_t elapsed=fm1_ticks()-start;
        t+=elapsed;if(elapsed<min)min=elapsed;
        for(unsigned i=0;i<32u;++i){
            digest=(digest^sj6_cue_word(l[i]))*16777619u;
            digest=(digest^sj6_cue_word(r[i]))*16777619u;
        }
    }
    if(a[0].hp_y==0.f || b[0].hp_y!=0.f || b[1].crusher_y!=0.f)return;
    /* Real clear keeps proof but resets only this owner; retire revokes it. */
    if(original_runtime_all_off(p)!=ORIGINAL_RUNTIME_OK || !sj6_zero_leaf_admit(p) ||
       a[0].hp_y!=0.f || a[1].crusher_y!=0.f)return;
    if(original_runtime_retire(p)!=ORIGINAL_RUNTIME_OK || engine_sj6_zero_leaf(p) ||
       !sj6_zero_leaf_admit(other))return;
    if(original_runtime_retire(other)!=ORIGINAL_RUNTIME_OK)return;
    sj6_route_cue_receipt.digest=digest;
    sj6_route_cue_receipt.ticks=t;
    sj6_route_cue_receipt.ticks_min=min;
    sj6_route_cue_receipt.n=1024u;
    sj6_route_cue_receipt.magic=0x535a4658u;
}
