#define MIXED_ONLY 1
#include "s1_pb_mod_ui.c"
static pref_t feedback_page(unsigned owner){
 ui.overlay=O_NONE;ui.part=PART_303A+owner;set_view(V_PART);build_pages();visit("DELAY");
 smc_peek_cancel();smc_page_set(cur_page());
 CHECK(pg.r[cur_page()][0].kind==R_S1DLFB&&pg.r[cur_page()][0].b==owner);
 for(unsigned i=1;i<4;i++)CHECK(pg.r[cur_page()][i].kind==R_NONE);
 return pg.r[cur_page()][0];
}
static void wet(s1_dry_part *p){
 s1c_set(&p->e,S1C_P_DLT,.020f);s1c_set_delay_gate(&p->e,1.f,1);
 s1c_set(&p->e,S1C_P_DLMIX,1.f);
}
int main(void){
 setup(V_PART,PART_303A);proj.set.bt|=PROJ_BT_CLK_IN;select_patch(0);stabilize();set_view(V_PART);build_pages();
 CHECK(page_named("DELAY")<0&&!current->e.delay_feedback_direct);fresh();
 for(unsigned i=0;i<NBASS;i++){current=NULL;select_patch(i);CHECK(current);parts[i]=current;}
 stabilize();undo_start();
 for(unsigned o=0;o<2;o++){
  CHECK(parts[o]->e.delay_feedback_direct&&parts[o]->e.t[42]==.1f);
  wet(parts[o]);CHECK(s1_part_note(parts[o],57+o,100));render(12);
 }
 unsigned writes=store_writes;
 for(unsigned o=0;o<2;o++){
  pref_t r=feedback_page(o);CHECK(pref_get(r)==14);
  char v[16],u[8],n[16],m[8];pref_value(r,v,u);
  CHECK((!strcmp(v,"10.0")||!strcmp(v,"10"))&&!strcmp(u,"%"));
  s1c *e=&parts[o]->e;int dec=e->ddec,len=e->dl[0].len;
  void *raw=e->dl[0].buf;
  detent(0,1);
  pref_value(r,n,m);CHECK(!strcmp(n,"10.6")&&!strcmp(m,"%"));
  CHECK(e->t[42]==.9f*15.f/127.f&&e->delay_feedback_direct&&read137(&proj.sound,o)[S1_FB_BYTE]==16);
  CHECK(e->ddec==dec&&e->dl[0].len==len&&e->dl[0].buf==raw&&e->t[41]==1.f);
 }
 CHECK(store_writes==writes&&undo_commit_stable());
 physical_history(0);CHECK(parts[0]->e.t[42]==.1f&&parts[1]->e.t[42]==.1f&&parts[0]->e.delay_feedback_direct);
 physical_history(1);CHECK(parts[0]->e.t[42]==.9f*15.f/127.f&&parts[1]->e.t[42]==.9f*15.f/127.f);
 CHECK(s1_part_note(parts[0],57,0)&&s1_part_note(parts[1],58,0));
 feedback_page(1);for(unsigned i=0;i<112;i++)detent(0,1);
 CHECK(parts[1]->e.t[42]==.9f&&parts[0]->e.t[42]<.11f);
 project_capture_stage();slot137(&project_stage.sound,0)[S1_FB_BYTE]=129;CHECK(project_validate_stage()<0);
 witness=proj;CHECK(!project_save()&&!project_load_stage());project_commit_stage();stabilize();
 CHECK(!memcmp(&witness.sound,&proj.sound,sizeof proj.sound)&&parts[0]->e.t[42]==.9f*15.f/127.f&&parts[1]->e.t[42]==.9f);
 project_capture_stage();uint8_t h[32];project_file_header(h);CHECK(!project_file_validate(h));
 FILE *f=fopen("out/media.bin","wb");CHECK(f);
 CHECK(fwrite(h,sizeof h,1,f)==1&&fwrite(&project_stage,sizeof project_stage,1,f)==1&&
       fwrite(store,sizeof store,1,f)==1&&fwrite(store_len,sizeof store_len,1,f)==1);
 CHECK(!fclose(f));
 printf("PASS scoped S1 DELAY FEEDBACK K1 from fresh direct gain, increasing first readout, actual tail ring/gate retained, Undo/Redo and exact recall: %u checks\n",checks);
}
