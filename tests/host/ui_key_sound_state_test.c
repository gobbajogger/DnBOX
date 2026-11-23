/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static uint16_t rendered[240*240],selection_only[240*240];
static void expect_state(int off)
{
 const char *text="KEY SOUND OFF";int x=91,y=28,w=tw(&FONT_T,text),ink=0,match=0;
 CHECK(x+w<210);memcpy(rendered,fb,sizeof rendered);
 cv_begin(240,138,C_BLACK);cv_text(0,0,&FONT_T,text,SOUND_GRAY);
 for(int dy=0;dy<FONT_T.h;dy++)for(int dx=0;dx<w;dx++)
  if(cv_px[dy*240+dx]==swap16(SOUND_GRAY)){
   ink++;match+=rendered[(y+dy)*240+x+dx]==cv_px[dy*240+dx];
  }
 CHECK(ink>0);
 if(off&&match!=ink)fprintf(stderr,"KEY_SOUND_OFF_STATE_MISSING\n");
 CHECK(off?match==ink:match!=ink);
}
int main(void)
{
 for(int kit=0;kit<2;kit++)for(int voice=0;voice<NDRUM;voice++){
  setup(V_PART,kit);ui.sel[kit]=voice;ui.page[V_PART][kit]=0;build_pages();
  int choices=kit==PART_808&&voice==D8_CP?2:1;
  for(int choice=0;choice<choices;choice++){
   if(choices==2)pref_set(PR(R_ENG,T_808,D8_CP,4),choice);
  for(int theme=0;theme<2;theme++)for(int mode=0;mode<8;mode++){
   pref_set(PR(R_PALETTE,0,0,0),theme?NPALETTES-1:0);seq.playing=mode&1;ui.rec=(mode>>1)&1;proj.set.keysound=(mode>>2)&1;
   int off=seq.playing&&!ui.rec&&!proj.set.keysound;
   CHECK(key_sounds()==!off);draw_header();draw_main();draw_knobs();expect_state(off);
   if(mode==1)memcpy(selection_only,rendered,sizeof rendered);
   if(mode==5)for(int y=HDR_H;y<HDR_H+2*BAND_H;y++)for(int x=0;x<240;x++)
    if(!(x>=91&&x<91+tw(&FONT_T,"KEY SOUND OFF")&&y>=28&&y<28+FONT_T.h))
     CHECK(rendered[y*240+x]==selection_only[y*240+x]);
   if(kit==0&&voice==2&&mode==1)shot(theme?"key-sound-off-light":"key-sound-off-dark");
   if(choices==2&&mode==1){char name[80];snprintf(name,sizeof name,"key-sound-off-%s-%s",choice?"maracas":"clap",theme?"light":"dark");shot(name);}
  }
  }
 }
 puts("PASS all22 drum key-sound state visibility across transport/record/mode/themes");
 puts("PASS 352 established mode combinations plus16 alternate MARACAS combinations; unchanged y28 oracle and redraw mask");
 return 0;
}
