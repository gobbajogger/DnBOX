/* Parameter-page footer: one continuous lane, real current-instrument state. */
#define SMC_UI_MAIN unused_smc_ui_main
#include "smc_ui_test.c"
static void check_lane(int voice,int length,int playhead){
 dpart_t *dp=&cur_pat()->drum[ui.part];
 dp->len=(uint8_t)length;
 memset(dp->hit,0,sizeof dp->hit);
 /* Other instrument deliberately differs; it must not leak into this footer. */
 for(int s=0;s<NSTEPS;s++){
  if(s&1)sm_set(dp->hit[voice],s);
  else sm_set(dp->hit[(voice+1)%NDRUM],s);
 }
 seq.playing=playhead>=0;
 eng_step[TRK_DRUM+ui.part]=(uint8_t)(playhead>=0?playhead:0);
 dpart_t saved=*dp;int selected=ui.sel[ui.part];
 draw_main();draw_knobs();
 CHECK(!memcmp(&saved,dp,sizeof saved));CHECK(ui.sel[ui.part]==selected);
 cv_begin(240,16,SOUND_BASE);cv_rect(3,0,234,16,SOUND_FACE);
 for(int s=0;s<16;s++){
  int absolute=ui.spage*16+s;
  uint16_t expected=(absolute<length && (absolute&1))?part_col():SOUND_LINE;
  if(playhead>=0 && absolute==playhead%length)expected=C_WHITE;
  cv_rect(9+s*14,0,11,4,expected);
  char number[4];put_i(number,absolute+1);text_c(14+s*14,6,&FONT_T,number,SOUND_GRAY);
 }
 /* Whole footer, including gaps and sixteen distinct labels: no sparse-pixel pass. */
 if(memcmp(fb+212*240,cv_px,240*16*sizeof(uint16_t))){
  fprintf(stderr,"lane mismatch part=%d voice=%d page=%d window=%d length=%d playhead=%d\n",ui.part,voice,cur_page(),ui.spage,length,playhead);
  for(int i=0;i<240*16;i++)if(fb[212*240+i]!=cv_px[i]){fprintf(stderr,"first pixel x=%d y=%d got=%04x want=%04x\n",i%240,212+i/240,fb[212*240+i],cv_px[i]);break;}
 }
 CHECK(!memcmp(fb+212*240,cv_px,240*16*sizeof(uint16_t)));

}
static void check_no_wrapped_hit_lane(void){
 static uint16_t empty[240*240];
 dpart_t *dp=&cur_pat()->drum[ui.part];int voice=ui.sel[ui.part];
 seq.playing=0;dp->len=NSTEPS;memset(dp->hit[voice],0,sizeof dp->hit[voice]);
 draw_main();draw_knobs();memcpy(empty,fb,sizeof empty);
 for(int s=0;s<NSTEPS;s++)sm_set(dp->hit[voice],s);
 draw_main();draw_knobs();unsigned changed=0;
 for(int y=20;y<240;y++)for(int x=0;x<240;x++)if(empty[y*240+x]!=fb[y*240+x]){
  changed++;CHECK(y>=212 && y<=215);CHECK(x>=9 && x<=229);
 }
 CHECK(changed==16*11*4); /* every hit changes once; no duplicate wrapped lane */
}
static void check_input_window(void){
 dpart_t *dp=&cur_pat()->drum[ui.part];
 dpart_t other=cur_pat()->drum[ui.part==PART_909?PART_808:PART_909];
 int voice=ui.sel[ui.part];seq.playing=0;ui.btn=0;dp->len=NSTEPS;
 for(int window=0;window<NSTEPS/16;window++){
  ui.spage=(uint8_t)window;memset(dp->hit,0,sizeof dp->hit);
  int absolute=window*16+3;
  drum_step(ui.part,3,1);now_ms+=10;drum_step(ui.part,3,0);
  CHECK(sm_get(dp->hit[voice],absolute));
  CHECK(!sm_any(dp->hit[(voice+1)%NDRUM]));
  draw_main();draw_knobs();CHECK(fb[213*240+11+3*14]==swap16(part_col()));
  drum_step(ui.part,3,1);now_ms+=10;drum_step(ui.part,3,0);
  CHECK(!sm_get(dp->hit[voice],absolute));
  draw_main();draw_knobs();CHECK(fb[213*240+11+3*14]==swap16(SOUND_LINE));
 }
 CHECK(!memcmp(&other,&cur_pat()->drum[ui.part==PART_909?PART_808:PART_909],sizeof other));
}
int main(void){
 for(int part=PART_909;part<=PART_808;part++){
  setup(V_PART,part);
  for(int voice=0;voice<NDRUM;voice++){
   ui.sel[part]=(uint8_t)voice;build_pages();
   for(int page=0;page<pg.n;page++){
    *page_of(V_PART)=page;
    for(int window=0;window<NSTEPS/16;window++){
     ui.spage=(uint8_t)window;check_lane(voice,NSTEPS,-1);
     check_no_wrapped_hit_lane();
    }
   }
  }
  ui.sel[part]=0;build_pages();*page_of(V_PART)=0;
  for(int window=0;window<NSTEPS/16;window++){
   ui.spage=(uint8_t)window;
   for(int n=1;n<=NSTEPS;n*=2)for(int step=0;step<NSTEPS;step++)check_lane(0,n,step);
   CHECK(ui.spage==window);
  }
  check_input_window();
  ui.spage=1;check_lane(0,32,17); /* visible step18 follows second input window */
  draw_header();draw_main();draw_knobs();draw_footer();
  shot(part==PART_909?"909-single-step-lane":"808-single-step-lane");
 }
 printf("Drum single lane/current instrument/hit/playhead: %u checks PASS\n",checks);
}
