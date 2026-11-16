/* SPDX-License-Identifier: GPL-3.0-only */
/* Constructed held-step display must name the controls physical knobs edit. */
#define SMC_UI_MAIN unused_held_label_smc_main
#include "smc_ui_test.c"
static int caption_matches(const uint16_t *actual,int cell,const char *name){
 int cx=34+cell*57;cv_begin(240,69,SOUND_FACE);text_c(cx,0,&FONT_T,name,SOUND_INK);unsigned ink=0;
 /* Compare the actual anti-aliased raster on the real knownface. Inferring
  * opaque ink from a different-colour composite is invalid when glyph bitmap
  * padding overlaps adjacent letters; earlier failed observer attempts remain. */
 for(int y=0;y<FONT_T.h;y++)for(int x=6+cell*57;x<63+cell*57;x++)if(cv_px[y*240+x]!=swap16(SOUND_FACE)){
  ink++;if(actual[(179+y)*240+x]!=cv_px[y*240+x])return 0;
 }return ink>0;
}
int main(void){
 setup(V_SEQ,PART_303A);ui.held_step=3;ui.touched=-1;ui.msg_until=0;draw_main();draw_knobs();draw_header();shot("held-step-labels");
 uint16_t actual[240*240];memcpy(actual,fb,sizeof actual);const char *names[]={"NOTE","GATE","ACCENT","SLIDE"};
 for(int i=0;i<4;i++)CHECK(caption_matches(actual,i,names[i]));
 /* A real missing-caption mutation must still reject; no relaxed tolerance. */
 for(int y=179;y<191;y++)for(int x=6;x<63;x++)actual[y*240+x]=swap16(SOUND_FACE);
 CHECK(!caption_matches(actual,0,"NOTE"));
 puts("Actual held-step exactAA captions + erased-caption tripwire PASS");return 0;
}
