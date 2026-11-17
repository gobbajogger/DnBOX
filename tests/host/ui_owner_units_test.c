/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
#include "../../experiments/original_synth/miami_selected_voice.h"
static void units(pref_t r,int v,const char *want) {
 char n[32],u[16];pref_value_of(r,v,n,u);printf("UNIT kind=%d a=%d b=%d c=%d v=%d result=%s%s want=%s\n",r.kind,r.a,r.b,r.c,v,n,u,want);CHECK(!strcmp(u,want));CHECK(n[0]);
}
int main(void) {
 setup(V_SEQ,PART_909);units(PR(R_DLEN,0,0,0),15,"steps");units(PR(R_BLEN,0,0,0),15,"steps");
 units(PR(R_ACCENT,0,0,0),127,"%");
 units(PR(R_ENG,T_909,0,0),34,"ms");units(PR(R_ENG,T_909,0,5),45,"st");
 units(PR(R_ENG,T_909,11,1),127,"%");units(PR(R_ENG,T_808,11,1),127,"%");
 for(int v=0;v<11;v++)for(int c=0;c<engine_nparams(T_808,v);c++) {
  const x0x_param_t*d=engine_param(T_808,v,c);
  if(!strcmp(d->name,"Decay"))units(PR(R_ENG,T_808,v,c),64,(v>=2&&v<=4)?"ms":"%");
 }
 setup(V_PART,PART_303A);CHECK(engine_bass_select_preset(0,ORIGINAL_SOUND_CHICAGO,0,&proj.sound)==0);run_ms(300);
 for(int v=0;v<=127;v++) {
  char n[32],u[16];pref_value_of(PR(R_ENG,T_303,0,0),v,n,u);
  CHECK(!*u);CHECK(strstr(n,"TRI")||strstr(n,"SAW")||strstr(n,"SQR")||strstr(n,"PUL")||!strcmp(n,"OFF"));
 }
 fxbus_t livefx={0};memset(livefx.pots,255,sizeof livefx.pots);
 for(int value=0;value<=127;value++) {
  float amount;char n[32],u[16];
  fxbus_set(&livefx,FX_RV_DECAY,value);
  CHECK(fxbus_param_si(FX_RV_DECAY,value,&amount)==X0X_U_PCT);
  CHECK(fabsf(amount-livefx.rv_decay*100.f)<.0001f);
  pref_value_of(PR(R_ENG,T_FX,0,FX_RV_DECAY),value,n,u);
  CHECK(!strcmp(u,"%"));CHECK(fabsf(strtof(n,0)-amount)<=.501f);
  pref_value_of(PR(R_ENG,T_FX,0,FX_DRIVE),value,n,u);
  CHECK(!strcmp(u,"%"));CHECK(fabsf(strtof(n,0)-value*100.f/127.f)<=.501f);
 }
 units(PR(R_ENG,T_FX,0,FX_DRIVE),41,"%");
 units(PR(R_ENG,T_FX,0,FX_RV_DECAY),73,"%");
 setup(V_PART,PART_303A);CHECK(engine_bass_select_preset(0,ORIGINAL_SOUND_MIAMI,0,&proj.sound)==0);run_ms(300);
 for(int value=0;value<=127;value++) {
  pref_t r=PR(R_ENG,T_303,0,7);char n[32],u[16];pref_value_of(r,value,n,u);
  CHECK(!strcmp(u,"%"));
  const ui_original_control_view *v=original_text_view(r,value);CHECK(v);
  CHECK(fabsf(strtof(n,0)-v->xmod_tune*.1f)<.051f);
  /* Independent audio-control compilation, not only the UI projection cache. */
  original_miami_program program;original_miami_voice_controls control;
  CHECK(original_sound_restore_miami(&program,proj.sound.v[T_303][0]));
  if(value!=proj.sound.v[T_303][0][7]) {
   float x=value<64?value/128.f:.5f+(value-64)/126.f;
   CHECK(original_miami_program_set(&program,7,x));
  }
  CHECK(original_miami_voice_compile(&control,&program));
  CHECK(fabsf(strtof(n,0)-100.f*control.patch_slave)<=.051f);
 }
 printf("OWNER UNITS PASS %u checks\n",checks);return 0;
}
