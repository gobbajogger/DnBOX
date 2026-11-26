#include "dnbx_core.h"
#include <assert.h>
#include <float.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static dnbx_part p;
static float l[128],r[128];
float carrier_tap[2][128];
/* Fixed original-shaped cue labels. Files are our generated candidate, not
 * sample assets/ref. Original mode remains external and scores them blindly. */
static void outputs(unsigned key,unsigned osc,unsigned bend,float live_ratio)
{
    assert(dnbx_init_superj6(&p,44100)==DNBX_OK);
    for(unsigned l=0;l<2;l++){
       dnbx_layer_params a;
       assert(!dnbx_get_layer(&p,l,&a));
       a.waveform=osc/3.f;a.oscillator_mode=DNBX_OSC_SUPERJ6;
       a.cutoff_hz=20000.f;a.resonance=0.f;a.amp.attack_s=.002f;
       a.amp.decay_s=.002f;a.amp.sustain=1.f;a.amp.release_s=.002f;
       a.filter_env_hz=0.f;a.sub_gain=0.f;a.gain=1.f;a.pan=0.f;a.pulse_width=.5f;
       assert(!dnbx_set_layer(&p,l,&a));
    }
    const uint8_t no[2]={0};dnbx_set_sj6_sources(&p,0,no);
    char output[80];snprintf(output,sizeof output,"test-%u-%u-%u.pcm",key,osc,bend);
    FILE *out=fopen(output,"wb");assert(out);
    uint32_t hash=2166136261u;
    for(unsigned i=0;i<65536;i+=128){
       if(i==16384)assert(dnbx_note_on(&p,key,87)>=0);
       if(i==40960)assert(dnbx_note_off(&p,key)>=0);
       memset(carrier_tap,0,sizeof carrier_tap);
       assert(dnbx_render_pitch_osc_levels_velocity(&p,l,r,128,bend&&i>=28672&&i<36864?live_ratio:1.f,.808f,0.,0.f)==DNBX_OK);
       if(i>=16384){
          assert(fwrite(carrier_tap[0],4,128,out)==128);
          for(unsigned k=0;k<128;k++){union{float f;uint32_t u;}w={carrier_tap[0][k]};hash=(hash^w.u)*16777619u;}
       }
    }
    fclose(out);
    printf("%u %u %u %08x\n",key,osc,bend,hash);
}
int main(void){
    for(unsigned w=0;w<4;w++)
        for(unsigned k=45;k<=105;k+=12)outputs(k,w,0,1.f);
    for(unsigned w=0;w<2;w++)outputs(65,w,1,.8908987181403393f);
}
