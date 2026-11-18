/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static uint16_t snapshot[240*240],banks[204*240];
static void render(void){draw_header();draw_main();draw_knobs();}
static void expect_number(int col,int step,int enabled){
 char text[8];put_i(text,step+1);int w=tw(&FONT_T,text),x=14+col*14-w/2,ink=0;
 memcpy(snapshot,fb,sizeof snapshot);cv_begin(240,KNOB_H,SOUND_BASE);
 cv_text(0,0,&FONT_T,text,enabled?SOUND_GRAY:SOUND_DIM);
 for(int y=0;y<FONT_T.h;y++)for(int dx=0;dx<w;dx++)
  if(cv_px[y*240+dx]!=swap16(SOUND_BASE)){
   if(snapshot[(224+y)*240+x+dx]!=cv_px[y*240+dx])fprintf(stderr,"SYNTH_STEP_NUMBER_MISSING step=%d\n",step+1);
   CHECK(snapshot[(224+y)*240+x+dx]==cv_px[y*240+dx]);ink++;
  }
 CHECK(ink);
}
int main(void){
 static const int kinds[]={ORIGINAL_SOUND_CHICAGO,ORIGINAL_SOUND_MIAMI,ORIGINAL_SOUND_JUNO,ORIGINAL_SOUND_SYNTHONE};
 static const int lengths[]={1,8,17,31,64};
 for(unsigned k=0;k<4;k++)for(int b=0;b<2;b++){
  setup(V_PART,PART_303A+b);CHECK(engine_bass_select_preset(b,kinds[k],0,&proj.sound)==0);
  int32_t audio[512];
  for(int n=0;n<1024&&engine_bass_preset_pending(b);n++)engine_render(audio,256);
  CHECK(!engine_bass_preset_pending(b));CHECK(engine_bass_bank(b)==kinds[k]);
  CHECK(engine_bass_sound_bank(&proj.sound,b)==kinds[k]);build_pages();
  for(int theme=0;theme<2;theme++){
   pref_set(PR(R_PALETTE,0,0,0),theme?NPALETTES-1:0);
   for(unsigned n=0;n<5;n++)for(int window=0;window<4;window++){
    bpart_t *bp=&cur_pat()->bass[b];bp->len=(uint8_t)lengths[n];ui.spage=window;
    for(int s=0;s<NSTEPS;s++)bp->step[s].flags=s%3==0?G_NOTE:s%3==1?G_TIE:G_REST;
    seq.playing=0;ui.held_step=-1;render();
    for(int c=0;c<16;c++)expect_number(c,window*16+c,window*16+c<bp->len);
    render();memcpy(banks,fb+20*240,sizeof banks);
    for(int c=0;c<16;c++){
     int step=window*16+c,x=8+c*14,enabled=step<bp->len;
     uint16_t expected=enabled?(step%3==2?SOUND_LINE:part_col()):SOUND_SIDE;
     CHECK(fb[237*240+x]==swap16(expected));
     CHECK(fb[238*240+x]==swap16(enabled&&step%3==0?part_col():enabled?SOUND_LINE:SOUND_SIDE));
     if(enabled){
      seq.playing=1;eng_step[TRK_BASS0+b]=step;ui.held_step=-1;render();
      CHECK(!memcmp(banks,fb+20*240,sizeof banks));
      ui.held_step=step;render();
      CHECK(fb[236*240+x]==swap16(SOUND_INK));CHECK(fb[239*240+x]==swap16(SOUND_INK));
      /* Held state can legitimately alter resolved bank values; ordinary
       * playback above and stopped restoration both retain these banks. */
      ui.held_step=-1;seq.playing=0;render();
      CHECK(!memcmp(banks,fb+20*240,sizeof banks));
     }
    }
    if(n==2&&window==1){char name[80];snprintf(name,sizeof name,"synth-step-kind%u-part%d-theme%d",k,b,theme);shot(name);}
   }
  }
 }
 puts("PASS synth parameter absolute16-step lane all current identities/parts/themes/lengths/windows");return 0;
}
