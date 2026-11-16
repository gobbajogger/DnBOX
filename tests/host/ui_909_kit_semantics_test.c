/* SPDX-License-Identifier: GPL-3.0-only */
/* Velocity sensitivity is not Accent gain or an incoming MIDI velocity. */
#define SMC_UI_MAIN unused_kit_semantics_smc_main
#include "smc_ui_test.c"
#include "dsp/drum909.h"
int main(void){
 setup(V_PART,PART_909);
 pref_t r=PR(R_ENG,T_909,NDRUM,1);
 CHECK(!strcmp(pref_name(r),"VELOCITY"));
 drum909_t *d=malloc(sizeof *d);CHECK(d);drum909_init(d);
 for(int raw=0;raw<128;raw++){
  float x=0;char n[16],u[8],en[16],eu[8];drum909_set(d,DR_KIT,1,raw);
  CHECK(drum909_param_si(DR_KIT,1,raw,&x)==X0X_U_PCT);
  CHECK(x==d->vel_depth*100.f);
  pref_value_of(r,raw,n,u);unit_value(X0X_U_PCT,x,en,eu);
  CHECK(!strcmp(n,en));CHECK(!strcmp(u,eu));
 }
 free(d);puts("909kit velocity sensitivity actualSI/label PASS");return 0;
}
