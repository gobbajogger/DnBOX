#include "juno6.h"
#include <stdio.h>
#include <string.h>
#define C(e) do{if(!(e))return fprintf(stderr,"FAIL %d: %s\n",__LINE__,#e),1;}while(0)
static juno6_t a[2],ref,control,prior,test;
static float x[2][256],y[2][256],z[2][256];
int main(int argc,char **argv){
 C(argc==3);juno6_init(a,44100);C(a[0].typed&&juno6_note_on(a,60,100));
 FILE*f=fopen(argv[1],"wb");C(f);for(unsigned i=0;i<32;i++){C(juno6_render_stereo(a,x[0],x[1],256));C(fwrite(x,sizeof x,1,f)==1);}C(!fclose(f));if(argv[2][0]=='0')return 0;
 for(unsigned o=0;o<2;o++)for(unsigned mode=0;mode<2;mode++){
  juno6_t*s=a+o,*peer=a+(o^1u); juno6_init(s,44100);juno6_init(peer,44100);
  juno6_oscillator_edit square={1.f,.43f,1.f};
  for(unsigned l=0;l<2;l++){juno6_apply_pre_note_wave(s,l,&square);dnbx_layer_params p=s->core.layer[l].params;p.pan=l?1.f:-1.f;C(dnbx_set_layer(&s->core,l,&p)==DNBX_OK);}
  juno6_set_lfo_tempo(s,132.f);juno6_set_panel_lfo(s,1,38,mode);
  juno6_set_lfo2_pwm(s,((o+1u)<<SJ6_LFO2_PWM_ROUTE_SHIFT)|110u);
  C(juno6_note_on(s,52+o,103));C(juno6_render_stereo(s,x[0],x[1],125));
  ref=control=*s;prior=*peer;
  C(juno6_set_mod_destination(s,4));
  for(unsigned v=0;v<128;v++){test=*s;C(juno6_set_mod_wheel(s,v));
   C(!memcmp(s->core.raw,test.core.raw,sizeof s->core.raw)&&!memcmp(s->core.raw_present,test.core.raw_present,sizeof s->core.raw_present));
   C(!memcmp(s->core.voice,test.core.voice,sizeof s->core.voice)&&!memcmp(peer,&prior,sizeof prior));
   C(!memcmp(s->lfo_phase_q40,test.lfo_phase_q40,sizeof s->lfo_phase_q40)&&s->lfo_step_q40[0]==test.lfo_step_q40[0]&&s->lfo2_pwm_until_tick==test.lfo2_pwm_until_tick);
   juno6_lfo_words w={.word={0,(float)v/127.f,0,0,(float)mode},.present=18};
   juno6_set_lfo_words(&ref,&w,0);C(s->lfo_step_q40[1]==ref.lfo_step_q40[1]&&s->lfo2_hold_period==ref.lfo2_hold_period);
  }
  unsigned changed=0;
  for(unsigned i=0;i<80;i++){C(juno6_render_stereo(s,x[0],x[1],256)&&juno6_render_stereo(&ref,y[0],y[1],256)&&juno6_render_stereo(&control,z[0],z[1],256));
   C(!memcmp(x,y,sizeof x));C(!memcmp(x[o^1],z[o^1],sizeof x[0]));changed+=memcmp(x[o],z[o],sizeof x[0])!=0;}
  C(changed>50);
  juno6_set_lfo_tempo(s,73.f);juno6_set_lfo_tempo(&ref,73.f);C(s->lfo_step_q40[1]==ref.lfo_step_q40[1]);
  C(juno6_set_mod_destination(s,1));uint64_t pwm_phase=s->lfo_phase_q40[1];
  C(juno6_set_mod_wheel(s,101)&&s->lfo_phase_q40[1]==pwm_phase&&s->lfo_step_q40[1]==ref.lfo_step_q40[1]);
  test=*s;C(!juno6_set_mod_destination(s,2)&&!juno6_set_mod_destination(s,3)&&!juno6_set_mod_destination(s,5)&&!memcmp(s,&test,sizeof test));
  printf("owner=%u mode=%u changed=%u/80 blocks; opposite oscillator and direct route exact\n",o,mode,changed);
 }
 printf("PASS DSP size %zu\n",sizeof a[0]);return 0;
}
