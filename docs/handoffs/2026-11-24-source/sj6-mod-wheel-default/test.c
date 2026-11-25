#include "juno6.h"
#include <stdio.h>
#include <string.h>
extern int juno6_set_mod_wheel(juno6_t*,unsigned) __attribute__((weak));
#define C(e) do{if(!(e))return fprintf(stderr,"FAIL %d: %s\n",__LINE__,#e),1;}while(0)
int main(int argc,char **argv) {
 static juno6_t a,b,prior;float x[2][256],y[2][256];C(argc==3);
 juno6_init(&a,44100); C(a.typed); C(juno6_note_on(&a,60,117)); FILE*f=fopen(argv[1],"wb");C(f);for(unsigned i=0;i<32;i++){C(juno6_render_stereo(&a,x[0],x[1],256));C(fwrite(x,sizeof x,1,f)==1);}C(!fclose(f));if(argv[2][0]=='0')return 0;
 C(juno6_set_mod_wheel);
 for(unsigned owner=0;owner<2;owner++)for(unsigned mode=0;mode<2;mode++){
  juno6_init(&a,44100);juno6_init(&b,44100); C(a.typed&&b.typed);
  juno6_set_lfo_tempo(&a,132.f);juno6_set_lfo_tempo(&b,132.f);
  juno6_set_panel_lfo(&a,0,40,mode);juno6_set_panel_lfo(&b,0,40,mode);
  C(juno6_note_on(&a,48+owner,96)&&juno6_note_on(&b,48+owner,96));
  for(unsigned v=0;v<128;v++){
   prior=a; C(juno6_set_mod_wheel(&a,v));
   C(!memcmp(a.core.raw,prior.core.raw,sizeof a.core.raw)&&!memcmp(a.core.raw_present,prior.core.raw_present,sizeof a.core.raw_present));
   C(!memcmp(a.lfo_phase_q40,prior.lfo_phase_q40,sizeof a.lfo_phase_q40));
   C(!memcmp(a.core.voice,prior.core.voice,sizeof a.core.voice)&&a.lfo_step_q40[1]==prior.lfo_step_q40[1]);
   juno6_lfo_words words={ .word={(float)v/127.f,0,0,(float)mode}, .present=9 };
   juno6_set_lfo_words(&b,&words,0); C(a.lfo_step_q40[0]==b.lfo_step_q40[0]);
  }
  C(juno6_render_stereo(&a,x[0],x[1],256)&&juno6_render_stereo(&b,y[0],y[1],256));
  C(!memcmp(x,y,sizeof x)); /* No LFO1 audible destination on this compact Init path. */
  juno6_set_lfo_tempo(&a,73.f);juno6_set_lfo_tempo(&b,73.f); C(a.lfo_step_q40[0]==b.lfo_step_q40[0]);
  prior=a; C(!juno6_set_mod_wheel(&a,128)&&!memcmp(&a,&prior,sizeof a));
  juno6_set_panel_lfo(&a,0,40,mode);juno6_set_panel_lfo(&b,0,40,mode);
  C(a.lfo_step_q40[0]==b.lfo_step_q40[0]);
 }
 printf("PASS 512 held lane rate turns; DSP size %zu\n",sizeof a);return 0;
}
