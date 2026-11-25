#define SJ6_LAYER1_FIXTURE_MAIN retained_layer1_main
#include "ui_superj6_layer1_tuning_test.c"
#include "app/sj6_137_host_witness.h"
_Static_assert(sizeof(sound_t)==2712 && sizeof(project_t)==36420 && sizeof(lane_t)==72,"unchanged ABI");
static project_t witness;
int sj6_137_fixture_lens;
static int choose137(project_t *dst,const project_t *legacy)
{
 if(legacy || dst!=&project_stage)return 0;
 project_reset_stage();
 dst->set.snd_rev=REV137;
 for(unsigned owner=0;owner<NBASS;owner++){
  CHECK(original_sound_init(dst->sound.v[T_303][owner],ORIGINAL_SOUND_JUNO,0));
  uint8_t pos[SJ6_EXTRA_COUNT]={0};
  pos[0]=1;
  original_sound_sj6_extra_store(dst->sound.v[T_303][owner],
      dst->sound.v[T_303][ORIGINAL_SJ6_EXTRA_ROW+owner],1u,pos,0,0);
  engine_bass_sound_refresh(&dst->sound,owner);
  slot137(&dst->sound,owner)[0]=137;
  slot137(&dst->sound,owner)[1]=(uint8_t)owner;
 }
 return 1;
}
static void lens(unsigned v)
{
 ui.part=PART_303A+v;
 sj6_137_fixture_lens=1;
 visit("FILTER ADSR");
 /* Fixture-only ref replacement: exercise the existing real K1 path without
    adding a product page, reassigning a product knob, or adding a gesture. */
 CHECK(pg.r[cur_page()][0].b==v && pg.r[cur_page()][0].c==SJ6_FILTER_VELOCITY);
}
static void walk137(unsigned v,unsigned position)
{
 lens(v);
 const unsigned start=parts[v]->filter_velocity_position;
 const int delta=position>start?1:-1;
 for(unsigned n=start;n!=position;n=(unsigned)((int)n+delta)) {
  now_ms+=300;turn(0,delta);render(1);
 }
 CHECK(parts[v]->filter_velocity_position==position && read137(&proj.sound,v)[2]==position);
}
static void positions(unsigned a,unsigned b)
{
 CHECK(parts[0] && parts[1] && parts[0]!=parts[1]);
 CHECK(parts[0]->filter_velocity_position==a && parts[1]->filter_velocity_position==b);
}
static void recover_audio137(void)
{
 engine_apply_sound(&proj.sound);render(5);
 CHECK(last_synth);
 /* engine_apply_sound restores owners in order; juno6_init wrapper captures
    both current synth addresses. Addresses are globally stable per part. */
}
static float rendered137[2][2][COUNT];
static void audio137(juno6_t p,unsigned arm,unsigned layer,unsigned velocity)
{
 p.core.layer[1-layer].params.gain=0;
 p.core.layer[layer].params.cutoff_hz=3500;
 p.core.layer[layer].params.filter_env_hz=900;
 CHECK(juno6_note_on(&p,60,velocity));
 for(unsigned i=0;i<COUNT;i+=256)
  CHECK(juno6_render_stereo(&p,rendered137[arm][0]+i,rendered137[arm][1]+i,256));
}
static void play137(unsigned owner, unsigned locked, unsigned base, unsigned peer)
{
 bpart_t *line=&proj.pat[0].bass[owner];
 for(unsigned k=0;k<2;++k) for(unsigned n=0;n<NSTEPS;++n) proj.pat[0].bass[k].step[n].flags=G_REST;
 line->len=16; line->rate=RATE_16; line->dir=DIR_FWD; line->transpose=24;
 line->step[3].flags=G_NOTE;line->step[3].note=45;line->step[3].velocity=1;
 line->step[4].flags=G_NOTE;line->step[4].note=45;line->step[4].velocity=127;
 seq.ppat[TRK_BASS0+owner]=0;engine_note_lock_refresh();engine_bass_edit_hold(1);seq_start(&seq);
 for(unsigned n=0;n<3000 && parts[owner]->filter_velocity_position!=locked;++n) render(1);
 CHECK(parts[owner]->filter_velocity_position==locked && parts[1-owner]->filter_velocity_position==peer);
 CHECK(!engine_superj6_knob_turn(owner,SJ6_FILTER_VELOCITY,17,0));
 for(unsigned n=0;n<3000 && parts[owner]->filter_velocity_position!=base;++n) render(1);
 CHECK(parts[owner]->filter_velocity_position==base && parts[1-owner]->filter_velocity_position==peer);
 seq_stop(&seq); engine_bass_edit_hold(0); render(4);
 CHECK(parts[owner]->filter_velocity_position==base && parts[1-owner]->filter_velocity_position==peer);
 engine_bass_edit_hold(1);seq_start(&seq);
 for(unsigned n=0;n<3000 && parts[owner]->filter_velocity_position!=locked;++n) render(1);
 CHECK(parts[owner]->filter_velocity_position==locked);
 seq_stop(&seq);engine_bass_edit_hold(0);render(4);
 CHECK(parts[owner]->filter_velocity_position==base && parts[1-owner]->filter_velocity_position==peer);
}
static int edit137(unsigned v,unsigned value){
 if(v>=NBASS || value>127 || proj.set.snd_rev!=REV137 || !canonical137(&proj.sound) ||
     original_sound_sj6_factory(proj.sound.v[T_303][v])!=0 ||
     engine_bass_sound_bank(&proj.sound,v)!=ORIGINAL_SOUND_JUNO)return 0;
 slot137(&proj.sound,v)[2]=(uint8_t)value;
 return 1;
}
int main(void){
 setup_tuning(0);ui.overlay=O_NONE;proj.set.bt|=PROJ_BT_CLK_IN;
 memset(slot137(&proj.sound,0),0,32);memset(slot137(&proj.sound,1),0xa5,32);
 witness=proj;
 CHECK(!choose137(&project_stage,&proj) && !edit137(0,3) && !memcmp(&witness,&proj,sizeof proj));
 CHECK(!project_save()); CHECK(!project_load()); 
 CHECK(!memcmp(slot137(&proj.sound,0),slot137(&witness.sound,0),32));
 CHECK(!memcmp(slot137(&proj.sound,1),slot137(&witness.sound,1),32));
 /* Avoid replacing even our synthetic existing fixture. Start on separately fresh virtual media. */
 for(unsigned i=0;i<OBJ_NOBJ;i++)store_len[i]=store_previous_len[i]=-1;
 memset(store,0,sizeof store);memset(store_previous,0,sizeof store_previous);
 CHECK(choose137(&project_stage,NULL));
 CHECK(!project_validate_stage());project_commit_stage();undo_start();
 recover_audio137();
 positions(0,0);
 for(unsigned v=0;v<2;++v){
  const unsigned final=v ? 127:0;
  const sound_t opaque=proj.sound;
  lens(v);
  for(unsigned det=1;det<128;++det) {
   now_ms+=300;turn(0,1);render(1);
   CHECK(parts[v]->filter_velocity_position==det && read137(&proj.sound,v)[2]==det);
   CHECK(parts[1-v]->filter_velocity_position==0 && read137(&proj.sound,1-v)[2]==0);
   CHECK(parts[v]->filter_velocity_scale==(float)(1.1*(double)((float)det/127.f)));
  }
  if(!final) for(unsigned det=127;det;--det) {
   now_ms+=300;turn(0,-1);render(1);
   CHECK(parts[v]->filter_velocity_position==det-1 && read137(&proj.sound,v)[2]==det-1);
  }
  CHECK(!memcmp(opaque.v[T_303][v],proj.sound.v[T_303][v],32));
  CHECK(!memcmp(opaque.v[T_303][1-v],proj.sound.v[T_303][1-v],32));
 }
 positions(0,127);
 for(unsigned v=0;v<2;++v) {
  const juno6_t current=*parts[v];
  walk137(v,v?0:127);
  for(unsigned l=0;l<2;++l) {
   audio137(current,0,l,1);audio137(*parts[v],1,l,1);
   CHECK(memcmp(rendered137[0],rendered137[1],sizeof rendered137[0]));
   audio137(current,0,l,127);audio137(*parts[v],1,l,127);
   CHECK(!memcmp(rendered137[0],rendered137[1],sizeof rendered137[0]));
  }
  walk137(v,v?127:0);
 }
 positions(0,127);
 CHECK(!edit137(0,128));
 engine_note_lock_capture(1);
 CHECK(!engine_bass_note_capture(proj.arr.lane,0,0,3,&proj.sound));
 CHECK(!engine_bass_note_capture(proj.arr.lane,0,1,3,&proj.sound));
 engine_note_lock_capture(0);
 int a=bass_note_find(proj.arr.lane,0,0,3),b=bass_note_find(proj.arr.lane,0,1,3);
 CHECK(a>=0 && b>=0 && a!=b);
 CHECK((original_sound_edit_mask(proj.arr.lane[a].val) & ORIGINAL_SJ6_EXTRA_BIT) &&
       BASS_NOTE_USED+ORIGINAL_SJ6_EXTRA_SIZE==64);
 CHECK(proj.arr.lane[a].i==NOTE137 && proj.arr.lane[b].i==NOTE137);
 CHECK(proj.arr.lane[a].val[BASS_NOTE_TAG+11]==0 && proj.arr.lane[b].val[BASS_NOTE_TAG+11]==127);
 CHECK(!engine_bass_note_validate(&proj.arr.lane[a])&&!engine_bass_note_validate(&proj.arr.lane[b]));
 CHECK(undo_commit_stable());
 walk137(0,53);walk137(1,47);positions(53,47);
 CHECK(undo_commit_stable());
 do_undo(0);positions(0,127);CHECK(read137(&proj.sound,0)[2]==0 && read137(&proj.sound,1)[2]==127);
 do_undo(1);positions(53,47);CHECK(read137(&proj.sound,0)[2]==53 && read137(&proj.sound,1)[2]==47);
 CHECK(proj.arr.lane[a].val[BASS_NOTE_TAG+11]==0 && proj.arr.lane[b].val[BASS_NOTE_TAG+11]==127);
 project_capture_stage();
 project_stage.arr.lane[a].i=BASS_NOTE_VERSION; /* even neutral value needs typed lock */
 CHECK(project_validate_stage()<0);
 project_stage.arr.lane[a].i=NOTE137;
 project_stage.arr.lane[a].val[BASS_NOTE_TAG+11]=128;
 CHECK(project_validate_stage()<0);
 project_stage.arr.lane[a].val[BASS_NOTE_TAG+11]=0;
 uint8_t h[32];project_file_header(h);CHECK(!project_file_validate(h));
 witness=proj;CHECK(!project_save() && !project_load_stage());project_commit_stage();
 CHECK(!memcmp(&witness.sound,&proj.sound,sizeof(sound_t)) && !memcmp(witness.arr.lane,proj.arr.lane,sizeof proj.arr.lane));
 CHECK(!project_load());CHECK(!memcmp(&witness.sound,&proj.sound,sizeof(sound_t)));
 CHECK(!memcmp(witness.arr.lane,proj.arr.lane,sizeof proj.arr.lane));
 recover_audio137();positions(53,47);
 project_capture_stage();project_file_header(h);
 FILE *f=fopen("out/media.bin","wb");CHECK(f);
 CHECK(fwrite(h,sizeof h,1,f)==1&&fwrite(&project_stage,sizeof project_stage,1,f)==1&&
       fwrite(store,sizeof store,1,f)==1&&fwrite(store_len,sizeof store_len,1,f)==1);
 CHECK(!fclose(f));
 play137(0,0,53,47);
 play137(1,127,47,53);
 /* LAB-ONLY RED: normal object CRCs and version markers cannot detect
    a valid previous typed SOUND combined with a valid new typed stream. */
 CHECK(edit137(0,89));
 proj.arr.lane[a].val[BASS_NOTE_TAG+11]=22;
 CHECK(!project_save());
 CHECK(store_len[OBJ_SOUND]==store_previous_len[OBJ_SOUND]);
 memcpy(store[OBJ_SOUND],store_previous[OBJ_SOUND],(size_t)store_len[OBJ_SOUND]);
 CHECK(!project_load());
 CHECK(read137(&proj.sound,0)[2]==53 && proj.arr.lane[a].val[BASS_NOTE_TAG+11]==22);
 puts("EXPECTED RED product admission: same-version torn typed SOUND/stream accepted");
 /* Typed storage cannot splice an old stream or sound into the new data. */
 store[OBJ_PAT0][0]=13;
 uint32_t w=store_writes;
 CHECK(project_load()==-9 && project_save()==-9 && store_writes==w);
 printf("PASS %u checks: physical all detents both owners, signed dual layer/AMP unity, real save/load/CRC, undo, V2 sequence onset/unlocked/Stop, ordinary opaque legacy retained\n",checks);
 return 0;
}
