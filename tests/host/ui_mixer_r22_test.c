/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static uint16_t saved[240*240];
static unsigned region_changed(int x,int y,int w,int h) {
 unsigned n=0;for(int j=y;j<y+h;j++)for(int i=x;i<x+w;i++)n+=saved[j*240+i]!=fb[j*240+i];return n;
}
int main(void) {
 for(int theme=0;theme<6;theme++) {
  setup(V_MIX,PART_909);palette_set(theme);CHECK(cur_page()==0);CHECK(drum_mix_page()<0);CHECK(!master_mix_page());CHECK(!wave_mix_page());
  for(int pos=0;pos<NPARTS;pos++) {
   int p=part_at(pos),x=3+pos*40;
   CHECK(tw(&FONT_T,part_s(p))<=32);
   for(int v=0;v<128;v++) {
    char number[16],unit[8];pref_value_of(PR(R_ENG,T_MIX,p,MX_LEVEL),v,number,unit);
    CHECK(tw(&FONT_S,number)<=32);CHECK(tw(&FONT_T,unit)<=32);
   }
ui.mixsel=(uint8_t)p;build_pages();eng_peak[p]=0;proj.sound.v[T_MIX][p][0]=0;draw_main();memcpy(saved,fb,sizeof saved);
   proj.sound.v[T_MIX][p][0]=127;draw_main();
   CHECK(region_changed(x,MAIN_Y+47,38,12)>0); /* level value, not the fader handle */
   memcpy(saved,fb,sizeof saved);eng_peak[p]=32767;draw_main();CHECK(region_changed(x,MAIN_Y+17,38,26)>0);
   /* Knob zero keeps its production level route, not a new parameter. */
   pref_t route=pg.r[cur_page()][0];CHECK(route.kind==R_ENG && route.a==T_MIX && route.b==p && route.c==MX_LEVEL);
   proj.sound.v[T_MIX][p][MX_LEVEL]=64;int others[NPARTS];for(int j=0;j<NPARTS;j++)others[j]=proj.sound.v[T_MIX][j][MX_LEVEL];
   enc_acc[EN_K1]=1;ui_input_only();CHECK(proj.sound.v[T_MIX][p][MX_LEVEL]>64);
   for(int j=0;j<NPARTS;j++)if(j!=p)CHECK(proj.sound.v[T_MIX][j][MX_LEVEL]==others[j]);
   now_ms+=2000;ui_frame();CHECK(!smc_ui_active());
   char name[48];snprintf(name,sizeof name,"mixer-theme-%d-selected-%d",theme,pos);ui_frame();shot(name);
  }
 }
 setup(V_MIX,PART_909);
 ui.mixsel=PART_909;build_pages();
 pref_t level_ref=PR(R_ENG,T_MIX,PART_909,MX_LEVEL);
 proj.sound.v[T_MIX][PART_909][MX_LEVEL]=64;
 int lane=motion_alloc(proj.arr.lane,seq.ppat[PART_909],PART_909,T_MIX,PART_909,MX_LEVEL);CHECK(lane>=0);
 for(int step=0;step<NSTEPS;step++)proj.arr.lane[lane].val[step]=110;
 seq_start(&seq);run_ms(300);CHECK(motion_now(level_ref)==110);
 draw_main();memcpy(saved,fb,sizeof saved);
 engine_motion_clear(lane);draw_main();
 CHECK(region_changed(3,MAIN_Y+47,38,12)>0);
 CHECK(proj.sound.v[T_MIX][PART_909][MX_LEVEL]==64);
 printf("PASS R22 mixer level readout and live meters: %u checks\n",checks);return 0;
}
