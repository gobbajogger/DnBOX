#include "original_runtime.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
struct event {unsigned time,key,velocity;};
#include "case.h"
static original_runtime_part p;
static float score[3][2][35000],l[256],r[256];
static const float base=.3345831036567688f;
static void acoustic(unsigned arm,unsigned start,unsigned end) {
 juno6_t *s=&p.dsp.juno.synth;
 while(start<end){
  unsigned n=end-start;
  if(n>128)n=128;
  assert(juno6_render_stereo(s,l,r,n));
  for(unsigned j=0;j<n;++j){
   assert(isfinite(l[j])&&isfinite(r[j]));
   if(start+j>=48000 && start+j<83000){
    score[arm][0][start+j-48000]=l[j];score[arm][1][start+j-48000]=r[j];
   }
  }
  start+=n;
 }
}
static void arm(unsigned id) {
 memset(&p,0,sizeof p);
 juno6_t *s=&p.dsp.juno.synth;
 p.status=ORIGINAL_RUNTIME_OK;p.kind=ORIGINAL_SOUND_JUNO;
 juno6_init(s,44100.f);
 assert(juno6_mono32_set(s,1)&&juno6_legato33_set(s,0));
 juno6_set_aux_lfo1(s,8,43);juno6_lfo1_delay_set(s,45);
 assert(original_runtime_param(&p,2,base)==1);
 assert(original_runtime_param(&p,JUNO6_APP_PARAM_BASE+180u,base)==1);
 const unsigned t[3]={49331,49587,70000};
 const float w[3]={id==1?1.f:base,id?1.f:base,base};
 unsigned i=0,j=0,at=0;
 for(;;){
  unsigned a=i<sizeof capture_events/sizeof *capture_events?capture_events[i].time:83000;
  unsigned b=j<3?t[j]:83000;
  unsigned n=a<b?a:b;
  acoustic(id,at,n);at=n;
  if(n==83000)break;
  if(a<=b){
   const struct event e=capture_events[i++];
   assert(original_runtime_key(&p,ORIGINAL_INPUT_USB,e.key,e.velocity)==1);
   if(e.time==49459 && e.key==60){
    assert(s->note_cut_latched==(id==1));
    if(id==1)assert(s->note_cut_target==base && s->macro[2]==1.f);
   }
  }else{
   assert(original_runtime_param(&p,2,w[j])==1);++j;
   if(n==49587 && id==1)assert(s->note_cut_latched && fabsf(s->note_cut_target-base)<1e-7);
   if(n==70000 && id==1)assert(s->note_cut_latched && s->note_cut_target==0.f);
  }
 }
 assert(!p.occupied_keys);
 assert(original_runtime_all_off(&p)==1 && !p.occupied_keys);
}
int main(void){
 for(unsigned i=0;i<3;i++)arm(i);
 for(unsigned arm=1;arm<3;arm++){
  assert(memcmp(score[arm][0],score[0][0],1331*sizeof(float))==0);
  double sum=0,ref=0,e=0;
  for(unsigned st=0;st<2;st++)for(unsigned i=6000;i<21872;i++){
   double a=score[0][st][i],b=score[arm][st][i],d=b-a;
   sum+=d*d;ref+=a*a;e+=b*b;
  }
  assert(ref>0 && sum>0);
  printf("%s-chord relative level %.6f dB, signed residual %.6f dBr\n",arm==1?"before":"after",10*log10(e/ref),10*log10(sum/ref));
 }
 puts("active combined ingress: exact cue event ordering, paired cutoff cases, no orphan count after release PASS");
}
