#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "fixed_candidate.h"
static uint32_t rng = UINT32_C(0x7b9d21f3);
static uint32_t next(void){rng^=rng<<13;rng^=rng>>17;rng^=rng<<5;return rng;}
static uint32_t word(float x){uint32_t b;memcpy(&b,&x,4);return b;}
static float fromword(uint32_t b){float x;memcpy(&x,&b,4);return x;}
static float oracle(float t,float r,int a){return t?(float)exp(-1./(((double)r*t)*(a?.6:1.))):0.f;}
typedef struct {unsigned n,diff,maxulp;float t,r;int a;uint32_t o,c;} stats;
static void test(stats *s,float t,float r,int a){
 float o=oracle(t,r,a),c=s1_retention_fixed(t,r,a);uint32_t u=word(o),v=word(c),d=u>v?u-v:v-u;
 s->n++;if(d){s->diff++;if(d>s->maxulp){s->maxulp=d;s->t=t;s->r=r;s->a=a;s->o=u;s->c=v;}}
}
static void emit(const char *name,stats s){printf("%s,%u,%u,%u,%.10g,%.10g,%d,%08x,%08x\n",name,s.n,s.diff,s.maxulp,s.t,s.r,s.a,s.o,s.c);}
int main(void){
 stats random={0},boundary={0},subnormal={0};
 puts("suite,points,differing_coefficients,max_ulp,worst_seconds,worst_rate,worst_attack,worst_oracle,worst_candidate");
 /* Stratified binary32 words across positive filter release 0..2 and random
  * binary32 candidate rates in 8000..192000. Attack/decay domain tested too. */
 for(unsigned i=0;i<300000;i++){
  float r=8000.f+184000.f*((next()>>8)*(1.f/16777216.f));
  uint32_t b=next()%UINT32_C(0x40000001);
  test(&random,fromword(b),r,0);
  float t=.0005f+(2.f-.0005f)*((next()>>8)*(1.f/16777216.f));
  test(&random,t,r,1);
 }
 /* Invert binary32 output midpoints to seconds, test that candidate and
  * both immediate duration neighbors. Harness log/exp are host-only. */
 float rates[]={8000.f,44100.f,48000.f,192000.f};
 float minima[]={.0005f,.005f,.004f,0.f};
 for(unsigned rr=0;rr<4;rr++)for(int role=0;role<4;role++)for(unsigned i=0;i<25000;i++){
  int a=role==0;
  uint32_t low=word(oracle(minima[role],rates[rr],a));
  uint32_t high=word(oracle(2.f,rates[rr],a));
  uint32_t b=low+(next()%(high-low));
  if(!b)b=1;
  double midpoint=((double)fromword(b)+(double)fromword(b+1))*.5;
  float t=(float)(-1./((double)rates[rr]*(a?.6:1.)*log(midpoint)));
  uint32_t u=word(t);
  if(t>0.f && t<=2.f && t>=minima[role]){
   test(&boundary,fromword(u-1),rates[rr],a);
   test(&boundary,t,rates[rr],a);
   test(&boundary,fromword(u+1),rates[rr],a);
  }
 }
 /* Dense exponent/subnormal-output transition durations, with neighbors. */
 for(unsigned rr=0;rr<4;rr++)for(unsigned i=0;i<30000;i++){
  double x=85.+21.*((double)i/29999.);
  float t=(float)(1./((double)rates[rr]*x));uint32_t u=word(t);
  for(int k=-1;k<=1;k++)test(&subnormal,fromword(u+k),rates[rr],0);
 }
 emit("random_binary32_inputs",random);emit("output_midpoint_duration_neighbors",boundary);emit("subnormal_transition_neighbors",subnormal);
 return random.diff||boundary.diff||subnormal.diff;
}
