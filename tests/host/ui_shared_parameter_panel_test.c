/* Reconstructed from the verified pre-rollback family fixture; no pixel baseline. */
#define SMC_UI_MAIN unused_smc_ui_main
#include "smc_ui_test.c"
static void shared_plane(void){
 static uint16_t saved_strip[240*KNOB_H];
 int page=cur_page();pref_t refs[4];int values[4];
 for(int k=0;k<4;k++){refs[k]=pg.r[page][k];values[k]=pref_get(refs[k]);}
 draw_header();draw_main();draw_knobs();
 /* Legitimate sparse-page read-only text may cross a former card seam.
  * Expected ink is explicit; a retained vertical divider must still fail. */
 cv_begin(240,KNOB_H,SOUND_FACE);
 if(ui.part==PART_BRK&&ui.view==V_SEQ&&cur_page()==0){
  char t[20];
  put_i(put_s(t,"A SLICES "),(int)brk_slice_count(&cur_pat()->brk,0));
  cv_text(105,vc(&FONT_T,17,10),&FONT_T,t,C_GRAY);
  put_i(put_s(t,"B SLICES "),(int)brk_slice_count(&cur_pat()->brk,1));
  cv_text(105,vc(&FONT_T,31,10),&FONT_T,t,C_GRAY);
 }
 for(int seam=1;seam<4;seam++)for(int y=24;y<=50;y++)
  CHECK(fb[(KNOB_Y+y)*240+seam*60]==cv_px[y*240+seam*60]);
 for(int k=0;k<4;k++)CHECK(pref_get(refs[k])==values[k]);
 memcpy(saved_strip,fb+KNOB_Y*240,sizeof saved_strip);
 draw_knobs();CHECK(!memcmp(saved_strip,fb+KNOB_Y*240,sizeof saved_strip));
 blit_hash[3]=0;draw_knobs();CHECK(!memcmp(saved_strip,fb+KNOB_Y*240,sizeof saved_strip));
}
static void flat_surface_contract(void){
 static uint16_t actual[240*80];
 for(int active=0;active<2;active++){
  cv_begin(240,80,C_BLACK);
  surface_panel(3,2,234,68,active,SOUND_FACE,SOUND_EDGE,SOUND_SIDE);
  memcpy(actual,cv_px,sizeof actual);
  cv_begin(240,80,C_BLACK);
  rbox(3,2,234,68,8,SOUND_EDGE);rbox(4,3,232,66,7,SOUND_FACE);
  CHECK(!memcmp(actual,cv_px,sizeof actual));
 }
}
static void bounded_selected_surfaces(void){
 const int sizes[][2]={{19,69},{17,12},{236,11},{229,21},{24,18},{1,1},{2,2}};
 static uint16_t unselected[240*80];
 for(unsigned n=0;n<sizeof sizes/sizeof sizes[0];n++){
  int w=sizes[n][0],h=sizes[n][1];
  for(int selected=0;selected<2;selected++){
   cv_begin(240,80,C_BLACK);night_panel(2,3,w,h,selected);
   for(int y=0;y<80;y++)for(int x=0;x<240;x++)
    if(x<2||x>=2+w||y<3||y>=3+h)CHECK(cv_px[y*240+x]==swap16(C_BLACK));
   if(!selected)memcpy(unselected,cv_px,sizeof unselected);
   else CHECK(memcmp(unselected,cv_px,sizeof unselected)!=0);
  }
 }
 cv_begin(240,80,C_BLACK);
 surface_panel(2,3,0,11,1,SOUND_SELECT,SOUND_SELECT_EDGE,SOUND_SIDE);
 surface_panel(2,3,19,0,1,SOUND_SELECT,SOUND_SELECT_EDGE,SOUND_SIDE);
 for(int i=0;i<240*80;i++)CHECK(cv_px[i]==swap16(C_BLACK));
}
static void break_actual_bank_feedback(void){
 for(int wanted=0;wanted<2;wanted++){
  setup(V_PART,PART_BRK);brkpart_t *bp=&cur_pat()->brk;
  bp->steps=0xffff;bp->set[BRK_BCHANCE]=wanted?100:0;
  bp->set[BRK_COMPLEX]=0;bp->set[BRK_PHRASE]=0;
  engine_brk_loops();seq.mute=0;seq.req_start=1;
  int slice=-1,bank=-1,div=0,running=0,observed=0;
  for(int n=0;n<400;n++){
   audio_block();engine_brk_state(&slice,&bank,&div,&running);
   if(running&&bank==wanted&&slice>=0){observed=1;break;}
  }
  CHECK(observed);draw_header();draw_main();draw_knobs();
  int y=111;
  for(int x=0;x<106;x++){
   CHECK(fb[y*240+(wanted?124:10)+x]==swap16(part_col()));
   CHECK(fb[y*240+(wanted?10:124)+x]==swap16(SOUND_FACE));
  }
  shot(wanted?"break-playing-B":"break-playing-A");seq.req_stop=1;audio_block();
 }
 setup(V_PART,PART_BRK);
 /* Boot publishes more than one audio-boundary budget of sound controls. */
 for(int n=0;n<16;n++)audio_block();
 CHECK(!SEQ_PART_OFF(&seq,TRK_BRK));engine_brk_live(32,1);
 for(int n=0;n<16&&!engine_brk_held();n++)audio_block();
 CHECK(engine_brk_held()!=0);
 int slice,bank,div,running;engine_brk_state(&slice,&bank,&div,&running);
 CHECK(bank==0&&slice==0&&!running);draw_main();
 for(int x=10;x<116;x++)CHECK(fb[111*240+x]==swap16(part_col()));
 shot("break-stopped-audition-A");engine_brk_live(32,0);audio_block();CHECK(!engine_brk_held());
 draw_main();
 for(int x=10;x<116;x++)CHECK(fb[111*240+x]==swap16(SOUND_FACE));
}
int main(void){
 flat_surface_contract();bounded_selected_surfaces();break_actual_bank_feedback();
 const int views[]={V_HOME,V_PART,V_FX,V_MIX,V_GLO,V_SONG,V_PERF,V_SEQ};
 for(unsigned v=0;v<sizeof views/sizeof views[0];v++)for(int part=0;part<NPARTS;part++){
  if(views[v]==V_PART&&part!=PART_BRK)continue;
  setup(views[v],part);
  for(int p=0;p<pg.n;p++){*page_of(views[v])=p;
   for(int touch=-1;touch<4;touch++){
    ui.touched=touch;ui.touch_until=now_ms+1000;blit_hash[3]=0;shared_plane();
   }
  }
 }
 setup(V_PART,PART_BRK);
 for(int mode=0;mode<4;mode++){
  pref_set(PR(R_BRKSLICE,0,0,0),mode);audio_block();CHECK(engine_brk_count(0)==(4<<mode));
  for(int page=0;page<(int)(engine_brk_count(0)+7)/8;page++){
   ui.brk_page=page;blit_hash[3]=0;shared_plane();
   CHECK(fb[(MAIN_Y+BAND_H+30)*240+120]==swap16(SOUND_FACE));
  }
 }
 pref_set(PR(R_BRKSLICE,0,0,0),1);ui.brk_page=0;audio_block();
 blit_hash[3]=0;shared_plane();shot("break-shared-panel");
 setup(V_MIX,PART_909);shared_plane();shot("mix-shared-panel");
 printf("Common parameter shared plane: %u checks PASS\n",checks);
}
