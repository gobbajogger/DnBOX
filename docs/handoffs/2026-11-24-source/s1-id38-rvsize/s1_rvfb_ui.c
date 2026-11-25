#define MIXED_ONLY 1
#include "s1_pb_mod_ui.c"
#include "dsp/s1c/s1u.h"
#define RVFB 38
extern int s1_typed_rvfb_choose(unsigned,unsigned);
static unsigned calls[2];
static float feedback[2],greatest_mix[2];
static unsigned observed_mix[2];
void host_reverb_lfo_tap(const s1c *e,float ungated,float actual)
{
 (void)ungated;
 for(unsigned i=0;i<2;i++)if(parts[i] && e==&parts[i]->e) {
  if(actual>greatest_mix[i]) greatest_mix[i]=actual;
  observed_mix[i]++;
 }
}
void __real_s1u_rvb_run(s1u_rvb *,float,float,float,float *,float *,float);
void __wrap_s1u_rvb_run(s1u_rvb *r,float fb,float il,float ir,float *ol,float *or_,float sr)
{
 for(unsigned i=0;i<2;i++)if(parts[i] && r==&parts[i]->e.rv){calls[i]++;feedback[i]=fb;}
 __real_s1u_rvb_run(r,fb,il,ir,ol,or_,sr);
}
static pref_t room(unsigned owner)
{
 ui.overlay=O_NONE;ui.part=PART_303A+owner;set_view(V_PART);build_pages();
 visit("FX SWITCHES");smc_peek_cancel();smc_page_set(cur_page());
 CHECK(s1_patch_at(pg.r[cur_page()][0].c)->factory_id==41);
 CHECK(s1_patch_at(pg.r[cur_page()][1].c)->factory_id==37);
 CHECK(s1_patch_at(pg.r[cur_page()][2].c)->factory_id==51);
 return pg.r[cur_page()][3];
}
static void walk(unsigned owner,unsigned position)
{
 pref_t r=room(owner);CHECK(r.kind==R_S1RVFB&&r.b==owner&&!pref_read_only(r));
 for(unsigned i=0;i<129&&pref_get(r)!=(int)position;i++)detent(3,pref_get(r)<(int)position?1:-1);
 CHECK(pref_get(r)==(int)position&&parts[owner]->e.t[RVFB]==(float)position*(1.f/128.f));
 CHECK(s1_rvfb_pos(read137(&proj.sound,owner)[S1_RVFB_BYTE])==position);
}
static void capture(unsigned owner, unsigned position,int wet,int32_t *out)
{
 stabilize();
 walk(owner,position);
 stabilize();
 for(unsigned i=0;i<2;i++) {
    s1c_front_set(&parts[i]->e,40,.75f);
    s1c_front_set(&parts[i]->e,37,wet?1.f:0.f);
 }
 /* Warm real gate smoothing with no note/energy. Then drive the exact two
  * product part processors directly, excluding unrelated mixer state. */
 render(640);
 CHECK(s1_part_note(parts[owner],57,101));CHECK(s1_part_note(parts[1-owner],65,97));
 memset(calls,0,sizeof calls);
 memset(observed_mix,0,sizeof observed_mix);
 memset(greatest_mix,0,sizeof greatest_mix);
 static float l[256], r[256];
 for(unsigned i=0;i<FRAMES;i+=256)
   for(unsigned v=0;v<2;v++){
    s1_part_render(parts[v],l,r,256);
    for(unsigned k=0;k<256;k++) {
      float x=(l[k]+r[k])*1048576.f;
      CHECK(x>=-1.e9f&&x<=1.e9f);
      out[(i+k)*2+v]=(int32_t)x;
    }
   }
 CHECK(calls[0]==FRAMES/S1C_RVB_DEC && calls[1]==FRAMES/S1C_RVB_DEC &&
       feedback[owner]==(float)position*(1.f/128.f));
 fprintf(stderr,"gate=%d owner%u tap samples %u/%u max %.5g/%.5g\n",wet,owner,observed_mix[0],observed_mix[1],greatest_mix[0],greatest_mix[1]);
 CHECK(observed_mix[0]==FRAMES&&observed_mix[1]==FRAMES);
 CHECK(wet ? (greatest_mix[0]>.1f && greatest_mix[1]>.1f) : (greatest_mix[0]<1.e-8f && greatest_mix[1]<1.e-8f));
 CHECK(s1_part_note(parts[owner],57,0));CHECK(s1_part_note(parts[1-owner],65,0));
}
int main(void)
{
 setup(V_PART,PART_303A);proj.set.bt|=PROJ_BT_CLK_IN;select_patch(0);
 CHECK(room(0).kind==R_NONE&&!s1_typed_rvfb_choose(0,128));
 fresh();
 for(unsigned i=0;i<2;i++){current=NULL;select_patch(i);CHECK(current);parts[i]=current;}
 stabilize();undo_start();CHECK(parts[0]->e.t[RVFB]==.5f&&parts[1]->e.t[RVFB]==.5f);
 for(unsigned i=0;i<=128;i++)CHECK(s1_rvfb_pos(s1_rvfb_code(i))==i);
 unsigned writes=store_writes;
 walk(0,128);walk(1,0);CHECK(!s1_typed_rvfb_choose(0,129));
 CHECK(store_writes==writes);
 CHECK(undo_commit_stable());physical_history(0);CHECK(parts[0]->e.t[RVFB]==.5f&&parts[1]->e.t[RVFB]==.5f);
 physical_history(1);CHECK(parts[0]->e.t[RVFB]==1.f&&parts[1]->e.t[RVFB]==0.f);
 project_capture_stage();slot137(&project_stage.sound,0)[S1_RVFB_BYTE]=129;CHECK(project_validate_stage()<0);
 witness=proj;CHECK(!project_save()&&!project_load_stage());project_commit_stage();stabilize();undo_start();
 CHECK(!memcmp(&witness.sound,&proj.sound,sizeof proj.sound)&&parts[0]->e.t[RVFB]==1.f&&parts[1]->e.t[RVFB]==0.f);
 project_capture_stage();uint8_t h[32];project_file_header(h);CHECK(!project_file_validate(h));
 FILE *f=fopen("out/media.bin","wb");CHECK(f);
 CHECK(fwrite(h,sizeof h,1,f)==1&&fwrite(&project_stage,sizeof project_stage,1,f)==1&&
       fwrite(store,sizeof store,1,f)==1&&fwrite(store_len,sizeof store_len,1,f)==1);
 CHECK(!fclose(f));
 for(unsigned v=0;v<2;v++){
   float peer=parts[1-v]->e.t[RVFB];
   capture(v,0,0,reference);capture(v,128,0,changed);
   /* Real wet gate audibility is checked directly at its applied multiplier. */
   capture(v,0,1,reference);capture(v,128,1,changed);
   CHECK(memcmp(reference,changed,sizeof reference)); /* real rendered receiver, not only target */
   CHECK(feedback[1-v]==peer);
   CHECK(parts[1-v]->e.t[RVFB]==peer);
 }
 printf("PASS scoped RV SIZE K4 ID38, physical Undo/Redo, packed recall, both real reverb receivers/cadence and settled wet gate: %u checks\n",checks);
}
