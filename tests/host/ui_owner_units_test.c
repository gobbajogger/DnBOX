/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
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
 printf("OWNER UNITS PASS %u checks\n",checks);return 0;
}
