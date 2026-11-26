#include "dnbx_core.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
static dnbx_part p;
int main(int argc,char **argv){
 assert(argc==4);
 unsigned shape=atoi(argv[1]), voice=atoi(argv[2]), range=atoi(argv[3]);
 static float l[128],r[128]; 
 assert(dnbx_init_superj6(&p,44100)==DNBX_OK);
 for(unsigned i=0;i<2;i++){
  dnbx_layer_params a;
  assert(!dnbx_get_layer(&p,i,&a));
  a.waveform=shape/3.f;a.oscillator_mode=DNBX_OSC_SUPERJ6;
  a.cutoff_hz=20000.f;a.resonance=0.f;a.amp.attack_s=.002f;
  a.amp.decay_s=.002f;a.amp.sustain=1.f;a.amp.release_s=.02f;
  a.filter_env_hz=0.f;a.sub_gain=0.f;a.gain=1.f;a.pan=0.f;a.pulse_width=.28f;
  a.lfo_pwm_depth=.1f; a.lfo_hz=3.f;
  assert(!dnbx_set_layer(&p,i,&a));
 }
 assert(dnbx_store_raw(&p,86u,range*.5f)==DNBX_RAW_STORED_ONLY);
 const uint8_t src[2]={0,voice ? 80 : 0};dnbx_set_sj6_sources(&p,0,src);
 for(unsigned k=0;k<160;k++){
  if(k==5)assert(dnbx_note_on(&p,48,87)>=0);
  if(k==20)assert(dnbx_note_on(&p,65,79)>=0);
  if(k==120){assert(dnbx_note_off(&p,48)>=0);assert(dnbx_note_off(&p,65)>=0);}
  assert(dnbx_render_pitch_osc_levels_velocity(&p,l,r,128,k>=32&&k<78?.917f:1.f,voice==2?.808f:voice?0.f:.808f,0.,0.f)==DNBX_OK);
  for(unsigned j=0;j<128;j++)assert(isfinite(l[j])&&isfinite(r[j]));
  assert(fwrite(l,sizeof *l,128,stdout)==128&&fwrite(r,sizeof *r,128,stdout)==128);
 }
}
