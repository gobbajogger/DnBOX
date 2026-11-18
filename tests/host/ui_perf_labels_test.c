/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
/* Exercise the production PERF band, not a parallel text formatter. */
static void expected_bottom(const char *label, uint16_t color) {
 uint16_t actual[240*14];
 memcpy(actual,cv_px+240*46,sizeof actual);
 cv_begin(240,80,C_BLACK);
 if(label)cv_text(4,46,&FONT_XS,label,color);
 CHECK(!memcmp(actual,cv_px+240*46,sizeof actual));
}
int main(void) {
 setup(V_PERF,PART_909);
 perf.test=-1;perf.stalls_on=0;
 cv_begin(240,80,C_BLACK);draw_perf(0);
 expected_bottom("STALLS OFF",C_DIM);
 perf.stalls_on=1;perf.stall[0]=2;perf.stall[1]=3;perf.stall[2]=4;
 cv_begin(240,80,C_BLACK);draw_perf(0);
 expected_bottom("STALLS  FETCH 2%  READ 3%  WRITE 4%",C_GRAY);
 for(int phase=0;phase<=4;phase++) {
  perf.test=0;perf.phase=phase;
  cv_begin(240,80,C_BLACK);draw_perf(0);
  expected_bottom(NULL,C_DIM);
 }
 puts("perf-labels: inactive/active stalls and 5 test phases PASS");
 return 0;
}
