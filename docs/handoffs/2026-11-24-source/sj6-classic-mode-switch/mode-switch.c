#include "classic-smk.c"
static void switch_knob(unsigned v,unsigned knob,int on)
{
 owner(v);visit("CHORUS");smc_peek_cancel();smc_page_set(cur_page());
 CHECK(knob<2);
 CHECK(pg.r[cur_page()][knob].c==(knob ? SJ6_CHORUS_II : SJ6_CHORUS_I));
 CHECK(!!pref_get(pg.r[cur_page()][knob])!=!!on);
 detent(knob,on?1:-1);
 render(1);
 CHECK(!!pref_get(pg.r[cur_page()][knob])==!!on);
}
static void targets(unsigned v,unsigned mode)
{
 const classic_mode_i *p=sj6_classic_i_probe(v);
 static const float rates[]={0.f,0x1.06a7eep-1f,0x1.b9db2p-1f,0x1.b9db1ep+0f};
 CHECK(p && p->started==!!mode);
 if(mode) {
  CHECK(p->mode_ii==mode);
  CHECK(p->p[0].pending==rates[mode]);
  CHECK(p->p[1].pending==0x1.853f2p-3f);
  CHECK(p->p[3].pending==(mode==3?0x1.1c28f6p-1f:0x1.028f5cp-1f));
 }
}
int main(void)
{
 setup(V_PART,PART_303A);prepare();
 for(unsigned v=0;v<2;v++)tune(v);
 const unsigned writes=store_writes;
 for(unsigned v=0;v<2;v++) {
  tune(v);detent(0,3);detent(1,4);detent(2,5);
  uint8_t mine[3],peer[32];
  memcpy(mine,ci(v),sizeof mine);memcpy(peer,read137(&proj.sound,1-v),sizeof peer);
  const classic_mode_i *p=sj6_classic_i_probe(v);
  classic_play(v);
  unsigned cursor=p->core.write[0];
  float phase=p->core.phase;
  switch_knob(v,1,1);targets(v,3);
  CHECK(!memcmp(mine,ci(v),3)&&!memcmp(peer,read137(&proj.sound,1-v),sizeof peer));
  fprintf(stderr,"transition=%u old=%u new=%u phase=%g>%g\n",p->mode_ii,cursor,p->core.write[0],(double)phase,(double)p->core.phase);
  CHECK((p->core.write[0]+CLASSIC_I_DELAY_FRAMES-cursor)%CLASSIC_I_DELAY_FRAMES==222u&&p->core.phase!=phase);
  classic_play(v);
  CHECK(p->core.write[0]!=cursor&&p->core.phase!=phase);
  cursor=p->core.write[0];phase=p->core.phase;
  switch_knob(v,0,0);targets(v,2);
  fprintf(stderr,"transition=%u old=%u new=%u phase=%g>%g\n",p->mode_ii,cursor,p->core.write[0],(double)phase,(double)p->core.phase);
  CHECK((p->core.write[0]+CLASSIC_I_DELAY_FRAMES-cursor)%CLASSIC_I_DELAY_FRAMES==222u&&p->core.phase!=phase);
  CHECK(!memcmp(mine,ci(v),3)&&!memcmp(peer,read137(&proj.sound,1-v),sizeof peer));
  classic_play(v);
  CHECK(p->core.write[0]!=cursor&&p->core.phase!=phase);
  cursor=p->core.write[0];phase=p->core.phase;
  switch_knob(v,1,0);targets(v,0);
  classic_play(v);
  CHECK(p->core.write[0]==cursor&&p->core.phase==phase);
  CHECK(!memcmp(mine,ci(v),3)&&!memcmp(peer,read137(&proj.sound,1-v),sizeof peer));
  switch_knob(v,0,1);targets(v,1);
  fprintf(stderr,"transition=%u old=%u new=%u phase=%g>%g\n",p->mode_ii,cursor,p->core.write[0],(double)phase,(double)p->core.phase);
  CHECK((p->core.write[0]+CLASSIC_I_DELAY_FRAMES-cursor)%CLASSIC_I_DELAY_FRAMES==222u&&p->core.phase!=phase);
  CHECK(ci(v)[0]==129&&ci(v)[1]==129&&ci(v)[2]==129);
  CHECK(!memcmp(peer,read137(&proj.sound,1-v),sizeof peer));
 }
 CHECK(store_writes==writes);
 printf("PASS physical I/Both/II/OFF/I actual corrected receiver owner lifecycle: %u checks\n",checks);
}
