#include "dnbx_core.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
float carrier_tap[2][128],raw_tap[2][128];
static dnbx_part p[2];
static float o[2][128];
static void controls(dnbx_part *s,unsigned l,unsigned wave,float width)
{
    dnbx_layer_params a;assert(!dnbx_get_layer(s,l,&a));a.waveform=wave/3.f;
    a.oscillator_mode=DNBX_OSC_SUPERJ6;a.cutoff_hz=20000.f;a.resonance=0.f;
    a.amp.attack_s=.002f;a.amp.decay_s=.002f;a.amp.sustain=1.f;a.amp.release_s=.002f;
    a.filter_env_hz=0.f;a.sub_gain=0.f;a.gain=1.f;a.pan=0.f;a.pulse_width=width;
    assert(!dnbx_set_layer(s,l,&a));
}
static void init(dnbx_part *s,unsigned a,unsigned b)
{
    assert(dnbx_init_superj6(s,44100)==DNBX_OK);
    controls(s,0,a,.5f);controls(s,1,b,.5f);
    const uint8_t no[2]={0};dnbx_set_sj6_sources(s,0,no);
}
static void tick(dnbx_part *s,unsigned n,float bend,float gain1,float gain2)
{
    memset(carrier_tap,0,sizeof carrier_tap);memset(raw_tap,0,sizeof raw_tap);
    assert(dnbx_render_pitch_osc_levels_velocity(s,o[0],o[1],n,bend,gain1,gain2,0.f)==DNBX_OK);
    for(unsigned l=0;l<2;++l)for(unsigned i=0;i<n;++i)assert(isfinite(raw_tap[l][i])&&isfinite(o[l][i]));
}
static uint32_t h(uint32_t v,const float *x)
{
    for(unsigned k=0;k<128;k++){union{float f;uint32_t u;}u={x[k]};v=(v^u.u)*16777619u;}
    return v;
}
static void mute(void)
{
    init(&p[0],0,1);init(&p[1],0,1);
    int i=dnbx_note_on(&p[0],65,87);assert(i==dnbx_note_on(&p[1],65,87)&&i>=0);
    float arm[2][128];
    unsigned a=0,b=0;
    for(unsigned k=0;k<41;++k){
        float ratio=(k==17||k==18)?.891f:k>=19&&k<=21?1.059f:1.f;
        tick(&p[0],128,ratio,.808f,.808f);memcpy(arm,raw_tap,sizeof arm);
        tick(&p[1],128,ratio, k>=12&&k<=20?0.f:.808f,k>=17&&k<=28?0.f:.808f);
        for(unsigned l=0;l<2;l++){
            int m=l?k>=17&&k<=28:k>=12&&k<=20;
            if(m){for(unsigned j=0;j<128;++j)assert(raw_tap[l][j]==0);a++;}
            else {assert(!memcmp(arm[l],raw_tap[l],sizeof arm[l]));b++;}
        }
        assert(!memcmp(&p[0].voice[i].layer[0].phase,&p[1].voice[i].layer[0].phase,sizeof(float)));
        assert(!memcmp(&p[0].voice[i].layer[1].phase,&p[1].voice[i].layer[1].phase,sizeof(float)));
    }
    printf("mute guards %u zero blocks %u exact audible blocks\n",a,b);
}
static void switch_modes(void)
{
    init(&p[0],0,1);int i=dnbx_note_on(&p[0],65,87);assert(i>=0);
    for(unsigned k=0;k<64;k++)tick(&p[0],128,1.f,.808f,.808f);
    unsigned seq[]={1,3,2,0};
    for(unsigned k=0;k<4;k++){
        float prev=p[0].voice[i].layer[0].phase;
        controls(&p[0],0,seq[k],.5f);
        assert(prev==p[0].voice[i].layer[0].phase);
        uint32_t m=2166136261u;
        for(unsigned j=0;j<4;j++){tick(&p[0],128,1.f,.808f,.808f);m=h(m,raw_tap[0]);}
        printf("switch %u %u %a %a %08x\n",k,seq[k],prev,p[0].voice[i].layer[0].phase,m);
    }
}
static void steal(void)
{
    init(&p[0],0,1);init(&p[1],0,1);
    for(unsigned k=0;k<6;k++){
        assert(dnbx_note_on(&p[0],48+k,87)>=0);
        assert(dnbx_note_on(&p[1],48+k,87)>=0);
    }
    for(unsigned k=0;k<41;k++){tick(&p[0],128,1.f,.808f,.808f);tick(&p[1],128,1.f,.808f,.808f);}
    assert(dnbx_active_voices(&p[0])==6);
    dnbx_voice old[6];memcpy(old,p[0].voice,sizeof old);
    int i=dnbx_note_on(&p[0],68,87);assert(i>=0&&old[i].note==48);
    assert(p[0].voice[i].layer[0].phase==0.f&&p[0].voice[i].layer[1].phase==0.f);
    const float z[4]={0};
    for(unsigned l=0;l<2;l++)assert(!memcmp(p[0].voice[i].layer[l].oscillator_history,z,sizeof z));
    for(int j=0;j<6;j++)if(i!=j)for(unsigned l=0;l<2;l++){
        assert(old[j].layer[l].phase==p[0].voice[j].layer[l].phase);
        assert(!memcmp(old[j].layer[l].oscillator_history,p[0].voice[j].layer[l].oscillator_history,sizeof z));
    }
    tick(&p[0],128,1.f,.808f,.808f);tick(&p[1],128,1.f,.808f,.808f);
    for(int j=0;j<6;j++)if(i!=j)for(unsigned l=0;l<2;l++){
        assert(p[1].voice[j].layer[l].phase==p[0].voice[j].layer[l].phase);
        assert(!memcmp(p[1].voice[j].layer[l].oscillator_history,p[0].voice[j].layer[l].oscillator_history,sizeof z));
    }
    printf("steal slot %d old %u new %u cold phase exact, ten survivor lanes exact\n",i,old[i].note,p[0].voice[i].note);
}
int main(void){switch_modes();}
