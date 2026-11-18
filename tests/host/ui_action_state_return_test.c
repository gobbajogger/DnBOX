/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static uint16_t before_main[240*240],after_main[240*240];
static uint32_t project_hash(void){uint32_t h=2166136261u;const uint8_t*p=(const uint8_t*)&proj;for(unsigned i=0;i<sizeof proj;i++)h=(h^p[i])*16777619u;return h;}
static void test_generator(void)
{
 setup(V_GEN,PART_303A);ui.gen_stale[0]=0;ui.touched=-1;ui.msg_until=0;draw_main();memcpy(before_main,fb,sizeof fb);
 pref_t density=PR(R_GEN,0,0,G_DENS);int old=pref_get(density);pref_set(density,old?old-1:1);CHECK(ui.gen_stale[0]);
 ui.touched=-1;ui.msg_until=0;draw_main();memcpy(after_main,fb,sizeof fb);
 /* Changing configured density does not alter the written line: only its
  * state/context band must explain the un-applied change. */
 for(int y=HDR_H;y<HDR_H+122;y++)CHECK(!memcmp(before_main+y*240,after_main+y*240,480));
 if(!memcmp(before_main+(HDR_H+122)*240,after_main+(HDR_H+122)*240,16*480))fprintf(stderr,"GEN_CHANGED_STATE_INVISIBLE\n");
 CHECK(memcmp(before_main+(HDR_H+122)*240,after_main+(HDR_H+122)*240,16*480));
 button_tap(B_OCTUP);CHECK(!ui.gen_stale[0]);
 puts("PASS generation changed/applied state visibility");
}
static void expect_caption(const char *s,int center,int y,uint16_t col)
{
 int w=tw(&FONT_B,s),left=center-w/2;CHECK(left>=0&&left+w<=240);
 cv_begin(240,138,col==C_BLACK?C_WHITE:C_BLACK);cv_text(0,0,&FONT_B,s,col);int ink=0;
 for(int dy=0;dy<FONT_B.h;dy++)for(int dx=0;dx<w;dx++)if(cv_px[dy*240+dx]==swap16(col)){
  if(after_main[(y+dy)*240+left+dx]!=cv_px[dy*240+dx]){fprintf(stderr,"MODAL_PHYSICAL_ACTION_CUE_MISSING %s\n",s);CHECK(0);}ink++;
 }CHECK(ink>0);
}
static void test_modal(void)
{
 setup(V_HOME,PART_909);sm_set(cur_pat()->drum[0].hit[0],1);sm_set(cur_pat()->drum[1].hit[0],2);
 uint32_t old=project_hash();ask(ACT_CLEAR_PART,PART_909,"CLEAR 909?","THIS PATTERN");draw_main();memcpy(after_main,fb,sizeof fb);
 int y=HDR_H+BAND_H+vc(&FONT_B,10,24);
 expect_caption(UI_CAP_SEL " YES",68,y,C_BLACK);expect_caption(UI_CAP_HOME " NO",172,y,C_GRAY);
 button_tap(B_HOME);CHECK(project_hash()==old);CHECK(ui.overlay==O_NONE);
 ask(ACT_CLEAR_PART,PART_909,"CLEAR 909?","THIS PATTERN");button_tap(B_SEL);
 CHECK(!sm_get(cur_pat()->drum[0].hit[0],1));CHECK(sm_get(cur_pat()->drum[1].hit[0],2));
 puts("PASS modal physical cues and actual cancel/confirm scope");
}
int main(void){const char *which=getenv("NOLOSS_CASE");if(which&&!strcmp(which,"modal"))test_modal();else test_generator();return 0;}
