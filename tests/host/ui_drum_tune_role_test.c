/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static uint16_t actual[240*28];
static void expected_sweep(int raw)
{
 float tau,m,pm;
 CHECK(drum909_param_si(0,0,raw,&tau)==X0X_U_MS);
 CHECK(drum909_param_si(0,4,pref_get(PR(R_ENG,T_909,0,4)),&m)==X0X_U_PCT);m*=.01f;
 (void)drum909_param_si(0,5,pref_get(PR(R_ENG,T_909,0,5)),&pm);
 /* 0.0.26 uses one sample per pixel in a 26x19 cell. At (4,4)
  * in a 32x19 slot, the cell is centred at x=7 with its axis at y=22. */
 cv_rect(7,22,26,1,C_LINE);
 int previous=0;
 for(int x=0;x<26;x++){
  float hz=engine_bd909_hz(tau,m,pm,x*100.f/25.f);
  float level=fm_log2f(hz/40.f)/3.32f;if(level<0)level=0;if(level>1)level=1;
  int y=21-(int)(level*16.f+.5f);
  if(x)cv_line(7+x-1,previous,7+x,y,SOUND_INK);
  previous=y;
 }
}
int main(void)
{
 setup(V_PART,PART_909);ui.sel[PART_909]=0;build_pages();
 pref_t r=PR(R_ENG,T_909,0,0);CHECK(drum_strip_role(r)==DG_BDSWEEP);
 for(int depth=0;depth<3;depth++){
  pref_set(PR(R_ENG,T_909,0,4),depth==0?0:depth==1?64:127);
  for(int v=0;v<128;v++){
   cv_begin(240,28,C_BLACK);dv_glyph(r,v,4,4,1,19.f/18.f,SOUND_INK);memcpy(actual,cv_px,sizeof actual);
   cv_begin(240,28,C_BLACK);expected_sweep(v);
   if(memcmp(actual,cv_px,sizeof actual))fprintf(stderr,"BD_TUNE_RASTER_MISMATCH raw=%d depth=%d\n",v,depth);
   CHECK(!memcmp(actual,cv_px,sizeof actual));
  }
 }
 pref_t pitched=PR(R_ENG,T_909,2,0);CHECK(drum_strip_role(pitched)==DG_TUNE);
 for(int v=0;v<128;v++){
  cv_begin(240,28,C_BLACK);dv_glyph(pitched,v,4,4,1,1,SOUND_INK);memcpy(actual,cv_px,sizeof actual);
  cv_begin(240,28,C_BLACK);dv_tune_fork(4,4,1,1,v/127.f,SOUND_INK);CHECK(!memcmp(actual,cv_px,sizeof actual));
 }
 pref_set(r,64);pref_set(PR(R_ENG,T_909,0,4),80);pref_set(PR(R_ENG,T_909,0,5),64);
 for(int settle=0;settle<52;settle++){int32_t audio[512];engine_render(audio,256);}
 now_ms |= 1u;
 CHECK(!SEQ_PART_OFF(&seq,PART_909));engine_drum(PART_909,0,1);
 for(int step=0;step<3;step++){
  int32_t audio[512];engine_render(audio,256);now_ms+=6;float ms,hz;CHECK(engine_bd909_pitch_view(&ms,&hz));CHECK(ms>=0&&ms<100);
  float level=fm_log2f(hz/40.f)/3.32f;if(level<0)level=0;if(level>1)level=1;
  uint32_t age;float env;CHECK(engine_drum_view(PART_909,DR_BD,&age,&env));
  int x=7+(int)(age*25.f/100.f+.5f),y=21-(int)(16.f*level+.5f);
  cv_begin(240,28,C_BLACK);dv_glyph(r,pref_get(r),4,4,1,19.f/18.f,SOUND_INK);
  if(cv_px[y*240+x]!=swap16(C_WHITE))fprintf(stderr,"BD_TUNE_LIVE_MARKER_MISSING\n");
  CHECK(cv_px[y*240+x]==swap16(C_WHITE));
  if(step==0)pref_set(r,0); /* next-note preview edit must not fabricate sounding position */
 }
 puts("PASS BD Tune time-constant uses native pitch-drop curve, not name-based oscillator fork");return 0;
}
