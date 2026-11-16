/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN unused_r22_material_smc_main
#include "smc_ui_test.c"
int main(void){
 setup(V_HOME,PART_909);
 uint16_t actual[240*69];
 for(int active=0;active<2;active++){
  cv_begin(240,69,SOUND_BASE);surface_panel(3,2,234,63,active,RGB(0,255,0),C_WHITE,C_BLACK);memcpy(actual,cv_px,sizeof actual);
  cv_begin(240,69,SOUND_BASE);rbox(3,2,234,63,9,active?SOUND_SELECT:SOUND_FACE);rframe(3,2,234,63,9,active?SOUND_SELECT_EDGE:SOUND_EDGE);
  CHECK(!memcmp(actual,cv_px,sizeof actual));
 }
 puts("Shared R22 flatpanelradius9 neutralmaterial exactmask PASS");return 0;
}
