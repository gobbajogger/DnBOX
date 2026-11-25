#define MIXED_ONLY 1
#include "s1_pb_mod_ui.c"
static pref_t pan_ui(unsigned owner,unsigned k){
 ui.overlay=O_NONE;ui.part=PART_303A+owner;set_view(V_PART);build_pages();
 visit("AUTO PAN");smc_peek_cancel();smc_page_set(cur_page());
 CHECK(pg.r[cur_page()][0].kind==R_S1PAN&&pg.r[cur_page()][1].kind==R_S1PAN);
 CHECK(pg.r[cur_page()][k].b==owner&&pg.r[cur_page()][k].c==k);
 CHECK(pg.r[cur_page()][2].kind==R_NONE&&pg.r[cur_page()][3].kind==R_NONE);
 return pg.r[cur_page()][k];
}
static void walk(unsigned owner,unsigned k,unsigned target){
 pref_t r=pan_ui(owner,k);CHECK(!pref_read_only(r));
 for(unsigned n=0;n<128&&pref_get(r)!=(int)target;n++)detent(k,pref_get(r)<(int)target?1:-1);
 CHECK(pref_get(r)==(int)target);
}
static void sync_panel(unsigned owner,int enabled){
 ui.overlay=O_NONE;ui.part=PART_303A+owner;set_view(V_PART);build_pages();
 visit("TEMPO SYNC");smc_peek_cancel();smc_page_set(cur_page());
 CHECK(s1_patch_at(pg.r[cur_page()][0].c)->factory_id==139);
 for(unsigned i=0;i<3&&parts[owner]->e.t[139]!=(float)enabled;i++)detent(0,enabled?1:-1);
 CHECK(parts[owner]->e.t[139]==(float)enabled);
}
static float raw(unsigned owner){return s1_pan_rate_hz(s1_pan_rate_word(read137(&proj.sound,owner)));}
int main(void){
 setup(V_PART,PART_303A);proj.set.bt|=PROJ_BT_CLK_IN;select_patch(0);set_view(V_PART);build_pages();
 CHECK(page_named("AUTO PAN")<0);stabilize();CHECK(current);
 float previous_default=current->e.t[36];
 fresh();
 for(unsigned i=0;i<NBASS;i++){current=NULL;select_patch(i);CHECK(current);parts[i]=current;}
 stabilize();undo_start();
 CHECK(parts[0]->e.t[35]==0.f&&parts[1]->e.t[35]==0.f);
 fprintf(stderr,"stock inherited pan Hz %.9g typed %.9g/%.9g\n",previous_default,parts[0]->e.t[36],parts[1]->e.t[36]);
 CHECK(s1c_front_init(36)==.25f&&parts[0]->e.t[36]==previous_default&&parts[1]->e.t[36]==previous_default);
 unsigned writes=store_writes;
 walk(0,0,95);walk(1,0,21);
 CHECK(parts[0]->e.t[35]==95.f/127.f&&parts[1]->e.t[35]==21.f/127.f);
 sync_panel(0,0);sync_panel(1,0);
 walk(0,1,79);walk(1,1,44);
 float free0=parts[0]->e.t[36],free1=parts[1]->e.t[36];
 CHECK(raw(0)==free0&&raw(1)==free1&&free0!=free1);
 CHECK(undo_commit_stable());
 sync_panel(0,1);
 float synced0=parts[0]->e.t[36];
 CHECK(synced0!=free0&&raw(0)==synced0&&raw(1)==free1);
 pref_t r=pan_ui(0,1);
 char value[24],unit[8];pref_value(r,value,unit);CHECK(*value&&!*unit);
 unsigned bound=pref_desc(r)->max;walk(0,1,bound);
 CHECK(raw(0)==parts[0]->e.t[36]&&raw(0)<=10.f&&raw(1)==free1);
 CHECK(store_writes==writes&&undo_commit_stable());
 physical_history(0);
 CHECK(raw(0)==free0&&parts[0]->e.t[36]==free0&&parts[0]->e.t[139]==0);
 physical_history(1);CHECK(raw(0)==parts[0]->e.t[36]&&raw(0)<=10.f&&parts[0]->e.t[139]==1);
 float saved0=raw(0);
 project_capture_stage();
 s1_pan_rate_put(slot137(&project_stage.sound,0),NAN);CHECK(project_validate_stage()<0);
 project_capture_stage();
 s1_pan_rate_put(slot137(&project_stage.sound,0),-0.f);CHECK(project_validate_stage()<0);
 project_capture_stage();
 s1_pan_rate_put(slot137(&project_stage.sound,0),10.01f);CHECK(project_validate_stage()<0);
 witness=proj;CHECK(!project_save()&&!project_load_stage());project_commit_stage();stabilize();
 CHECK(!memcmp(&witness.sound,&proj.sound,sizeof proj.sound)&&raw(0)==saved0&&raw(1)==free1);
 CHECK(parts[0]->e.t[36]==saved0&&parts[1]->e.t[36]==free1);
 CHECK(parts[0]->e.t[35]==95.f/127.f&&parts[1]->e.t[35]==21.f/127.f);
 project_capture_stage();uint8_t h[32];project_file_header(h);CHECK(!project_file_validate(h));
 FILE *f=fopen("out/media.bin","wb");CHECK(f);
 CHECK(fwrite(h,sizeof h,1,f)==1&&fwrite(&project_stage,sizeof project_stage,1,f)==1&&
       fwrite(store,sizeof store,1,f)==1&&fwrite(store_len,sizeof store_len,1,f)==1);
 CHECK(!fclose(f));
 printf("PASS actual both-owner AUTO PAN AMOUNT/RATE K1/K2, original clock rebase within single edit, exact physical Hz storage, Undo/Redo and cold recall: %u checks\n",checks);
}
