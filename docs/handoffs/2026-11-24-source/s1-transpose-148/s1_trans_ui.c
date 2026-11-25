#define MIXED_ONLY 1
#include "s1_pb_mod_ui.c"
#include "dsp/s1c/s1c_front.h"
#define TRANSPOSE 148
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
 for(int i=0;i<pg.n;i++)for(unsigned j=0;j<4;j++)n+=pg.r[i][j].kind==R_S1TRANS;
 return n;
}
static void tuning(unsigned owner)
{
 ui.overlay=O_NONE;ui.part=PART_303A+owner;set_view(V_PART);build_pages();
 visit("TUNING");smc_peek_cancel();smc_page_set(cur_page());
 CHECK(tuning_count()==1);
 CHECK(pg.r[cur_page()][0].kind==R_S1A4 && pg.r[cur_page()][1].kind==R_S1TRANS && pg.r[cur_page()][1].b==owner);
 for(unsigned k=2;k<4;k++)CHECK(pg.r[cur_page()][k].kind==R_NONE);
}
static void walk(unsigned owner,int next){
 tuning(owner);
 pref_t r=pg.r[cur_page()][1];CHECK(!pref_read_only(r));
 for(unsigned i=0;i<49 && pref_get(r)-24!=next;++i)detent(1,pref_get(r)-24<next?1:-1);
 CHECK(pref_get(r)-24==next);
 char num[16],unit[8],wanted[12];snprintf(wanted,sizeof wanted,"%d",next);
 pref_value(r,num,unit);CHECK(!strcmp(num,wanted)&&!strcmp(unit,"st"));
 CHECK(parts[owner]->e.t[TRANSPOSE]==(float)next&&s1_trans_st(read137(&proj.sound,owner)[S1_TRANS_BYTE])==next);
}
static void sequence_proof(unsigned owner)
{
 for(unsigned i=0;i<NBASS;i++)for(unsigned j=0;j<NSTEPS;j++)proj.pat[0].bass[i].step[j].flags=G_REST;
 bpart_t *bp=&proj.pat[0].bass[owner];
 bp->len=16;bp->rate=RATE_16;bp->dir=DIR_FWD;bp->transpose=36; /* independent +12 pattern */
 bp->step[0].flags=G_NOTE;bp->step[0].note=45;bp->step[0].velocity=100;
 seq.ppat[TRK_BASS0+owner]=0;engine_bass_edit_hold(1);seq_start(&seq);
 int found=0;
 for(unsigned n=0;n<2400&&!found;n++){
   render(1);
   for(unsigned i=0;i<S1C_VOICES;i++)if(parts[owner]->e.v[i].root==57 && parts[owner]->e.v[i].gate)found=1;
 }
 CHECK(found&&hz(owner,57)==440.f); /* pattern 45+12 makes root 57; S1 +12 tunes to MIDI 69 */
 seq_stop(&seq);engine_bass_edit_hold(0);render(4);
 CHECK(bp->transpose==36);
}
int main(void)
{
 setup(V_PART,PART_303A);proj.set.bt|=PROJ_BT_CLK_IN;
 select_patch(0);set_view(V_PART);build_pages();
 CHECK(!tuning_count() && !s1_typed_trans_choose(0,12));
 fresh();
 for(unsigned i=0;i<NBASS;i++){current=NULL;select_patch(i);CHECK(current);parts[i]=current;}
 stabilize();undo_start();
 for(int i=-24;i<=24;++i)CHECK(s1_trans_st(s1_trans_code(i))==i);
 CHECK(parts[0]->e.t[TRANSPOSE]==0 && parts[1]->e.t[TRANSPOSE]==0);
 CHECK(parts[0]->e.t[143]==440 && parts[1]->e.t[143]==440);
 event(0,69,100);event(1,69,91);
 const unsigned saves=store_writes;
 walk(0,-24);walk(0,24);
 CHECK(!s1_typed_trans_choose(0,25)&&!s1_typed_trans_choose(0,-25));
 CHECK(hz(0,69)==440.f && hz(1,69)==440.f && parts[1]->e.t[TRANSPOSE]==0.f);
 event(0,69,0);event(0,69,100);CHECK(hz(0,69)==1760.f);
 walk(1,-12);
 event(1,69,0);event(1,69,100);CHECK(hz(1,69)==220.f);
 CHECK(store_writes==saves);
 CHECK(undo_commit_stable());physical_history(0);
 CHECK(parts[0]->e.t[TRANSPOSE]==0.f&&parts[1]->e.t[TRANSPOSE]==0.f&&hz(0,69)==1760.f&&hz(1,69)==220.f);
 physical_history(1);CHECK(parts[0]->e.t[TRANSPOSE]==24.f&&parts[1]->e.t[TRANSPOSE]==-12.f);
 event(0,127,100);event(1,1,100);
 CHECK(hz(0,127)==440.f*exp2f((127.f-69.f)/12.f));
 CHECK(hz(1,1)==440.f*exp2f(-69.f/12.f));
 event(0,127,0);event(1,1,0);
 event(0,69,0);event(1,69,0);render(4);
 /* Original mono fallback resolves the remaining raw key with current synth transpose. */
 CHECK(s1c_front_set(&parts[0]->e,20,1.f));walk(0,0);
 event(0,69,100);event(0,81,100);CHECK(hz(0,81)==880.f);
 walk(0,12);CHECK(hz(0,81)==880.f);
 event(0,81,0);CHECK(hz(0,69)==880.f);
 event(0,69,0);render(4);CHECK(s1c_front_set(&parts[0]->e,20,0.f));
 sequence_proof(0);
 project_capture_stage();slot137(&project_stage.sound,0)[S1_TRANS_BYTE]=49;CHECK(project_validate_stage()<0);
 witness=proj;CHECK(!project_save()&&!project_load_stage());project_commit_stage();stabilize();undo_start();
 CHECK(!memcmp(&witness.sound,&proj.sound,sizeof proj.sound));
 CHECK(parts[0]->e.t[TRANSPOSE]==12.f&&parts[1]->e.t[TRANSPOSE]==-12.f);
 CHECK(proj.pat[0].bass[0].transpose==36);
 event(0,69,100);event(1,69,100);CHECK(hz(0,69)==880.f&&hz(1,69)==220.f);
 project_capture_stage();uint8_t h[32];project_file_header(h);CHECK(!project_file_validate(h));
 FILE *f=fopen("out/media.bin","wb");CHECK(f);
 CHECK(fwrite(h,sizeof h,1,f)==1&&fwrite(&project_stage,sizeof project_stage,1,f)==1&&fwrite(store,sizeof store,1,f)==1&&fwrite(store_len,sizeof store_len,1,f)==1);
 CHECK(!fclose(f));
 /* Resolved old coherent typed generation displays value but rejects edits. */
 uint8_t saved[PLAT_STORE_MAX];memcpy(saved,store[OBJ_SOUND],sizeof saved);
 walk(0,7);CHECK(!project_save());memcpy(store[OBJ_SOUND],saved,sizeof saved);
 unsigned w=store_writes;CHECK(!project_load());stabilize();tuning(0);
 pref_t r=pg.r[cur_page()][1];CHECK(pref_read_only(r)&&pref_get(r)==36);
 detent(1,-1);CHECK(pref_get(r)==36&&parts[0]->e.t[TRANSPOSE]==12.f&&project_save()==-9&&store_writes==w);
 puts("PASS recovered typed transpose is readable without durable or physical editing");
 printf("PASS actual Synth One ID148 TRANSPOSE K2, onset/held/mono fallback, independent composed pattern offset, undo/recall: %u checks\n",checks);
}
