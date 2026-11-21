/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static uint16_t expected[240*82];
int main(void) {
 setup(V_PART,PART_909);pref_set(PR(R_ENG,T_909,0,4),64);
 pref_t refs[]={PR(R_ENG,T_909,0,0),PR(R_ENG,T_909,0,4),PR(R_ENG,T_909,0,5),PR(R_ENG,T_909,0,1),PR(R_ENG,T_808,0,1)};
 for(unsigned i=0;i<sizeof refs/sizeof refs[0];i++)for(int value=0;value<128;value+=63) {
  cv_begin(240,82,C_BLACK);dv_glyph(refs[i],value,13,13,23.f/32.f,16.f/18.f,C_HI);memcpy(expected,cv_px,sizeof expected);
  cv_begin(240,82,C_BLACK);drum_strip_glyph(refs[i],value,12,12,C_HI,drum_strip_role(refs[i]));
  CHECK(!memcmp(expected,cv_px,sizeof expected));
 }
 printf("DRUM shared part/strip semantics PASS %u checks\n",checks);return 0;
}
