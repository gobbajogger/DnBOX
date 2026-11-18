/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static uint16_t rendered[240*240], body[240*220];
static project_t saved;
static void expect_mode(int part,int enabled)
{
 const char *label=part==PART_303A?"CHICAGO A FREE PLAY":"CHICAGO B FREE PLAY";
 int w=tw(&FONT_T,label),ink=0,match=0;CHECK(w<=121);
 memcpy(rendered,fb,sizeof rendered);
 cv_begin(240,HDR_H,C_BLACK);cv_text(4,4,&FONT_T,label,part_col());
 for(int y=4;y<4+FONT_T.h;y++)for(int x=4;x<4+w;x++)if(cv_px[y*240+x]==swap16(part_col())){
  ink++;match+=rendered[y*240+x]==cv_px[y*240+x];
 }
 CHECK(ink>0);
 if(enabled&&match!=ink)fprintf(stderr,"FREE_PLAY_MODE_CUE_MISSING part=%d label=%s actual=%s ink=%d match=%d msg=%s until=%u now=%u\n",part,label,bass_header_name(),ink,match,ui.msg,ui.msg_until,plat_ms());
 if(!enabled&&match==ink)fprintf(stderr,"FREE_PLAY_MODE_CUE_UNEXPECTED\n");
 CHECK(enabled?match==ink:match!=ink);
}
int main(void)
{
 for(int part=PART_303A;part<=PART_303B;part++)for(int theme=0;theme<2;theme++)for(int playing=0;playing<2;playing++){
  setup(V_SEQ,part);
  CHECK(!engine_bass_select_preset(bidx(),ORIGINAL_SOUND_CHICAGO,0,&proj.sound));
  for(int n=0;n<1024&&engine_bass_preset_pending(bidx());n++){int32_t audio[512];engine_render(audio,256);}
  CHECK(!engine_bass_preset_pending(bidx()));CHECK(engine_bass_bank(bidx())==ORIGINAL_SOUND_CHICAGO);build_pages();
  pref_set(PR(R_PALETTE,0,0,0),theme?NPALETTES-1:0);
  ui.msg_until=0;ui.msg[0]=0;seq.playing=playing;ui.rec=0;ui.freeplay[bidx()]=0;ui.held_step=-1;build_pages();
  memcpy(&saved,&proj,sizeof saved);draw_header();draw_main();draw_knobs();draw_footer();memcpy(body,fb+240*20,sizeof body);expect_mode(part,0);
  ui.freeplay[bidx()]=1;CHECK(kbd_on());draw_header();draw_main();draw_knobs();draw_footer();
  expect_mode(part,1);CHECK(!memcmp(body,fb+240*20,sizeof body));CHECK(!memcmp(&saved,&proj,sizeof saved));
  ui.rec=1;draw_header();expect_mode(part,0);CHECK(kbd_on());ui.rec=0;draw_header();expect_mode(part,1);
  shot(part==PART_303A?(theme?"freeplay-a-light":"freeplay-a-dark"):(theme?"freeplay-b-light":"freeplay-b-dark"));
 }
 puts("PASS FREE PLAY mode visible without replacing roll, scale, seed, controls or step data");return 0;
}
