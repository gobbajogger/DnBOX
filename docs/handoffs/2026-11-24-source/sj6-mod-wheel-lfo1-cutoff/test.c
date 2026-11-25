#include "juno6.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
extern int juno6_configure_lfo1_cutoff(juno6_t*,float,float,float) __attribute__((weak));
#define C(e) do{if(!(e))return fprintf(stderr,"FAIL %d: %s\n",__LINE__,#e),1;}while(0)
static juno6_t a[2], prior, baseline, peer_save;
static float x[2][256], y[2][256];
int main(int argc,char**argv){
 C(argc==3);juno6_init(a,44100);C(juno6_note_on(a,60,100));FILE*f=fopen(argv[1],"wb");C(f);
 for(unsigned i=0;i<32;i++){C(juno6_render_stereo(a,x[0],x[1],256));C(fwrite(x,sizeof x,1,f)==1);}C(!fclose(f));
 if(argv[2][0]=='0')return 0;
 C(juno6_configure_lfo1_cutoff);
 for(unsigned owner=0;owner<2;owner++){
  juno6_t*s=a+owner,*peer=a+(owner^1u);juno6_init(s,44100);juno6_init(peer,44100);juno6_set_panel_lfo(s,0,55,1);
  C(juno6_note_on(s,54+owner,103));baseline=*s;peer_save=*peer;
  C(juno6_configure_lfo1_cutoff(s,.32f,.70f,0.f));
  C(juno6_set_mod_destination(s,2));
  for(unsigned i=0;i<32;i++){C(juno6_render_stereo(s,x[0],x[1],256)&&juno6_render_stereo(&baseline,y[0],y[1],256));C(!memcmp(x,y,sizeof x));}
  prior=*s; C(juno6_set_mod_wheel(s,1)); /* 1/127 < source threshold .01 */
  C(juno6_render_stereo(s,x[0],x[1],256)&&juno6_render_stereo(&baseline,y[0],y[1],256));C(!memcmp(x,y,sizeof x));C(s->core.raw[89]==0.f);
  C(juno6_set_mod_wheel(s,100));unsigned changed=0;
  for(unsigned i=0;i<80;i++){C(juno6_render_stereo(s,x[0],x[1],256)&&juno6_render_stereo(&baseline,y[0],y[1],256));changed+=memcmp(x,y,sizeof x)!=0;}
  C(changed>65&&s->core.raw[89]==0.f&&!memcmp(peer,&peer_save,sizeof *peer));
  prior=*s;C(juno6_set_mod_destination(s,0));C(juno6_set_mod_wheel(s,120));
  C(s->lfo_phase_q40[0]==prior.lfo_phase_q40[0]&&s->lfo_step_q40[0]!=prior.lfo_step_q40[0]);
  C(juno6_render_stereo(s,x[0],x[1],256));
#ifdef POST
  juno6_init(&prior,44100);C(juno6_configure_lfo1_cutoff(&prior,.3f,.7f,.8f));
  C(juno6_render_stereo(&prior,x[0],x[1],256)&&juno6_render_stereo(&prior,x[0],x[1],256)&&juno6_render_stereo(&prior,x[0],x[1],40));
  C(prior.lfo1_cut_current==.7f&&prior.mod_wheel_cutoff_hz==0.f);
  C(juno6_render_stereo(&prior,x[0],x[1],1));
  float current=.7f,target=.3f;volatile float diff=target-current;
  C(prior.lfo1_cut_target==target&&prior.lfo1_cut_current==current+diff*0x1.217c58p-1f);
  C(prior.lfo1_cut_until==43960);
#endif
  prior=*s; baseline=*s; C(juno6_set_mod_destination(&prior,5)&&juno6_set_mod_destination(&baseline,5));
  C(juno6_set_mod_wheel(&prior,120)&&juno6_set_mod_wheel(&baseline,127));
  C(prior.mod_wheel_cutoff_hz!=baseline.mod_wheel_cutoff_hz);
  C(juno6_render_stereo(&prior,x[0],x[1],256)&&juno6_render_stereo(&baseline,y[0],y[1],256));
  C(!memcmp(x,y,sizeof x)); /* inherited compact high-end recurrent-coefficient plateau */
  printf("owner=%u %u/80 audible complete stereo blocks\n",owner,changed);
 }
 printf("PASS part size %zu\n",sizeof a[0]);
 return 0;
}
