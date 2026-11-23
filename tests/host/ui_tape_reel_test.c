/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
/* G-FINAL TAPE: two round radius-5 reels at cell (6,7) and (19,7); the knob strip
 * places the 26x19 cell at (cx-13, 29). Spokes turn only with the live tape phase. */
static uint32_t reel_hash(void){uint32_t h=2166136261u;for(int k=0;k<240*80;k++)h=(h^cv_px[k])*16777619u;return h;}
int main(void) {
 setup(V_FX,PART_909);
 uint32_t seen[16];
 for(int phase=0;phase<16;phase++)for(int val=16;val<=127;val+=111) {
  tape_glyph_phase=(uint8_t)phase;tape_glyph_running=1;
  cv_begin(240,80,SOUND_FACE);
  CHECK(control_glyph(PR(R_TAPE,0,0,0),val,120,C_HI));
  if(val==16)seen[phase]=reel_hash();
  for(int side=0;side<2;side++) {
   int cx=107+(side?19:6),cy=36;
   /* A circular reel cannot occupy the corners of its bounding square. */
   for(int y=-6;y<=0;y++)for(int x=-6;x<=6;x++)          /* lower corners hold the tape path */
    if(x*x+y*y>=50)CHECK(cv_px[(cy+y)*240+cx+x]==swap16(SOUND_FACE));
   CHECK(cv_px[cy*240+cx+5]!=swap16(SOUND_FACE));
   CHECK(cv_px[(cy-5)*240+cx]!=swap16(SOUND_FACE));
   CHECK(cv_px[cy*240+cx]!=swap16(SOUND_FACE));
  }
 }
 CHECK(seen[0]!=seen[4]);                         /* live: the spokes move with the tape */
 tape_glyph_running=0;tape_glyph_phase=4;cv_begin(240,80,SOUND_FACE);control_glyph(PR(R_TAPE,0,0,0),16,120,C_HI);
 uint32_t idle4=reel_hash();tape_glyph_phase=0;cv_begin(240,80,SOUND_FACE);control_glyph(PR(R_TAPE,0,0,0),16,120,C_HI);
 CHECK(idle4==reel_hash());                       /* stopped tape: no wall-clock motion */
 printf("tape-reels: phases=16 wear-endpoints=2 round-reels live PASS checks=%u\n",checks);
 return 0;
}
