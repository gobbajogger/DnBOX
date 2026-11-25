#include "juno6.h"
#include <stdio.h>
#include <string.h>
extern int juno6_set_mod_destination(juno6_t*,unsigned) __attribute__((weak));
#define C(e) do{if(!(e))return fprintf(stderr,"FAIL %d: %s\n",__LINE__,#e),1;}while(0)
int main(int argc,char **argv){
 static juno6_t a[2],ref,control,prior; static float x[2][256],y[2][256],z[2][256];C(argc==3);
 juno6_init(a,44100);C(a[0].typed&&juno6_note_on(a,60,100));
 FILE*f=fopen(argv[1],"wb");C(f);for(unsigned i=0;i<32;i++){C(juno6_render_stereo(a,x[0],x[1],256));C(fwrite(x,sizeof x,1,f)==1);}C(!fclose(f));if(argv[2][0]=='0')return 0;
 C(juno6_set_mod_destination);
 for(unsigned o=0;o<2;o++){
  juno6_t*s=a+o,*peer=a+(o^1u); juno6_init(s,44100);juno6_init(peer,44100);
  float words[2]={0.f,.875f};juno6_set_vibrato_words(s,words,3,0);
  C(juno6_note_on(s,52+o,103));
  C(juno6_render_stereo(s,x[0],x[1],125));
  ref=control=*s;prior=*peer;
  C(juno6_set_mod_destination(s,1));
  unsigned changed=0;
  for(unsigned v=1;v<128;v++){
   juno6_t before=*s;
   C(juno6_set_mod_wheel(s,v));
   C(!memcmp(s->core.raw,before.core.raw,sizeof s->core.raw)&&!memcmp(s->core.raw_present,before.core.raw_present,sizeof s->core.raw_present));
   C(!memcmp(s->core.voice,before.core.voice,sizeof s->core.voice)&&!memcmp(peer,&prior,sizeof prior));
   C(s->vibrato_phase==before.vibrato_phase&&s->vibrato_step==before.vibrato_step&&s->vibrato_left==before.vibrato_left);
   juno6_set_vibrato(&ref,(float)v/127.f,words[1],0);
   C(juno6_render_stereo(s,x[0],x[1],256)&&juno6_render_stereo(&ref,y[0],y[1],256)&&juno6_render_stereo(&control,z[0],z[1],256));
   C(!memcmp(x,y,sizeof x));changed+=memcmp(x,z,sizeof x)!=0;
  }
  C(changed>95);
  float prior_vib=s->vibrato_depth;ref=*s;
  C(!juno6_set_mod_destination(s,2)&&!juno6_set_mod_destination(s,5)&&!memcmp(s,&ref,sizeof ref));
  C(juno6_set_mod_destination(s,6));C(juno6_set_mod_wheel(s,101)&&s->vibrato_depth==prior_vib&&s->lfo_step_q40[0]);
  C(juno6_set_mod_destination(s,1)&&s->vibrato_depth==prior_vib);
  C(juno6_set_mod_wheel(s,0)&&s->vibrato_depth==0.f);
  printf("owner=%u: %u/127 stereo blocks changed with audible vibrato; matched reference transfer exact\n",o,changed);
 }
 printf("PASS DSP %zu B\n",sizeof a[0]);return 0;
}
