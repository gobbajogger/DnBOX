/* SPDX-License-Identifier: GPL-3.0-only */
#define MIXED_ONLY 1
#include "s1_pb_mod_ui.c"
#include "dsp/juno6.h"
static juno6_t *parts_j[2];
static unsigned initialized;
void __real_juno6_init(juno6_t *,float);
void __wrap_juno6_init(juno6_t *p,float sr){__real_juno6_init(p,sr);for(unsigned i=0;i<initialized;i++)if(parts_j[i]==p)return;CHECK(initialized<2);parts_j[initialized++]=p;}
static unsigned count_page(const char *name)
{
 unsigned n=0;for(int i=0;i<pg.n;i++)n+=!strcmp(page_title(&pg,i),name);return n;
}
static void owner_view(unsigned owner)
{
 ui.overlay=O_NONE;ui.part=PART_303A+owner;set_view(V_PART);build_pages();
}
static void arrange(void)
{
 project_reset_stage();project_stage.set.snd_rev=REV137;
 for(unsigned i=0;i<NBASS;i++){
  CHECK(original_sound_init(project_stage.sound.v[T_303][i],ORIGINAL_SOUND_JUNO,0));
  engine_bass_sound_refresh(&project_stage.sound,i);
  slot137(&project_stage.sound,i)[0]=137;
  slot137(&project_stage.sound,i)[1]=(uint8_t)i;
 }
 CHECK(!project_validate_stage());initialized=0;project_commit_stage();stabilize();
 CHECK(initialized==2 && parts_j[0]!=parts_j[1]);undo_start();
}
static void goto_delay(unsigned v)
{
 owner_view(v);CHECK(count_page("DELAY")==1);visit("DELAY");smc_peek_cancel();smc_page_set(cur_page());
 static const unsigned slots[]={SJ6_DELAY24,SJ6_DELAY25,SJ6_DELAY26,SJ6_DELAY27};
 static const char *labels[]={"ENABLE","TIME","FEEDBACK","MIX"};
 for(unsigned i=0;i<4;i++){
  const pref_t r=pg.r[cur_page()][i];
  CHECK(r.kind==R_SJ6KNOB && r.b==v && r.c==slots[i]);
  CHECK(!strcmp(pref_desc(r)->name,labels[i]) && !pref_read_only(r));
 }
}
static void goto_clock(unsigned v)
{
 owner_view(v);CHECK(count_page("DELAY CLOCK")==1);visit("DELAY CLOCK");
 smc_peek_cancel();smc_page_set(cur_page());
 const pref_t r=pg.r[cur_page()][0];
 CHECK(r.kind==R_SJ6KNOB && r.b==v && r.c==SJ6_DELAYSYNC);
 CHECK(!strcmp(pref_desc(r)->name,"CLOCK"));
}
static uint8_t zero[4];
static uint8_t wanted[2][4];
static const uint8_t *bs(unsigned v){return read137(&proj.sound,v)+SJ6_DELAY_BYTE;}
static void match_runtime(unsigned v)
{
 goto_delay(v);
 const juno6_t *j=parts_j[v];
 CHECK(j->delay24.tap && j->delay24.left!=parts_j[1-v]->delay24.left);
 for(unsigned i=0;i<4;i++){
  unsigned expected=i?bs(v)[i]:bs(v)[0]&1u;
  CHECK(pref_get(pg.r[cur_page()][i])==(int)expected);
  CHECK(juno6_delay_position(j,pg.r[cur_page()][i].c)==expected);
 }
 CHECK(j->delay24.fb_target==.99f*((float)bs(v)[2]/127.f));
 const unsigned sync=(bs(v)[0]&2u)!=0;
 CHECK(j->delay24.delay_target==(sync?.03125f*(240.f/seq_tempo(&seq))*44100.f:
    bs(v)[1]?1.5f*((float)bs(v)[1]/127.f)*44100.f:1.f));
 goto_clock(v);
 CHECK(pref_get(pg.r[cur_page()][0])==(int)sync&&j->delay24.sync==sync);
}
static void physical_value(unsigned v,unsigned col,unsigned value)
{
 goto_delay(v);const pref_t r=pg.r[cur_page()][col];
 for(unsigned i=0;pref_get(r)!=(int)value && i<256;++i){
  detent(col,pref_get(r)<(int)value?1:-1);render(1);
 }
 CHECK(pref_get(r)==(int)value && bs(v)[col]==value);
}
static int validate_bad(unsigned col,unsigned bad)
{
 project_capture_stage();slot137(&project_stage.sound,0)[SJ6_DELAY_BYTE+col]=(uint8_t)bad;
 return project_validate_stage();
}
int main(void)
{
 setup(V_PART,PART_303A);proj.set.bt|=PROJ_BT_CLK_IN;
 owner_view(0);CHECK(!count_page("DELAY"));
 arrange();
 seq.ext=0;seq.bpm=120;render(1);
 const unsigned writes_before=store_writes;
 const sound_t carry=proj.sound;
 for(unsigned v=0;v<2;v++) {
  CHECK(!memcmp(bs(v),zero,4));
  goto_clock(v);CHECK(!pref_get(pg.r[cur_page()][0]) && !parts_j[v]->delay24.sync);
  physical_value(v,0,1);physical_value(v,1,15+v);physical_value(v,2,4+v);physical_value(v,3,5+v);
  goto_clock(v);detent(0,1);render(1);
  CHECK(bs(v)[0]==3 && parts_j[v]->delay24.sync==1 && parts_j[v]->delay24.time_raw==15+v);
  memcpy(wanted[v],bs(v),4);
  CHECK(!memcmp(bs(1-v),wanted[1-v],4));match_runtime(v);
 }
 CHECK(store_writes==writes_before);
 /* A real tempo tick can exceed the short-ring support without corrupting
    authored intent. Two running DSP owners independently refuse wet. */
 seq.bpm=20;render(2);
 for(unsigned v=0;v<2;v++){
  CHECK(parts_j[v]->delay24.clock_refused && !memcmp(bs(v),wanted[v],4));
  goto_delay(v);char num[16],unit[16];pref_value(pg.r[cur_page()][1],num,unit);
  CHECK(!strcmp(num,"TOO LONG"));
  detent(1,1);render(1);CHECK(!memcmp(bs(v),wanted[v],4));
  unsigned pos=parts_j[v]->delay24.pos;float l=.3125f,r=-.125f;
  juno6_delay24_process(parts_j[v],&l,&r,1);
  CHECK(l==.3125f && r==-.125f && parts_j[v]->delay24.pos!=pos);
 }
 seq.bpm=120;render(1);
 for(unsigned v=0;v<2;v++){CHECK(!parts_j[v]->delay24.clock_refused);match_runtime(v);}
 goto_delay(0);CHECK(!engine_superj6_knob_turn(0,SJ6_DELAY25,127,0));render(1);
 CHECK(!memcmp(bs(0),wanted[0],4));
 CHECK(undo_commit_stable());CHECK(undo.n>0);
 for(unsigned i=0;undo.cur>0 && i<64;i++)physical_history(0);
 CHECK(!undo.cur && !memcmp(bs(0),zero,4) && !memcmp(bs(1),zero,4));match_runtime(0);match_runtime(1);
 for(unsigned i=0;undo.cur<undo.n && i<64;i++)physical_history(1);
 CHECK(undo.cur==undo.n && !memcmp(bs(0),wanted[0],4) && !memcmp(bs(1),wanted[1],4));
 for(unsigned v=0;v<2;v++){
  match_runtime(v);
  CHECK(validate_bad(v,128)<0);
 }
 CHECK(validate_bad(0,4)<0 && validate_bad(2,128)<0 && validate_bad(3,128)<0);
 project_capture_stage();
 for(unsigned v=0;v<2;v++)memcpy(slot137(&project_stage.sound,v)+SJ6_DELAY_BYTE,zero,4);
 CHECK(!memcmp(&carry,&project_stage.sound,sizeof carry));
 CHECK(!project_save()&&!project_load_stage());project_commit_stage();stabilize();
 for(unsigned v=0;v<2;v++){CHECK(!memcmp(bs(v),wanted[v],4));match_runtime(v);}
 CHECK(!project_load());stabilize();
 const unsigned writes_reopened=store_writes;
 seq.ext=0;seq.bpm=96;render(2);
 for(unsigned v=0;v<2;v++){CHECK(!memcmp(bs(v),wanted[v],4));match_runtime(v);}
 CHECK(store_writes==writes_reopened);
 project_capture_stage();uint8_t h[32];project_file_header(h);CHECK(!project_file_validate(h));
 FILE *f=fopen("out/delay-media.bin","wb");CHECK(f);
 CHECK(fwrite(h,sizeof h,1,f)==1&&fwrite(&project_stage,sizeof project_stage,1,f)==1&&
       fwrite(store,sizeof store,1,f)==1&&fwrite(store_len,sizeof store_len,1,f)==1);
 CHECK(!fclose(f));
 uint8_t old_sound_obj[PLAT_STORE_MAX];
 memcpy(old_sound_obj,store[OBJ_SOUND],sizeof old_sound_obj);
 physical_value(0,2,9);CHECK(!project_save());
 memcpy(store[OBJ_SOUND],old_sound_obj,sizeof old_sound_obj);
 const unsigned writes=store_writes;
 CHECK(!project_load());stabilize();
 CHECK(!memcmp(bs(0),wanted[0],4) && !memcmp(bs(1),wanted[1],4));
 owner_view(0);visit("DELAY");
 CHECK(pref_read_only(pg.r[cur_page()][2]));
 detent(2,1);render(3);
 CHECK(!memcmp(bs(0),wanted[0],4) && project_save()==-9 && store_writes==writes);
 printf("PASS typed SJ6 DELAY clock + four physical controls/Undo/Redo both owners, short-ring tempo refusal and staged/native virtual reopen: %u checks\n",checks);
}
