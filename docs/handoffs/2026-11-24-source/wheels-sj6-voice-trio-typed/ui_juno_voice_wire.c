#include "gate_helpers.inc"
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
static void dry_full(unsigned n)
{
 int32_t lr[512];unsigned nonzero=0;
 while(n--){engine_render(lr,256);for(unsigned i=0;i<512;i++)nonzero+=lr[i]!=0;}
 CHECK(nonzero>256);
}
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
 voice(0);edge(0,48,79);edge(0,60,110);voice0(0,60,79);dry_full(1);
 unsigned cur=undo.cur;physical_history(0);CHECK(undo.cur==cur);
 vals(0,3,17);vals(1,3,18);
 edge(0,60,0);edge(0,48,0);juno6_all_off(parts_juno[0]);
 physical_history(0);vals(0,0,0);vals(1,0,0);
 physical_history(1);vals(0,3,17);vals(1,3,18);
 CHECK(store_writes==writes);
 for(unsigned v=0;v<2;v++){
  project_capture_stage();uint8_t *b=sj6_voice_payload(&project_stage.sound,v);
  b[0]=(uint8_t)(b[0]^4u);CHECK(project_validate_stage()<0);
  project_capture_stage();b=sj6_voice_payload(&project_stage.sound,v);
  b[1]=128;CHECK(project_validate_stage()<0);
 }
 witness=proj;CHECK(!project_save()&&!project_load_stage());project_commit_stage();stabilize();
 /* juno wrapper records latest initialization; select per owner to verify owner-latched state. */
 for(unsigned v=0;v<NBASS;v++){
  vals(v,3,17+v);
  juno=NULL;CHECK(!engine_bass_select_preset(v,ORIGINAL_SOUND_JUNO,0,&proj.sound));
  for(unsigned n=0;n<1024&&engine_bass_preset_pending(v);n++)render(1);
  CHECK(juno);parts_juno[v]=juno;vals(v,3,17+v);
 }
 CHECK(!memcmp(&witness.sound,&proj.sound,sizeof proj.sound));
 for(unsigned v=0;v<NBASS;v++){
   edge(v,48,79);
   const dnbx_layer_voice *l=&parts_juno[v]->core.voice[0].layer[0];
   const float before=l->phase;
   edge(v,60,110);voice0(v,60,79);CHECK(parts_juno[v]->core.sj6_glide_offset<0.f);
   float delta=l->phase-before;if(delta<0.f)delta+=1.f;
   CHECK(l->increment>delta&&fabsf(delta/l->increment-parts_juno[v]->core.sj6_glide_ratio)<.0002f);
   edge(v,60,0);edge(v,48,0);juno6_all_off(parts_juno[v]);
 }
 project_capture_stage();uint8_t h[32];project_file_header(h);CHECK(!project_file_validate(h));
 FILE *f=fopen("out/voice-media.bin","wb");CHECK(f);
 CHECK(fwrite(h,sizeof h,1,f)==1&&fwrite(&project_stage,sizeof project_stage,1,f)==1&&
       fwrite(store,sizeof store,1,f)==1&&fwrite(store_len,sizeof store_len,1,f)==1&&!fclose(f));
 printf("PASS real VOICE trio b36-b37 owner-qualified, held Mono history refuses without writes, Undo/Redo and cold sound+audio restored both parts: %u checks\n",checks);
}
