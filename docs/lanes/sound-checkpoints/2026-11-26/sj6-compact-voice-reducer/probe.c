#include "dnbx_core.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

float reducer_tap[2][DNBX_MAX_FRAMES];

static void probe(unsigned leg,unsigned notes,float pan)
{
    dnbx_part s;
    assert(dnbx_init_superj6(&s,44100)==DNBX_OK);
    assert(dnbx_set_velocity_mode(&s,DNBX_VELOCITY_LINEAR)==DNBX_OK);
    for(unsigned j=0;j<2;++j) {
        dnbx_layer_params p;
        dnbx_default_layer_params(&p);
        memset(&p.amp,0,sizeof(p.amp));
        memset(&p.filter,0,sizeof(p.filter));
        p.amp.attack_s=.002f; p.amp.decay_s=.002f; p.amp.sustain=1.f; p.amp.release_s=.1f;
        p.filter.sustain=1.f; p.filter.release_s=.1f;
        p.oscillator_mode=DNBX_OSC_SUPERJ6;
        p.waveform=0.f; p.gain=1.f; p.sub_gain=0.f; p.cutoff_hz=20000.f; p.resonance=0.f;
        p.pan=j==leg?pan:0.f;
        assert(dnbx_set_layer(&s,j,&p)==DNBX_OK);
    }
    for(unsigned v=0;v<notes;++v)assert(dnbx_note_on(&s,69+v,87)==(int)v);
    assert(dnbx_active_voices(&s)==(int)notes);
    /* In the six-note case force only the oscillator step to be correlated
       for a worst-case headroom witness; the allocator still admitted six
       genuinely distinct MIDI notes through the unmodified public ABI. */
    for(unsigned v=1;v<notes;++v)
      for(unsigned j=0;j<2;++j)s.voice[v].layer[j].increment=s.voice[0].layer[j].increment;
    const float gains[2]={leg==0?.808f:0.f,leg==1?.808f:0.f};
    float l[128],r[128];
    double pre=0,left=0,right=0,peak=0;
    unsigned count=0;
    for(unsigned k=0;k<172;++k) {
        assert(dnbx_render_pitch_osc_levels(&s,l,r,128,1.f,gains[0],gains[1])==DNBX_OK);
        for(unsigned n=0;n<128;++n) {
           const float x=reducer_tap[leg][n];
           assert(reducer_tap[!leg][n]==0.f);
           /* Inspect exactly the real compact arithmetic; no output fitting. */
           assert(l[n]==(x*(1.f/6.f))*(.5f-.5f*pan));
           assert(r[n]==(x*(1.f/6.f))*(.5f+.5f*pan));
           if(k>=50) {
               pre+=(double)x*x;left+=(double)l[n]*l[n];right+=(double)r[n]*r[n];++count;
               if(fabs(l[n])>peak)peak=fabs(l[n]);
               if(fabs(r[n])>peak)peak=fabs(r[n]);
           }
        }
    }
    printf("layer=%u voices=%u pan=%+.1f pre_rms=%.9f left_rms=%.9f right_rms=%.9f level_delta_L=%.7f level_delta_R=%.7f actual_out_peak=%.9f\n",
      leg+1,notes,(double)pan,sqrt(pre/count),sqrt(left/count),sqrt(right/count),10*log10(left/pre),10*log10(right/pre),peak);
}
int main(void)
{
    probe(0,1,0);probe(1,1,0);probe(0,6,0);probe(1,6,0);probe(1,1,.5f);
    return 0;
}
