/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
int main(void) {
 setup(V_FX,PART_909);
 for(int phase=0;phase<16;phase++)for(int val=16;val<=127;val+=111) {
  tape_glyph_phase=(uint8_t)phase;
  cv_begin(240,80,SOUND_FACE);
  CHECK(control_glyph(PR(R_TAPE,0,0,0),val,120,C_HI));
  for(int side=-1;side<=1;side+=2) {
   int cx=120+side*10,cy=37;
   /* A circular radius-7 reel cannot occupy its rounded-square corners. */
   for(int y=-7;y<=7;y++)for(int x=-7;x<=7;x++)
    if(x*x+y*y>=60)CHECK(cv_px[(cy+y)*240+cx+x]==swap16(SOUND_FACE));
   CHECK(cv_px[cy*240+cx+7]!=swap16(SOUND_FACE));
   CHECK(cv_px[(cy-7)*240+cx]!=swap16(SOUND_FACE));
   CHECK(cv_px[cy*240+cx]!=swap16(SOUND_FACE));
  }
 }
 printf("tape-reels: phases=16 wear-endpoints=2 circular-support PASS checks=%u\n",checks);
 return 0;
}
