#define MIXED_ONLY 1
#include "s1_pb_mod_ui.c"
#include "dsp/s1c/s1c_front.h"
#define A4 143
static void event(unsigned owner,unsigned note,unsigned vel)
{
 const unsigned ch=seq.ch[TRK_BASS0+owner];
 host_midi_put(0x9u|((0x90u|ch)<<8)|(note<<16)|(vel<<24));render(1);
}
static float hz(unsigned owner,unsigned note)
{
 for(unsigned i=0;i<S1C_VOICES;i++){
  const s1c_voice *v=&parts[owner]->e.v[i];
  if(v->root==(int)note&&v->gate)return v->freq;
 }
 CHECK(0);return 0;
}
static unsigned tuning_count(void){
 unsigned n=0;
 for(int i=0;i<pg.n;i++)for(unsigned j=0;j<4;j++)n+=pg.r[i][j].kind==R_S1A4;
 return n;
}
static void tuning(unsigned owner)
{
 ui.overlay=O_NONE;ui.part=PART_303A+owner;set_view(V_PART);build_pages();
 visit("TUNING");smc_peek_cancel();smc_page_set(cur_page());
 CHECK(tuning_count()==1);
 CHECK(pg.r[cur_page()][0].kind==R_S1A4&&pg.r[cur_page()][0].b==owner);
 for(unsigned k=1;k<4;k++)CHECK(pg.r[cur_page()][k].kind==R_NONE);
}
static void walk(unsigned owner,unsigned next){
 tuning(owner);
 pref_t r=pg.r[cur_page()][0];CHECK(!pref_read_only(r));
 for(unsigned i=0;i<61 && pref_get(r)+410!=(int)next;++i)
   detent(0,pref_get(r)+410<(int)next?1:-1);
 CHECK(pref_get(r)+410==(int)next);
 char num[16],unit[8],wanted[8];snprintf(wanted,sizeof wanted,"%u",next);
 pref_value(r,num,unit);CHECK(!strcmp(num,wanted)&&!strcmp(unit,"Hz"));
 CHECK(parts[owner]->e.t[A4]==(float)next&&s1_a4_hz(read137(&proj.sound,owner)[S1_A4_BYTE])==next);
}
int main(void)
{
 setup(V_PART,PART_303A);proj.set.bt|=PROJ_BT_CLK_IN;
 select_patch(0);set_view(V_PART);build_pages();
 CHECK(!tuning_count() && !s1_typed_a4_choose(0,430));
 fresh();
 for(unsigned i=0;i<NBASS;i++){current=NULL;select_patch(i);CHECK(current);parts[i]=current;}
 stabilize();undo_start();
 for(unsigned i=410;i<=470;++i)CHECK(s1_a4_hz(s1_a4_code(i))==i);
 CHECK(parts[0]->e.t[A4]==440 && parts[1]->e.t[A4]==440);
 event(0,69,100);event(1,69,91);
 const unsigned saves=store_writes;
 walk(0,410);walk(0,470);
 CHECK(!s1_typed_a4_choose(0,471)&&!s1_typed_a4_choose(0,409));
 CHECK(hz(0,69)==440.f && hz(1,69)==440.f && parts[1]->e.t[A4]==440.f);
 event(0,69,0);event(0,69,100);CHECK(hz(0,69)==470.f);
 walk(1,419);
 event(1,69,0);event(1,69,100);CHECK(hz(1,69)==419.f);
 CHECK(store_writes==saves);
 CHECK(undo_commit_stable());physical_history(0);
 CHECK(parts[0]->e.t[A4]==440.f&&parts[1]->e.t[A4]==440.f&&hz(0,69)==470.f&&hz(1,69)==419.f);
 physical_history(1);CHECK(parts[0]->e.t[A4]==470.f&&parts[1]->e.t[A4]==419.f);
 event(0,69,0);event(1,69,0);render(4);
 project_capture_stage();slot137(&project_stage.sound,0)[S1_A4_BYTE]=61;CHECK(project_validate_stage()<0);
 witness=proj;CHECK(!project_save()&&!project_load_stage());project_commit_stage();stabilize();undo_start();
 CHECK(!memcmp(&witness.sound,&proj.sound,sizeof proj.sound));
 CHECK(parts[0]->e.t[A4]==470.f&&parts[1]->e.t[A4]==419.f);
 event(0,69,100);event(1,69,100);CHECK(hz(0,69)==470.f&&hz(1,69)==419.f);
 project_capture_stage();uint8_t h[32];project_file_header(h);CHECK(!project_file_validate(h));
 FILE *f=fopen("out/media.bin","wb");CHECK(f);
 CHECK(fwrite(h,sizeof h,1,f)==1&&fwrite(&project_stage,sizeof project_stage,1,f)==1&&
       fwrite(store,sizeof store,1,f)==1&&fwrite(store_len,sizeof store_len,1,f)==1);
 CHECK(!fclose(f));
 /* Real writer generations: tear latest Sound, show previous A REF but block actual edit. */
 uint8_t saved[PLAT_STORE_MAX];memcpy(saved,store[OBJ_SOUND],sizeof saved);
 walk(0,469);CHECK(!project_save());
 memcpy(store[OBJ_SOUND],saved,sizeof saved);unsigned writes=store_writes;
 CHECK(!project_load());stabilize();tuning(0);
 pref_t r=pg.r[cur_page()][0];
 CHECK(pref_read_only(r)&&pref_get(r)+410==470);
 detent(0,-1);
 CHECK(parts[0]->e.t[A4]==470&&pref_get(r)+410==470&&project_save()==-9&&writes==store_writes);
 puts("PASS mixed-generation recovery renders exact A REF and disallows K1/Save writes");
 printf("PASS physical S1 TUNING/A REF K1, owner-specific onset-only real MIDI, Undo/Redo and typed restore: %u checks\n",checks);
}
