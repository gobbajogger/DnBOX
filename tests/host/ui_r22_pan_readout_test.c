/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN unused_r22_pan_readout_smc_main
#include "smc_ui_test.c"
int main(void){
 const int raw[]={0,64,127};const char *want[]={"L50","C","R50"};
 for(int c=0;c<3;c++){
  setup(V_MIX,PART_909);pref_set(PR(R_ENG,T_MIX,0,MX_PAN),raw[c]);ui.touched=-1;ui.msg_until=0;
  draw_main();draw_knobs();draw_header();uint16_t actual[240*240];memcpy(actual,fb,sizeof actual);
  cv_begin(240,69,C_BLACK);text_c(91,0,&FONT_S,want[c],C_WHITE);unsigned ink=0;
  for(int y=0;y<FONT_S.h;y++)for(int x=63;x<120;x++)if(cv_px[y*240+x]==swap16(C_WHITE)){
   ink++;CHECK(actual[(218+y)*240+x]==swap16(SOUND_WHITE));
  }CHECK(ink>0);
 }
 puts("Actual R22 shared control readout retains L50/C/R50, notpercent PASS");return 0;
}
