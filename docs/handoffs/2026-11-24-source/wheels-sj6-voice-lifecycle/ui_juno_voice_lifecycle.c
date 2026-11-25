#include "gate_helpers.inc"
static void voice(unsigned v)
{
 ui.overlay=O_NONE;ui.part=PART_303A+v;set_view(V_PART);build_pages();
 visit("VOICE");smc_peek_cancel();smc_page_set(cur_page());
 for(unsigned k=0;k<3;k++)CHECK(pg.r[cur_page()][k].kind==R_SJ6MONO32&&pg.r[cur_page()][k].b==v&&pg.r[cur_page()][k].c==k);
 CHECK(pg.r[cur_page()][3].kind==R_NONE);
}
static void raw_detent(unsigned k,int d){now_ms+=180;enc_acc[EN_K1+k]=d;input();}
static void edge(unsigned v,unsigned note,unsigned vel){key_midi(v,note,vel);sample(1);}
static void pedal(unsigned v,unsigned down)
{
 unsigned ch=seq.ch[TRK_BASS0+v];
 host_midi_put(11u|((0xb0u|ch)<<8)|(64u<<16)|((down?127u:0u)<<24));
 sample(1);CHECK(parts_juno[v]->core.damper==!!down);
}
static dnbx_voice *mono_note(unsigned v,unsigned n,unsigned vel)
{
 dnbx_voice *a=&parts_juno[v]->core.voice[0];
 CHECK(all_gates(parts_juno[v])==1&&a->gate&&a->note==n&&a->velocity==vel/127.f);
 return a;
}
static void phase_progress(unsigned v)
{
 int32_t lr[512];unsigned audible=0;
 for(unsigned r=0;r<24;r++){engine_render(lr,256);for(unsigned i=0;i<512;i++)audible+=lr[i]!=0;}
 CHECK(audible>256);
 CHECK(dnbx_active_voices(&parts_juno[v]->core)>0);
}
static void all_notes(void)
{
 const uint32_t fx=1u<<B_FX, seq=1u<<B_SEQ;
 keyboard_panic_reset(0);
 (void)keyboard_panic(fx,fx);
 (void)keyboard_panic(fx|seq,seq);
 CHECK(keyboard_panic(fx,seq));
 sample(1);
 (void)keyboard_panic(0,fx);
 for(unsigned o=0;o<2;o++){
  const juno6_t *j=parts_juno[o];CHECK(!all_gates(j)&&!j->core.sj6_glide_offset&&!j->core.damper);
  CHECK(!(j->mono32_keys[0]|j->mono32_keys[1]|j->mono32_keys[2]|j->mono32_keys[3]));
 }
}
int main(void)
{
 setup(V_PART,PART_303A);proj.set.bt|=PROJ_BT_CLK_IN;stage_juno(1);
 const unsigned writes=store_writes;
 for(unsigned v=0;v<2;v++){
  const unsigned peer=1-v;juno6_t *j=parts_juno[v];dnbx_part *p=&j->core;
  ui.overlay=O_NONE;ui.part=PART_303A+v;set_view(V_PART);build_pages();
  visit("AMP ADSR");smc_peek_cancel();smc_page_set(cur_page());
  CHECK(pg.r[cur_page()][3].c==SJ6_AMP_RELEASE);
  for(unsigned n=0;n<67;n++)detent(3,1);
  render(2);
  CHECK(p->layer[0].params.amp.release_s>.02f);
  voice(v);
  edge(v,43,76);edge(v,50,91);edge(peer,80,89);phase_progress(v);
  CHECK(all_gates(j)==2&&gates(parts_juno[peer],80)==1);
  const float before=p->voice[0].layer[0].amp.level;CHECK(before>0.f);
  detent(0,1);CHECK(j->mono32_on&&!all_gates(j)&&gates(parts_juno[peer],80)==1);
  CHECK(dnbx_active_voices(p)>0&&p->voice[0].layer[0].amp.level>before*.90f); /* soft releases, not hard zero */
  edge(v,43,0);edge(v,50,0);edge(peer,80,0);
  detent(1,1);for(unsigned k=0;k<32;k++)detent(2,1);
  edge(v,48,79);edge(v,72,111);mono_note(v,72,79);CHECK(p->sj6_glide_offset<0.f);
  pedal(v,1);edge(v,72,0);mono_note(v,72,79);sample(42); /* still sounds under pedal */
  pedal(v,0);mono_note(v,48,79);CHECK(p->sj6_glide_offset>0.f);
  const float freeze=p->sj6_glide_offset;
  for(unsigned k=0;k<32;k++)raw_detent(2,-1);
  CHECK(j->glide35_detent==0&&p->sj6_glide_offset==freeze);sample(42);CHECK(freeze==p->sj6_glide_offset);
  for(unsigned k=0;k<32;k++)raw_detent(2,1);
  CHECK(j->glide35_detent==32);sample(33);CHECK(p->sj6_glide_offset<freeze);
  edge(v,48,0);CHECK(!all_gates(j));
  edge(v,60,96);edge(v,60,100);edge(v,60,0);mono_note(v,60,96);
  edge(v,60,0);CHECK(!all_gates(j)); /* final aggregate off cannot stick */
  edge(v,47,82);pedal(v,1);edge(v,47,0);mono_note(v,47,82);
  edge(v,68,110);mono_note(v,68,110);CHECK(!p->sj6_glide_offset); /* new physical phrase */
  edge(v,68,0);pedal(v,0);CHECK(!all_gates(j));
  edge(v,47,82);edge(v,70,110);mono_note(v,70,82);
  const float off=p->sj6_glide_offset;CHECK(off<0.f);
  edge(peer,83,80);CHECK(gates(parts_juno[peer],83)==1);
  detent(0,-1);CHECK(!j->mono32_on&&!all_gates(j)&&gates(parts_juno[peer],83)==1);
  CHECK(p->sj6_glide_offset<=0.f); /* release tail not treated as new correction */
  edge(v,70,0);edge(v,47,0);edge(peer,83,0);
  voice(v);detent(0,1);edge(v,52,92);edge(v,78,99);
  pedal(v,1);edge(v,78,0);mono_note(v,78,92);
  const uint8_t *raw=sj6_voice_read(&proj.sound,v);const unsigned tag=raw[0],glide=raw[1];
  all_notes();CHECK(j->mono32_on&&j->legato33_on&&raw[0]==tag&&raw[1]==glide);
  edge(v,78,0);edge(v,52,0);edge(v,60,102);mono_note(v,60,102);
  CHECK(p->sj6_glide_offset==0.f);edge(v,60,0);
  voice(v);detent(0,-1);detent(1,-1);for(unsigned k=0;k<32;k++)detent(2,-1);
 }
 CHECK(store_writes==writes);
 printf("PASS real physical held Mono ON/OFF, duplicate MIDI and pedal sweep, zero-rate pause/resume, actual FX+SEQ All Notes Off and fresh attack, both owners: %u checks\n",checks);
}
