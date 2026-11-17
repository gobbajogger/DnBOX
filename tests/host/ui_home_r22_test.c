/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static uint16_t textmask[240*69],expected_bg;
static void expected_text(int x,int y,const char *s,uint16_t ink,uint16_t bg) {
 expected_bg=bg;cv_begin(240,69,bg);cv_text(x,y,&FONT_T,s,ink);memcpy(textmask,cv_px,sizeof textmask);
}
static void check_text(void) {
 for(int i=0;i<240*69;i++)if(textmask[i]!=swap16(expected_bg))CHECK(fb[MAIN_Y*240+i]==textmask[i]);
}
int main(void) {
 setup(V_HOME,PART_909);ui.dirty=0;
 expected_text(8,6,"PATTERNS 1-16",SOUND_GRAY,C_BLACK);draw_main();check_text();
 expected_text(232-tw(&FONT_T,"SAVED"),6,"SAVED",SOUND_GRAY,SOUND_FACE);draw_main();check_text();
 CHECK(fb[(MAIN_Y+3)*240+220]==swap16(SOUND_FACE)); /* saved-state pill */
 for(int bank=0;bank<2;bank++) {
  ui.pbank=bank;draw_main();
  for(int pos=0;pos<NPARTS;pos++)for(int c=0;c<16;c++) {
   int part=part_at(pos),current=seq.ppat[part]==bank*16+c;
   CHECK(fb[(MAIN_Y+38+pos*18)*240+43+c*12]==swap16(current?PART_COL[part]:UI_N_SIDE));
  }
 }
 printf("HOME R22 grid/saved PASS %u checks\n",checks);return 0;
}
