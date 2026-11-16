/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN unused_cell_alignment_smc_main
#include "smc_ui_test.c"
int main(void){
 setup(V_PART,PART_808);ui.sel[PART_808]=7;*page_of(V_PART)=1;build_pages();
 ui.msg_until=0;ui.dirty=1;for(int i=0;i<4;i++)blit_hash[i]=0;ui_frame();
 uint16_t actual[240*240];memcpy(actual,fb,sizeof actual);
 /* Both active/inactive two-control rows span two shared57px tracks each.
  * Verify opaque native-font pixels in the actual committed frame. */
 cv_begin(240,138,SOUND_FACE);text_c(63,40,&FONT_T,"TUNE",SOUND_GRAY);
 unsigned ink=0;for(int y=40;y<52;y++)for(int x=42;x<84;x++)
  if(cv_px[y*240+x]!=swap16(SOUND_FACE)){ink++;CHECK(actual[y*240+x]==cv_px[y*240+x]);}
 CHECK(ink>0);puts("Real compact cell caption uses shared even tracks PASS");return 0;
}
