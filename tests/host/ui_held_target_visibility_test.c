/* SPDX-License-Identifier: GPL-3.0-only */
/* Reconstructed real-input regression, not a recovered original pixel oracle. */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static uint16_t target_a[240*240],target_b[240*240],repeat_a[240*240];
static void tap_panel(int b){held_btn=1u<<b;++now_ms;input();held_btn=0;++now_ms;input();}
static void exact_context(const char *text,uint16_t ink){
 uint16_t got[HDR_H*40];CHECK(132+tw(&FONT_T,text)<=168);
 for(int y=0;y<HDR_H;y++)for(int x=128;x<168;x++)got[y*40+x-128]=fb[y*240+x];
 cv_begin(240,HDR_H,C_BLACK);night_panel(0,0,240,HDR_H,0);night_panel(0,0,240,HDR_H,0);
 cv_text(132,4,&FONT_T,text,ink);
 for(int y=0;y<HDR_H;y++)for(int x=128;x<168;x++)CHECK(got[y*40+x-128]==cv_px[y*240+x]);
}
static void capture_target(int part,int step,uint16_t *out){
 setup(part==PART_BRK?V_PART:V_SEQ,part);ui.spage=step/16;
 if(part<NKIT){proj.pat[0].drum[part].len=NSTEPS;for(int s=0;s<NSTEPS;s++)sm_set(proj.pat[0].drum[part].hit[0],s);}
 else if(part==PART_BRK)proj.pat[0].brk.steps=0xffff;
 else {bpart_t *bp=&proj.pat[0].bass[part-PART_303A];bp->len=NSTEPS;
  for(int s=0;s<NSTEPS;s++){bp->step[s].flags=G_NOTE;bp->step[s].note=48;bp->step[s].velocity=100;}}
 held_keys=1u<<WHITE_KEY[step%16];++now_ms;input();
 CHECK((part<NKIT?ui.hold_step:part==PART_BRK?ui.brk_held:ui.held_step)==step);
 if(part!=PART_BRK)tap_panel(B_EDIT);
 CHECK(ui.view==V_PART);CHECK(lock_step()==step);CHECK(motion_count(proj.arr.lane)==0);
 ui.frame=0;CHECK(!smc_ui_active());memset(fb,0,sizeof fb);memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);
 draw_header();draw_main();draw_knobs();unsigned visible=0;
 for(unsigned i=0;i<sizeof fb/sizeof fb[0];i++)visible+=fb[i]!=0;CHECK(visible>100);
 memcpy(out,fb,sizeof target_a);char expected[16];snprintf(expected,sizeof expected,"STEP %d",step+1);exact_context(expected,C_AMB);
 /* This case is a hold/edit gesture, not the short tap which intentionally
  * deletes an existing step and changes the saved-state indicator. */
 int saved_before=save_state();static pattern_t pattern_before;pattern_before=proj.pat[0];
 now_ms+=STEP_TAP_MS+1;
 held_keys=0;++now_ms;input();CHECK(lock_step()<0);
 CHECK(save_state()==saved_before);CHECK(!memcmp(&pattern_before,&proj.pat[0],sizeof pattern_before));draw_header();
 snprintf(expected,sizeof expected,"P%d",seq.ppat[part]+1);exact_context(expected,C_WHITE);
 for(int y=0;y<HDR_H;y++)for(int x=168;x<240;x++){
  if(fb[y*240+x]!=out[y*240+x])fprintf(stderr,"status difference part=%d step=%d x=%d y=%d before=%04x after=%04x save=%d\n",part,step,x,y,out[y*240+x],fb[y*240+x],save_state());
  CHECK(fb[y*240+x]==out[y*240+x]);
 }
}
int main(void){
 unsigned failures=0;const int targets[][2]={{2,10},{2,34},{2,63}};
 for(int part=PART_909;part<=PART_BRK;part++)for(unsigned pair=0;pair<sizeof targets/sizeof targets[0];pair++){
  if(part==PART_BRK&&targets[pair][1]>=16)continue;
  capture_target(part,targets[pair][0],target_a);capture_target(part,targets[pair][0],repeat_a);CHECK(!memcmp(target_a,repeat_a,sizeof target_a));
  capture_target(part,targets[pair][1],target_b);
  if(!memcmp(target_a,target_b,sizeof target_a)){fprintf(stderr,"FAIL held-target part=%d steps=%d/%d\n",part,targets[pair][0]+1,targets[pair][1]+1);failures++;}
 }
 if(failures)return 1;puts("held-target exact context/release/status PASS");return 0;
}
