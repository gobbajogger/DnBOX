/* Actual production renderer with experimental family geometry enabled.
 * Captures are implementation WIP, not final canonical glyph admission. */
#define SMC_UI_MAIN inherited_fixture
#include "smc_ui_test.c"
static void break_probe_frame(void){ui.msg_until=0;ui.touched=-1;ui.touch_until=0;draw_header();draw_main();draw_knobs();}
int main(void){
 setup(V_PART,PART_BRK);
 const char *longname="WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW";
 for(int width=106;width<=220;width+=114){
  struct {char value[40];unsigned guard;} bounded={{0},0x12345678};
  break_layout_fit_name(bounded.value,sizeof bounded.value,width==106?"B ":"PREVIEW B ",longname,width);
  CHECK(bounded.guard==0x12345678);CHECK(strlen(bounded.value)<40);
  CHECK(tw(&FONT_T,bounded.value)<=width);CHECK(strstr(bounded.value,"..")!=0);
 }

 const char *const names[4]={"loops","sequence","groove","retrig"};
 for(int theme=0;theme<2;theme++)for(int page=0;page<4;page++){
  setup(page?V_SEQ:V_PART,PART_BRK);proj.set.palette=theme?NPALETTES-1:0;palette_set(proj.set.palette);
  ui.page[ui.view][PART_BRK]=(uint8_t)(page?page-1:0);build_pages();
  brkpart_t *bp=&cur_pat()->brk;bp->steps=0x5555;
  for(int i=0;i<16;i++)bp->slice[i]=(uint8_t)(i%2?0:i%8+1);
  for(int i=0;i<16;i++)audio_block();break_probe_frame();
  CHECK(pg.n==(page?3:1));
  for(int i=0;i<16;i++)for(int y=234;y<238;y++)for(int x=9+i*14;x<20+i*14;x++)
   CHECK(fb[y*240+x]==swap16((bp->steps>>i)&1u?part_col():C_LINE));
  static uint16_t before[26*240];memcpy(before,fb+214*240,sizeof before);
  bp->steps^=1u<<15;draw_knobs();unsigned changed=0;
  for(int i=0;i<26*240;i++)changed+=before[i]!=fb[214*240+i];
  CHECK(changed==44);bp->steps^=1u<<15;break_probe_frame();
  char name[80];snprintf(name,sizeof name,"break-family-%s-%s",names[page],theme?"light":"dark");shot(name);
  if(!page)for(int bank=0;bank<2;bank++){
   brkpart_t original=*bp;CHECK(brk_choice_turn(PR(R_BRKSLOT,bank,0,0),1));audio_block();now_ms+=1500;break_probe_frame();
   CHECK(engine_brk_preview_active());CHECK(!memcmp(bp,&original,sizeof original));
   snprintf(name,sizeof name,"break-family-preview-%c-%s",bank?'B':'A',theme?"light":"dark");shot(name);
   brk_choice_cancel();audio_block();CHECK(!memcmp(bp,&original,sizeof original));break_probe_frame();
  }
 }
 /* Real count changes and every reachable eight-slice waveform window. */
 setup(V_SEQ,PART_BRK);for(int n=0;n<16;n++)audio_block();
 for(int mode=0;mode<4;mode++){
  pref_set(PR(R_BRKSLICE,0,0,0),mode);audio_block();CHECK(engine_brk_count(0)==(4<<mode));
  for(int window=0;window<(((4<<mode)+7)>>3);window++){
   ui.brk_page=window;break_probe_frame();
   brkpart_t before=cur_pat()->brk;
   draw_main();draw_knobs();CHECK(!memcmp(&before,&cur_pat()->brk,sizeof before));
  }
 }
 /* Stopped held-slice release and DSP count adoption must invalidate footer without a project edit. */
 setup(V_SEQ,PART_BRK);for(int n=0;n<16;n++)audio_block();
 pref_set(PR(R_BRKSLICE,0,0,0),1);audio_block();CHECK(engine_brk_count(0)==8);
 cur_pat()->brk.steps=0xffff;cur_pat()->brk.slice[0]=8;
 engine_brk_live(32,1);audio_block();CHECK(engine_brk_held());
 pref_set(PR(R_BRKSLICE,0,0,0),0);CHECK(engine_brk_count(0)==8);
 break_probe_frame();static uint16_t pending_footer[240*26];
 memcpy(pending_footer,fb+214*240,sizeof pending_footer);
 brkpart_t pending_project=cur_pat()->brk;
 engine_brk_live(32,0);audio_block();CHECK(!engine_brk_held());
 engine_brk_flush();CHECK(engine_brk_count(0)==4);
 CHECK(!memcmp(&pending_project,&cur_pat()->brk,sizeof pending_project));
 draw_knobs();unsigned label_changes=0;
 for(int y=214;y<240;y++)for(int x=0;x<240;x++)if(pending_footer[(y-214)*240+x]!=fb[y*240+x]){
  CHECK(y>=224&&y<233&&x>=9&&x<20);label_changes++;
 }
 CHECK(label_changes>0);
 cv_begin(240,KNOB_H,C_BLACK);text_c(14,vc(&FONT_T,224-KNOB_Y,9),&FONT_T,"4",C_HI);
 for(int y=224;y<233;y++)for(int x=9;x<20;x++)CHECK(fb[y*240+x]==cv_px[(y-KNOB_Y)*240+x]);
 /* Actual A/B sequencer ownership, not synthetic active flags. */
 for(int wanted=0;wanted<2;wanted++){
  setup(V_PART,PART_BRK);brkpart_t *bp=&cur_pat()->brk;
  bp->steps=0xffff;bp->set[BRK_BCHANCE]=wanted?100:0;
  bp->set[BRK_COMPLEX]=bp->set[BRK_PHRASE]=0;engine_brk_loops();seq.mute=0;seq.req_start=1;
  int slice=-1,bank=-1,div=0,running=0;
  for(int n=0;n<400;n++){audio_block();engine_brk_state(&slice,&bank,&div,&running);if(running&&bank==wanted&&slice>=0)break;}
  CHECK(running&&bank==wanted&&slice>=0);break_probe_frame();
  for(int x=0;x<106;x++){
   CHECK(fb[111*240+(wanted?124:10)+x]==swap16(part_col()));
   CHECK(fb[111*240+(wanted?10:124)+x]==swap16(SOUND_FACE));
  }
  shot(wanted?"break-family-playing-B":"break-family-playing-A");seq.req_stop=1;audio_block();
 }
 /* The footer keeps actual lock notches distinct from slice and hit rows. */
 setup(V_SEQ,PART_BRK);cur_pat()->brk.steps=0xffff;
 int lane=motion_alloc(proj.arr.lane,0,PART_BRK,T_BRK,0,1);CHECK(lane>=0);
 proj.arr.lane[lane].val[15]=13;break_probe_frame();CHECK(lock_marks(-1,-1,0)&(1u<<15));
 for(int y=238;y<240;y++)for(int x=223;x<226;x++)CHECK(fb[y*240+x]==swap16(C_AMB));
 shot("break-family-lock-step16");
 puts("BREAK family experimental geometry:15 actual captures, all slice windows, actual A/B and footer state/cache checks; canonical glyph admission pending");return 0;
}
