/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static uint16_t actual[240*240];
static void cue(void)
{
 const char *s="EMPTY   WHITE KEY: ADD BAR";
 const felucca_font_t *f=&FONT_XS;
 int x=50,y=HDR_H+vc(f,11,11),w=tw(f,s),ink=0;
 CHECK(x+w<=240);
 memcpy(actual,fb,sizeof actual);
 cv_begin(240,138,C_BLACK);cv_text(0,0,f,s,C_WHITE);
 for(int dy=0;dy<f->h;dy++)for(int dx=0;dx<w;dx++)
  if(cv_px[dy*240+dx]==swap16(C_WHITE)){
   if(actual[(y+dy)*240+x+dx]!=swap16(C_WHITE)){
    fprintf(stderr,"EMPTY_SONG_ACTION_CUE_MISSING\n");CHECK(0);
   }ink++;
  }
 CHECK(ink);
 /* Actual selected-row surface must contrast with its text in either theme,
  * not merely contain the expected dark-on-dark pixels. */
 unsigned bg=swap16(actual[(HDR_H+16)*240+225]),fg=C_WHITE;
 int br=((bg>>11)&31)*255/31,bg8=((bg>>5)&63)*255/63,bb=(bg&31)*255/31;
 int fr=((fg>>11)&31)*255/31,fg8=((fg>>5)&63)*255/63,fb8=(fg&31)*255/31;
 int delta=abs((br*3+bg8*6+bb)-(fr*3+fg8*6+fb8))/10;
 if(delta<90)fprintf(stderr,"SONG_SELECTED_CONTRAST_MISSING delta=%d\n",delta);
 CHECK(delta>=90);
}
int main(void)
{
 for(int theme=0;theme<2;theme++){
  setup(V_SONG,PART_909);pref_set(PR(R_PALETTE,0,0,0),theme?NPALETTES-1:0);ui.song_sel=0;song()->len=0;
  draw_header();draw_main();draw_knobs();cue();shot(theme?"song-empty-light":"song-empty-dark");
  int key=-1;for(int k=0;k<27;k++)if(KEY_WHITE[k]==3){key=k;break;}
  CHECK(key>=0);held_keys=1u<<key;input();held_keys=0;input();
  CHECK(song()->len==1);CHECK(ui.song_sel==1);
  for(int p=0;p<NPARTS;p++)CHECK(song()->bar[0].pat[p]==3);
 }
 puts("PASS empty SONG visible action and real white-key five-part assignment");
 return 0;
}
