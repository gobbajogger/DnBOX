#define MIXED_ONLY 1
#include "s1_pb_mod_ui.c"
#define CRUSH_LFO 58
extern int s1_typed_crush_choose(unsigned,unsigned);
static unsigned crush_count(void){
 unsigned n=0;for(int i=0;i<pg.n;i++)for(unsigned j=0;j<4;j++)n+=pg.r[i][j].kind==R_S1CRUSH;
 return n;
}
static pref_t crush(unsigned owner)
{
 ui.overlay=O_NONE;ui.part=PART_303A+owner;set_view(V_PART);build_pages();
 visit("FX SWITCHES");smc_peek_cancel();smc_page_set(cur_page());
 CHECK(s1_patch_at(pg.r[cur_page()][2].c)->factory_id==51);
 CHECK(pg.r[cur_page()][3].kind==R_S1RVFB);
 visit("CRUSH MOD");smc_peek_cancel();smc_page_set(cur_page());
 CHECK(crush_count()==1&&pg.r[cur_page()][0].kind==R_S1CRUSH&&pg.r[cur_page()][0].b==owner);
 for(unsigned i=1;i<4;i++)CHECK(pg.r[cur_page()][i].kind==R_NONE);
 return pg.r[cur_page()][0];
}
static void walk(unsigned owner,unsigned next){
 static const char *const names[]={"OFF","LFO 1","LFO 2","BOTH"};
 pref_t r=crush(owner);CHECK(!pref_read_only(r));
 for(unsigned i=0;i<4 && pref_get(r)!=(int)next;++i)detent(0,pref_get(r)<(int)next?1:-1);
 CHECK(pref_get(r)==(int)next&&parts[owner]->e.t[CRUSH_LFO]==(float)next);
 char num[16],unit[8];pref_value(r,num,unit);CHECK(!strcmp(num,names[next])&&!*unit);
 CHECK(read137(&proj.sound,owner)[S1_CRUSH_BYTE]==next);
}
int main(void)
{
 setup(V_PART,PART_303A);proj.set.bt|=PROJ_BT_CLK_IN;select_patch(0);set_view(V_PART);build_pages();
 CHECK(!crush_count()&&!s1_typed_crush_choose(0,1));fresh();
 for(unsigned i=0;i<NBASS;i++){current=NULL;select_patch(i);CHECK(current);parts[i]=current;}
 stabilize();undo_start();CHECK(parts[0]->e.t[CRUSH_LFO]==0.f&&parts[1]->e.t[CRUSH_LFO]==0.f);
 unsigned writes=store_writes;
 walk(0,1);walk(0,3);walk(1,2);CHECK(!s1_typed_crush_choose(0,4));
 CHECK(parts[0]->e.t[CRUSH_LFO]==3&&parts[1]->e.t[CRUSH_LFO]==2&&store_writes==writes);
 CHECK(undo_commit_stable());physical_history(0);CHECK(parts[0]->e.t[CRUSH_LFO]==0.f&&parts[1]->e.t[CRUSH_LFO]==0.f);
 physical_history(1);CHECK(parts[0]->e.t[CRUSH_LFO]==3.f&&parts[1]->e.t[CRUSH_LFO]==2.f);
 project_capture_stage();slot137(&project_stage.sound,0)[S1_CRUSH_BYTE]=4;CHECK(project_validate_stage()<0);
 witness=proj;CHECK(!project_save()&&!project_load_stage());project_commit_stage();stabilize();undo_start();
 CHECK(!memcmp(&witness.sound,&proj.sound,sizeof proj.sound));
 CHECK(parts[0]->e.t[CRUSH_LFO]==3.f&&parts[1]->e.t[CRUSH_LFO]==2.f);
 project_capture_stage();uint8_t h[32];project_file_header(h);CHECK(!project_file_validate(h));
 FILE *f=fopen("out/media.bin","wb");CHECK(f);
 CHECK(fwrite(h,sizeof h,1,f)==1&&fwrite(&project_stage,sizeof project_stage,1,f)==1&&
       fwrite(store,sizeof store,1,f)==1&&fwrite(store_len,sizeof store_len,1,f)==1);
 CHECK(!fclose(f));
 /* Show the restored previous route but do not permit editing or saving it. */
 uint8_t saved[PLAT_STORE_MAX];memcpy(saved,store[OBJ_SOUND],sizeof saved);
 walk(0,1);CHECK(!project_save());memcpy(store[OBJ_SOUND],saved,sizeof saved);
 writes=store_writes;CHECK(!project_load());stabilize();
 pref_t r=crush(0);CHECK(pref_read_only(r)&&pref_get(r)==3);
 detent(0,-1);CHECK(pref_get(r)==3&&parts[0]->e.t[CRUSH_LFO]==3.f&&project_save()==-9&&store_writes==writes);
 printf("PASS actual CRUSH MOD physical choices and typed b20, Undo/Redo and owner recall without touching ID51: %u checks\n",checks);
}
