#include "juno6.h"
#include <stdio.h>
#include <string.h>
#define C(e) do{if(!(e))return fprintf(stderr,"FAIL %d: %s\n",__LINE__,#e),1;}while(0)
extern float juno6_mod_cutoff_hz(unsigned) __attribute__((weak));
static float exact(unsigned u){float x=(float)u/127.0f,t; double b,k;
 if(x<=.1f){t=x/.1f;b=20;k=10;}
 else if(x<=.3f){t=(x-.1f)/.2f;b=30;k=170;}
 else if(x<=.5f){t=(x-.3f)/.2f;b=200;k=400;}
 else if(x<=.7f){t=(x-.5f)/.2f;b=600;k=3500;}
 else if(x<=.9f){t=(x-.7f)/.2f;b=4100;k=10300;}
 else{t=(x-.9f)/.1f;b=14400;k=5600;}
 float sq=t*t;return (float)(b+(double)sq*k);
}
static juno6_t a[2],ref,control,prior,tmp;
static float x[2][256],y[2][256],z[2][256];
int main(int argc,char **argv){
 C(argc==3);juno6_init(a,44100);C(a[0].typed&&juno6_note_on(a,60,100));
 FILE*f=fopen(argv[1],"wb");C(f);for(unsigned i=0;i<32;i++){C(juno6_render_stereo(a,x[0],x[1],256));C(fwrite(x,sizeof x,1,f)==1);}C(!fclose(f));if(argv[2][0]=='0')return 0;
 for(unsigned o=0;o<2;o++){
  juno6_t*s=a+o,*peer=a+(o^1u);juno6_init(s,44100);juno6_init(peer,44100);
  C(juno6_set(s,2,.68f));C(juno6_note_on(s,62+o,103));C(juno6_render_stereo(s,x[0],x[1],125));
  ref=control=*s;prior=*peer;
  C(juno6_set_mod_destination(s,5));
  C(juno6_mod_cutoff_hz);
  dnbx_layer_params base[2]={s->core.layer[0].params,s->core.layer[1].params};
  float prev=0;unsigned audible=0;
  for(unsigned u=0;u<128;u++){
   tmp=*s;C(juno6_set_mod_wheel(s,u));
   float hz=juno6_mod_cutoff_hz(u);
   C(hz==exact(u)&&hz>=prev);prev=hz;
   C(!memcmp(s->core.raw,tmp.core.raw,sizeof s->core.raw)&&!memcmp(s->macro,tmp.macro,sizeof s->macro)&&!memcmp(peer,&prior,sizeof prior));
   C(!memcmp(s->core.voice,tmp.core.voice,sizeof s->core.voice));
   ref.core.layer[0].params.cutoff_hz=ref.core.layer[1].params.cutoff_hz=hz;
   C(juno6_render_stereo(s,x[0],x[1],256)&&juno6_render_stereo(&ref,y[0],y[1],256)&&juno6_render_stereo(&control,z[0],z[1],256));
   C(!memcmp(x,y,sizeof x));audible+=memcmp(x,z,sizeof x)!=0;
   for(unsigned l=0;l<2;l++) C(!memcmp(&base[l],&s->core.layer[l].params,sizeof base[l]));
  }
  C(audible>65&&prev>=20000.f&&prev<20000.01f&&juno6_mod_cutoff_hz(0)==20.f);
  printf("owner=%u: %u/128 blocks differed from unchanged filter; source target executor exact\n",o,audible);
 }
 printf("PASS DSP size %zu\n",sizeof a[0]);return 0;
}
