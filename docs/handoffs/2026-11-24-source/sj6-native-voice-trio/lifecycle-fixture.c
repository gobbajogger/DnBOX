#include "gate_helpers.inc"
#include <stddef.h>
static void voice(unsigned v)
{
 ui.overlay=O_NONE;ui.part=PART_303A+v;set_view(V_PART);build_pages();
 visit("VOICE");smc_peek_cancel();smc_page_set(cur_page());
 for(unsigned i=0;i<3;i++)CHECK(pg.r[cur_page()][i].kind==R_SJ6MONO32&&pg.r[cur_page()][i].b==v&&pg.r[cur_page()][i].c==i);
 for(unsigned i=3;i<4;i++)CHECK(pg.r[cur_page()][i].kind==R_NONE);
}
static dnbx_voice *voice0(unsigned v,unsigned n,unsigned vel)
{
 dnbx_voice *a=&parts_juno[v]->core.voice[0];
 CHECK(all_gates(parts_juno[v])==1&&a->gate&&a->note==n&&a->velocity==vel/127.f&&!all_gates(parts_juno[1-v]));
 return a;
}
static void edge(unsigned v,unsigned note,unsigned vel){key_midi(v,note,vel);sample(1);}
static void vals(unsigned owner,unsigned flags,unsigned glide)
{
 const uint8_t *b=sj6_voice_read(&proj.sound,owner);
 const juno6_t *p=parts_juno[owner];
 CHECK(b[0]==sj6_voice_tag(owner,flags,glide)&&b[1]==glide);
 CHECK(p->mono32_on==(flags&1u)&&p->legato33_on==((flags>>1)&1u)&&p->glide35_detent==glide);
}
int main(void)
{
 setup(V_PART,PART_303A);proj.set.bt|=PROJ_BT_CLK_IN;stage_juno(1);
 unsigned writes=store_writes;undo_start();
 for(unsigned v=0;v<NBASS;v++){
  voice(v);vals(v,0,0);
  detent(0,1);detent(1,1);
  for(unsigned n=0;n<17u+v;n++)detent(2,1);
  vals(v,3,17+v);
 }
 CHECK(store_writes==writes&&undo_commit_stable());
 for(unsigned v=0;v<2;v++){
  edge(v,60,99);edge(v,60,107);voice0(v,60,99);
  edge(v,60,0);voice0(v,60,99);
  edge(v,60,0);CHECK(!parts_juno[v]->core.voice[0].gate);
  original_runtime_part *runtime=(original_runtime_part *)((char *)parts_juno[v]-offsetof(original_runtime_part,dsp.juno.synth));
  CHECK(original_runtime_key(runtime,ORIGINAL_INPUT_USB,61,105)==ORIGINAL_RUNTIME_OK);
  CHECK(original_runtime_key(runtime,ORIGINAL_INPUT_TRS,61,89)==ORIGINAL_RUNTIME_OK);
  voice0(v,61,105);
  CHECK(original_runtime_key(runtime,ORIGINAL_INPUT_TRS,61,0)==ORIGINAL_RUNTIME_OK);
  voice0(v,61,105);
  CHECK(original_runtime_release_source(runtime,ORIGINAL_INPUT_USB)==ORIGINAL_RUNTIME_OK);
  CHECK(!parts_juno[v]->core.voice[0].gate);
#if X0X_ARP_ENGINE
  engine_arp_cfg prev=runtime->arp.cfg, cfg=prev;
  CHECK(!prev.on);
  edge(v,62,98);edge(v,62,116);voice0(v,62,98);
  CHECK(runtime->keys[ORIGINAL_INPUT_USB][62]==2u);
  cfg.on=1;CHECK(original_runtime_arp_config(runtime,&cfg));
  CHECK(!parts_juno[v]->core.voice[0].gate&&runtime->arp.nheld==1u);
  CHECK(original_runtime_arp_config(runtime,&prev));
  CHECK(parts_juno[v]->core.voice[0].gate&&parts_juno[v]->core.voice[0].note==62);
  edge(v,62,0);CHECK(parts_juno[v]->core.voice[0].gate);
  edge(v,62,0);CHECK(!parts_juno[v]->core.voice[0].gate);
#endif
  edge(v,127,103);edge(v,0,100);voice0(v,0,103);
  edge(v,0,0);voice0(v,127,103);
  edge(v,127,0);CHECK(!parts_juno[v]->core.voice[0].gate);
  juno6_all_off(parts_juno[v]);
 }
 CHECK(store_writes==writes);
 printf("PASS physical both-owner VOICE; USB duplicate, cross-source final release, arp handoff, MIDI boundaries; %u (%u flash writes)\n",checks,store_writes-writes);
}
