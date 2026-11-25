#include "juno6.h"
#include <stdio.h>
#include <string.h>
extern int juno6_configure_lfo1_pwm(juno6_t*,unsigned,unsigned,float,float) __attribute__((weak));
#define C(e) do{if(!(e))return fprintf(stderr,"FAIL %d: %s\n",__LINE__,#e),1;}while(0)
static juno6_t a[2], ref, baseline, peer_save;
static float x[2][256],y[2][256],z[2][256];
int main(int argc,char **argv){
 C(argc==3);juno6_init(a,44100); C(juno6_note_on(a,60,100));
 FILE*f=fopen(argv[1],"wb");C(f);for(unsigned i=0;i<32;i++){C(juno6_render_stereo(a,x[0],x[1],256));C(fwrite(x,sizeof x,1,f)==1);}C(!fclose(f));if(argv[2][0]=='0')return 0;
 C(juno6_configure_lfo1_pwm);
 for(unsigned owner=0;owner<2;owner++){
  juno6_t*s=a+owner,*peer=a+(owner^1u);juno6_init(s,44100);juno6_init(peer,44100);
  juno6_oscillator_edit square={1.f,.43f,1.f};
  for(unsigned l=0;l<2;l++){juno6_apply_pre_note_wave(s,l,&square);dnbx_layer_params p=s->core.layer[l].params;p.pan=l?1.f:-1.f;C(dnbx_set_layer(&s->core,l,&p)==DNBX_OK);}
  juno6_set_panel_lfo(s,0,72,1);
  C(juno6_note_on(s,52+owner,103));
  C(!juno6_set_mod_destination(s,2));
  ref=*s;C(!juno6_configure_lfo1_pwm(s,1,1,.43f,.2f)&&!memcmp(s,&ref,sizeof ref));
  C(juno6_configure_lfo1_pwm(s,69+owner,1,.43f,0.f));
  C(juno6_set_mod_destination(s,2));ref=baseline=*s;peer_save=*peer;
  C(juno6_set_mod_wheel(s,91));
  C(!memcmp(s->core.raw,ref.core.raw,sizeof s->core.raw)&&!memcmp(peer,&peer_save,sizeof *peer));
  C(!memcmp(s->lfo_phase_q40,ref.lfo_phase_q40,sizeof s->lfo_phase_q40)&&!memcmp(s->core.voice,ref.core.voice,sizeof s->core.voice));
  C(juno6_configure_lfo1_pwm(&ref,69+owner,1,.43f,91.f/127.f));
  unsigned changed=0;
  for(unsigned i=0;i<80;i++){C(juno6_render_stereo(s,x[0],x[1],256)&&juno6_render_stereo(&ref,y[0],y[1],256)&&juno6_render_stereo(&baseline,z[0],z[1],256));
    C(!memcmp(x,y,sizeof x));C(!memcmp(x[owner^1],z[owner^1],sizeof x[0]));changed+=memcmp(x[owner],z[owner],sizeof x[0])!=0;}
  C(changed>70);
  C(juno6_set_mod_destination(s,0));uint64_t prior=s->lfo_phase_q40[0],rate=s->lfo_step_q40[0];
  C(juno6_set_mod_wheel(s,126)&&s->lfo_phase_q40[0]==prior&&s->lfo_step_q40[0]!=rate);
  unsigned rate_audio=0;
  for(unsigned i=0;i<80;i++){C(juno6_render_stereo(s,x[0],x[1],256)&&juno6_render_stereo(&ref,y[0],y[1],256));
    C(!memcmp(x[owner^1],y[owner^1],sizeof x[0]));rate_audio+=memcmp(x[owner],y[owner],sizeof x[0])!=0;}
  C(rate_audio>70);
  C(juno6_configure_lfo1_pwm(s,69+owner,1,.29f,.314159274f));ref=*s;
  C(s->core.raw[89]==.314159274f);
  C(juno6_set_mod_destination(s,2)&&juno6_set_mod_wheel(s,0));
  C(juno6_configure_lfo1_pwm(&ref,69+owner,1,.29f,0.f));
  C(juno6_render_stereo(s,x[0],x[1],256)&&juno6_render_stereo(&ref,y[0],y[1],256));
  C(!memcmp(x,y,sizeof x)&&s->core.raw[89]==.314159274f);
  printf("owner=%u: %u/80 blocks changed (amount), %u/80 changed (rate), real selected PWM lane only\n",owner,changed,rate_audio);
 }
 printf("PASS DSP %zu\n",sizeof a[0]);return 0;
}
