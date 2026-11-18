/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static uint16_t pixels[240*28];
int main(void)
{
 setup(V_PART,PART_909);ui.sel[PART_909]=DR_SD;build_pages();
 for(int raw=0;raw<128;raw++) {
  /* Independent native pot law: 300..4000 ms nominal, then 0.7 decay scale.
   * Fixed 24 ms excitation hold is NOT part of the decay value. */
  float expected=.7f*x0x_pot_exp_value(X0X_EXP_SD_TONE,raw),value=0;
  int unit=drum909_param_si(DR_SD,1,raw,&value);
  if(unit!=X0X_U_MS || fm_fabsf(value-expected)>.001f)
   fprintf(stderr,"SNARE_TONE_PHYSICAL_DECAY raw=%d unit=%d value=%f expected=%f\n",raw,unit,value,expected);
  CHECK(unit==X0X_U_MS);CHECK(fm_fabsf(value-expected)<.001f);
  pref_t r=PR(R_ENG,T_909,DR_SD,1);
  cv_begin(240,28,C_BLACK);dv_glyph(r,raw,4,4,1,1,SOUND_INK);memcpy(pixels,cv_px,sizeof pixels);
  cv_begin(240,28,C_BLACK);float points[34];
  for(int i=0;i<=16;i++){
   float t=i/16.f,elapsed=fm_maxf(0.f,2000.f*t-24.f);
   points[2*i]=4+1+30*t;
   points[2*i+1]=4+12*(1-fm_exp2f(-16.60964047f*elapsed/expected));
  }
  cv_poly_aa(points,17,SOUND_INK);
  if(memcmp(pixels,cv_px,sizeof pixels))fprintf(stderr,"SNARE_TONE_CURVE_LAW raw=%d\n",raw);
  CHECK(!memcmp(pixels,cv_px,sizeof pixels));
 }
 drum_curve_t curve;CHECK(dv_909_curve(DR_SD,&curve));
 CHECK(curve.hold==24.f);
 CHECK(fm_fabsf(curve.window-(24.f+.7f*x0x_pot_exp_value(X0X_EXP_SD_TONE,127)))<.001f);
 puts("PASS snare Tone physical decay duration excludes fixed hold");return 0;
}
