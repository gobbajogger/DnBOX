#include "../../../firmware/hal/fm1_time.h"
struct sj6_route_cue_receipt { uint32_t magic, state_size, coeff_size, out[4][4]; };
volatile struct sj6_route_cue_receipt sj6_route_cue_receipt;
static int prep_full(dnbx_part *s,unsigned shape,unsigned who)
{
    if(dnbx_init_superj6(s,44100)!=DNBX_OK)return 0;
    for(unsigned l=0;l<2;l++){
      dnbx_layer_params a;
      if(dnbx_get_layer(s,l,&a))return 0;
      a.waveform=shape/3.f;a.oscillator_mode=DNBX_OSC_SUPERJ6;
      a.cutoff_hz=20000.f;a.resonance=0.f;a.amp.attack_s=.002f;
      a.amp.decay_s=.002f;a.amp.sustain=1.f;a.amp.release_s=.002f;
      a.filter_env_hz=0.f;a.sub_gain=0.f;a.gain=1.f;a.pan=0.f;a.pulse_width=.5f;
      if(dnbx_set_layer(s,l,&a))return 0;
    }
    dnbx_set_sj6_sources(s,0,(const uint8_t[2]){0});
    const uint8_t keys[2][6]={{45,57,64,76,88,100},{50,61,69,81,93,105}};
    for(unsigned i=0;i<6;i++)if(dnbx_note_on(s,keys[who][i],87)<0)return 0;
    unsigned n=0;
    for(unsigned i=0;i<DNBX_VOICES;++i)n+=s->voice[i].gate!=0;
    return n==6;
}
void sg6_zero_signed_cue(original_runtime_part *p,original_runtime_part *other,float cue[4][256])
{
    dnbx_part *s[2]={&p->dsp.juno.synth.core,&other->dsp.juno.synth.core};
    for(unsigned mode=0;mode<4;mode++){
      uint32_t max=0,sum=0,d=2166136261u,mn=UINT32_MAX;
      for(unsigned j=0;j<2;j++)if(!prep_full(s[j], mode>>1,j)){sj6_route_cue_receipt.magic=0xDEAD0001;return;}
      for(unsigned shot=0;shot<9;shot++){
        const float ratio=mode&1?(shot&1?.8908987f:1.0594631f):1.f;
        uint32_t start=fm1_ticks();
        int a=dnbx_render_pitch_osc_levels_velocity(s[0],cue[0],cue[1],256,ratio,.808f,.808f,0.f);
        int b=dnbx_render_pitch_osc_levels_velocity(s[1],cue[2],cue[3],256,ratio,.808f,.808f,0.f);
        uint32_t elapsed=fm1_ticks()-start;
        if(a!=DNBX_OK||b!=DNBX_OK){sj6_route_cue_receipt.magic=0xDEAD0002;return;}
        for(unsigned q=0;q<4;q++)for(unsigned i=0;i<256;i++){
           union {float f;uint32_t u;} w={.f=cue[q][i]};d=(d^w.u)*16777619u;
        }
        if(shot){sum+=elapsed;if(elapsed>max)max=elapsed;if(elapsed<mn)mn=elapsed;}
      }
      sj6_route_cue_receipt.out[mode][0]=sum;sj6_route_cue_receipt.out[mode][1]=max;
      sj6_route_cue_receipt.out[mode][2]=mn;sj6_route_cue_receipt.out[mode][3]=d;
    }
    memset(s[0],0,sizeof(*s[0]));memset(s[1],0,sizeof(*s[1]));
    sj6_route_cue_receipt.state_size=sizeof(dnbx_part);
    sj6_route_cue_receipt.coeff_size=sizeof(s[0]->voice[0].layer[0].oscillator_history);
    sj6_route_cue_receipt.magic=0x535a4658u;
}
