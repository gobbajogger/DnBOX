#define MIXED_ONLY 1
#include "s1_pb_mod_ui.c"
#include "app/original_runtime.h"
extern const classic_mode_i *sj6_classic_i_probe(unsigned);
static const uint8_t *ci(unsigned v){return read137(&proj.sound,v)+SJ6_CLASSIC_I_BYTE;}
static void owner(unsigned v){
 ui.overlay=O_NONE;ui.part=PART_303A+v;set_view(V_PART);build_pages();
}
static void prepare(void){
 project_reset_stage();project_stage.set.snd_rev=REV137;
 for(unsigned v=0;v<NBASS;v++){
  CHECK(original_sound_init(project_stage.sound.v[T_303][v],ORIGINAL_SOUND_JUNO,0));
  engine_bass_sound_refresh(&project_stage.sound,v);
  slot137(&project_stage.sound,v)[0]=137;slot137(&project_stage.sound,v)[1]=v;
 }
 CHECK(!project_validate_stage());project_commit_stage();stabilize();
}
static void tune(unsigned v){
 owner(v);visit("CHORUS");smc_peek_cancel();smc_page_set(cur_page());
 CHECK(pg.r[cur_page()][0].c==SJ6_CHORUS_I && pg.r[cur_page()][1].c==SJ6_CHORUS_II);
 if(!pref_get(pg.r[cur_page()][0])){detent(0,1);render(2);owner(v);visit("CHORUS");}
 CHECK(pref_get(pg.r[cur_page()][0])==1&&!pref_get(pg.r[cur_page()][1]));
 CHECK(pg.r[cur_page()][2].kind==R_NONE&&pg.r[cur_page()][3].kind==R_NONE);
 now_ms+=160;enc_acc[EN_SELECT]=1;input();
 CHECK(!strcmp(page_title(&pg,cur_page()),"CHORUS I"));smc_peek_cancel();smc_page_set(cur_page());
 for(unsigned i=0;i<3;i++)CHECK(pg.r[cur_page()][i].kind==R_SJ6CLASSIC_I&&pg.r[cur_page()][i].b==v);
}
static void classic_play(unsigned v) {
 uint32_t gen,ctx;int arp;engine_live_key_state(v,&gen,&arp,&ctx);
 CHECK(engine_live_key(v,gen,1,48,100,0,0));
 int32_t mixed[512],signal=0;
 for(unsigned t=0;t<12;++t){
  engine_render(mixed,256);
  for(unsigned j=0;j<512;++j)signal|=mixed[j];
 }
 CHECK(signal);
 CHECK(engine_live_key(v,gen,1,48,0,0,0));render(2);
}
int main(void){
 setup(V_PART,PART_303A);proj.set.bt|=PROJ_BT_CLK_IN;
 CHECK(page_named("CHORUS I")<0);
 prepare();
 for(unsigned v=0;v<2;v++)tune(v);
 undo_start();
 const unsigned w=store_writes;
 for(unsigned v=0;v<2;v++){
  tune(v);CHECK(sj6_classic_i_probe(v));
  uint8_t peer[3];memcpy(peer,ci(1-v),3);
  for(unsigned k=0;k<3;k++)for(unsigned det=0;det<4+v;++det)detent(k,1);
  const classic_mode_i *fx=sj6_classic_i_probe(v);CHECK(fx&&fx->started);
  CHECK(!memcmp(peer,ci(1-v),3));CHECK(ci(v)[0]&&ci(v)[1]&&ci(v)[2]);
  CHECK(fx->p[0].pending==(float)(16.*pow((ci(v)[0]-1u)/127.f,3.33)));
  CHECK(fx->p[1].pending==(float)(.97*((ci(v)[1]-1u)/127.f)));
  CHECK(fx->p[3].pending==(ci(v)[2]-1u)/127.f);
  classic_play(v);
  CHECK(fx->core.phase>0&&fx->core.write[0]!=0&&fx->p[0].pending>.0f&&fx->p[1].pending>.0f&&fx->p[3].pending>.0f);
  double sum=0;for(unsigned i=0;i<CLASSIC_I_DELAY_FRAMES;i++)sum+=fabs(fx->core.ring[0][i]);
  CHECK(sum>1.);
 }
 CHECK(w==store_writes);
 CHECK(undo_commit_stable());physical_history(0);
 for(unsigned v=0;v<2;v++)CHECK(ci(v)[0]==SJ6_CLASSIC_I_DEFAULT&&ci(v)[1]==SJ6_CLASSIC_I_DEFAULT&&ci(v)[2]==SJ6_CLASSIC_I_DEFAULT&&sj6_classic_i_probe(v));
 physical_history(1);
 for(unsigned v=0;v<2;v++)CHECK(ci(v)[0]&&ci(v)[1]&&ci(v)[2]&&sj6_classic_i_probe(v));
 witness=proj;CHECK(!project_save()&&!project_load_stage());project_commit_stage();stabilize();
 CHECK(!memcmp(&witness.sound,&proj.sound,sizeof proj.sound));
 for(unsigned v=0;v<2;v++){tune(v);CHECK(sj6_classic_i_probe(v));classic_play(v);}
 project_capture_stage();slot137(&project_stage.sound,0)[SJ6_CLASSIC_I_BYTE]=130;CHECK(project_validate_stage()<0);
 project_capture_stage();uint8_t h[32];project_file_header(h);CHECK(!project_file_validate(h));
 FILE *f=fopen("out/media.bin","wb");CHECK(f);
 CHECK(fwrite(h,sizeof h,1,f)==1&&fwrite(&project_stage,sizeof project_stage,1,f)==1&&
       fwrite(store,sizeof store,1,f)==1&&fwrite(store_len,sizeof store_len,1,f)==1);
 CHECK(!fclose(f));
 uint8_t prev[PLAT_STORE_MAX],desired[3];memcpy(prev,store[OBJ_SOUND],sizeof prev);memcpy(desired,ci(0),3);
 tune(0);detent(0,1);CHECK(!project_save());memcpy(store[OBJ_SOUND],prev,sizeof prev);
 unsigned n=store_writes;CHECK(!project_load());stabilize();tune(0);
 CHECK(pref_read_only(pg.r[cur_page()][0])&&!memcmp(ci(0),desired,3));
 detent(0,-1);CHECK(!memcmp(ci(0),desired,3)&&project_save()==-9&&store_writes==n);
 printf("PASS Classic I typed SMK switches/parameters, audible isolated owners, Undo/Redo, actual project writer/reopen and torn read-only: %u checks\n",checks);
}
