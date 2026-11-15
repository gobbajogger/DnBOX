/* SPDX-License-Identifier: GPL-3.0-only */
#include "sj6x.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
static unsigned checks,failures;
#define C(x) do{checks++;if(!(x)){failures++;fprintf(stderr,"FAIL line%d: %s\n",__LINE__,#x);}}while(0)
static int different(const float*a,const float*b,unsigned n){for(unsigned i=0;i<n;i++)if(a[i]!=b[i])return 1;return 0;}
int main(void){
 sj6x_engine a,b,save;sj6x_patch p,bad;float x[1024],y[1024],z[1024];
 C(sj6x_init(&a,48000.f));sj6x_default_patch(&p);C(sj6x_apply(&a,&p));
 C(sj6x_note_on(&a,60,100));C(sj6x_render(&a,x,1024));float energy=0;for(unsigned i=0;i<1024;i++){C(isfinite(x[i]));energy+=x[i]*x[i];}C(energy>0.0001f);
 /* Block partition may not affect sample order, envelopes, PRNG or LFO. */
 C(sj6x_init(&a,48000));C(sj6x_apply(&a,&p));b=a;C(sj6x_note_on(&a,61,119));C(sj6x_note_on(&b,61,119));
 C(sj6x_render(&a,x,1024));for(unsigned off=0;off<1024;off+=64)C(sj6x_render(&b,y+off,64));C(!memcmp(x,y,sizeof x));C(!memcmp(&a,&b,sizeof a));
 /* Amp2 is not replaced by amp1/link flag and filter contour is independent. */
 p.envelope[0].attack=.002f;p.envelope[1].attack=.5f;p.envelope[2].attack=.2f;
 C(sj6x_init(&a,48000));C(sj6x_apply(&a,&p));C(sj6x_note_on(&a,60,127));C(sj6x_render(&a,x,1024));
 C(a.voice[0].envelope[0].value>a.voice[0].envelope[1].value);
 C(a.voice[0].envelope[2].value>a.voice[0].envelope[1].value);
 /* Every manually named target modulation route has an actual effect. */
 for(unsigned route=1;route<=8;route*=2){
  sj6x_default_patch(&p);C(sj6x_init(&a,48000));C(sj6x_apply(&a,&p));C(sj6x_note_on(&a,69,100));b=a;
  p.lfo[0].hz=9;p.lfo[0].depth=.7f;p.lfo[0].routes=route;C(sj6x_apply(&b,&p));C(sj6x_render(&a,x,1024));C(sj6x_render(&b,y,1024));C(different(x,y,1024));
 }
 /* Six active voices, stealing, velocity-zero note-off and bounded release. */
 sj6x_default_patch(&p);for(unsigned i=0;i<3;i++)p.envelope[i].release=.005f;
 C(sj6x_init(&a,48000));C(sj6x_apply(&a,&p));for(unsigned n=60;n<67;n++)C(sj6x_note_on(&a,n,127));
 unsigned active=0;for(unsigned i=0;i<6;i++)active+=a.voice[i].gate;C(active==6);
 for(unsigned n=60;n<67;n++)C(sj6x_note_on(&a,n,0));C(sj6x_render(&a,x,1024));
 for(unsigned i=0;i<6;i++)C(!a.voice[i].envelope[0].stage&&!a.voice[i].envelope[1].stage);
 /* Cold apply and invalid events/output preserve state and sentinel. */
 save=a;bad=p;bad.saw=NAN;C(!sj6x_apply(&a,&bad));C(!memcmp(&a,&save,sizeof a));
 C(!sj6x_note_on(&a,128,100));C(!sj6x_note_on(&a,60,128));C(!sj6x_note_off(&a,128));
 C(!sj6x_render(&a,(float*)&a,1));C(!sj6x_render(&a,x,4097));C(!sj6x_render(&a,(float*)((unsigned char*)x+1),1));
 C(!memcmp(&a,&save,sizeof a));C(sj6x_render(&a,0,0));C(!sj6x_init(&b,NAN));C(!sj6x_init(&b,7999));
 /* No hidden clamp: valid gain1.001/level1.32 survives patch admission. */
 p.saw=p.pulse=1.001f;p.level=1.32f;C(sj6x_apply(&a,&p));C(a.patch.saw==1.001f&&a.patch.level==1.32f);
 C(sj6x_all_off(&a));C(sj6x_render(&a,z,1024));for(unsigned i=0;i<1024;i++)C(z[i]==0);
 printf("{\"checks\":%u,\"failures\":%u,\"engine_bytes_host\":%zu,\"patch_bytes_host\":%zu}\n",checks,failures,sizeof a,sizeof p);
 return failures?1:0;
}
