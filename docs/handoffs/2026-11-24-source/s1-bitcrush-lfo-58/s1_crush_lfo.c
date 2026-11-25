#define MIXED_ONLY 1
#include "s1_pb_mod_ui.c"
#include <float.h>
static const s1c *watched;
static unsigned route, ticks, latches, nonkr_latches, outputs, kr_diffs, independent;
static uint32_t sequence_hash;
static s1u_crush oracle,kr;
static float out,kr_out;
static float bounded(float x,float lo,float hi){return x<lo?lo:x>hi?hi:x;}
static float inc(float base,float mod,float sr){
 const float oct=(float)log2(base)+4.f*mod;
 const float hz=bounded((float)exp2(oct),2048.f,48000.f);
 return fmaxf(sr/hz,1.f);
}
static float step(s1u_crush*c,float x,int old){
 if(c->idx<=c->n){
  c->v=x;
  if(old)c->idx+=c->inc-c->n;
  else {c->idx+=c->inc;c->idx-=c->n;}
  c->n=0;
 } else x=c->v;
 c->n+=1;return x;
}
void host_crush_tap(const s1c *e,int pre,int f,float x,float base,float a,float b)
{
 if(e!=watched)return;
 if(pre) {
  CHECK(base>=2048.f&&base<=48000.f&&a>=0.f&&a<=1.f&&b>=0.f&&b<=1.f);
  if(a!=b)independent++;
  float m=route==1?a:route==2?b:.5f*(a+b);
  oracle.inc=inc(base,m,e->sr);
  if(oracle.idx<=oracle.n){latches++;if(f%8){nonkr_latches++;}}
  out=step(&oracle,x,0);
  if(!(f%8))kr.inc=oracle.inc;
  kr_out=step(&kr,x,1);
  if(out!=kr_out)kr_diffs++;
  ticks++;
 } else {
  if(!(x==out&&oracle.idx==e->crush.idx&&oracle.n==e->crush.n&&oracle.v==e->crush.v))fprintf(stderr,"diverge f=%d route=%u target=%g x=%a/%a idx=%a/%a n=%a/%a v=%a/%a inc=%a/%a\n",f,route,e->t[58],x,out,e->crush.idx,oracle.idx,e->crush.n,oracle.n,e->crush.v,oracle.v,e->crush.inc,oracle.inc);
  CHECK(x==out&&oracle.idx==e->crush.idx&&oracle.n==e->crush.n&&oracle.v==e->crush.v);
  if(x!=0.f)outputs++;
  uint32_t b;memcpy(&b,&x,sizeof b);sequence_hash=(sequence_hash^b)*16777619u;
 }
}
static void voices(void){
 for(unsigned i=0;i<NBASS;i++){current=NULL;select_patch(i);CHECK(current);parts[i]=current;}
 stabilize();
}
int main(void)
{
 setup(V_PART,PART_303A);proj.set.bt|=PROJ_BT_CLK_IN;
 voices();
 const char *dump=getenv("S1_CRUSH_UNTOUCHED");
 if(dump){
   engine_bass_on(0,60,0,0);engine_bass_on(1,67,0,0);record(reference);
   FILE *f=fopen(dump,"wb");CHECK(f);CHECK(fwrite(reference,sizeof reference,1,f)==1&&!fclose(f));return 0;
 }
 uint32_t hashes[2][3];
 for(unsigned owner=0;owner<NBASS;owner++)for(unsigned r=1;r<=3;r++){
   stabilize();
   s1c *e=&parts[owner]->e,*peer=&parts[1-owner]->e;
   CHECK(s1c_front_set(e,58,(float)r));
   CHECK(s1c_front_set(e,34,2600.f));
   CHECK(s1c_front_set(e,139,0.f));
   CHECK(s1c_front_set(e,15,27.f)&&s1c_front_set(e,47,71.f));
   CHECK(s1c_front_set(e,13,1.f)&&s1c_front_set(e,45,2.f));
   CHECK(s1c_front_set(e,14,.96f)&&s1c_front_set(e,46,.63f));
   engine_bass_on(owner,61,0,0);engine_bass_on(1-owner,70,0,0);render(8);
   watched=e;oracle=kr=e->crush;route=r;ticks=latches=nonkr_latches=outputs=kr_diffs=independent=0;sequence_hash=2166136261u;
   record(reference);watched=NULL;
   fprintf(stderr,"route=%u owner=%u frames=%u latches=%u nonKR=%u legacyDiff=%u voiced=%u\n",r,owner,ticks,latches,nonkr_latches,kr_diffs,outputs);
   CHECK(ticks==FRAMES&&latches>200&&nonkr_latches>100&&outputs>200&&kr_diffs>100);
   CHECK(peer->t[58]==0.f && independent>4000);hashes[owner][r-1]=sequence_hash;
 }
 for(unsigned o=0;o<2;o++)CHECK(hashes[o][0]!=hashes[o][1]&&hashes[o][1]!=hashes[o][2]&&hashes[o][0]!=hashes[o][2]);
 printf("PASS actual both-owner ID58 original latch law all three routes, every sample and stale KR counterfactual: %u checks\n",checks);
}
