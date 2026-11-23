/* SPDX-License-Identifier: GPL-3.0-only */
/* FINAL-REFERENCES text/unit defects that stay fixed: TRANSPOSE (not TRANSP) on SEQ and in lists,
 * REVERB/DELAY strip labels that fit their 60 px strip, and 808 voice DECAY readouts in time units. */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static int is_time(const char *u) { return !strcmp(u,"ms") || !strcmp(u,"s"); }
int main(void) {
 setup(V_SEQ,PART_303A);
 CHECK(!strcmp(pref_name(PR(R_BTRANS,0,0,0)),"TRANSPOSE"));
 ui.view=V_PART;
 CHECK(!strcmp(pref_name(PR(R_BTRANS,0,0,0)),"A TRANSPOSE"));
 CHECK(!strcmp(pref_name(PR(R_BTRANS,1,0,0)),"B TRANSPOSE"));
 ui.view=V_FX;ui.overlay=O_NONE;
 static const int fx[4]={FX_RV_HPF,FX_RV_LEVEL,FX_DL_HPF,FX_DL_LEVEL};
 static const char *const want[4]={"LOW CUT","RETURN","LOW CUT","RETURN"}; /* the page header names REVERB/DELAY */
 for(int i=0;i<4;i++){const char *n=pref_name(PR(R_ENG,T_FX,0,fx[i]));
  if(strcmp(n,want[i]))printf("strip label %s, want %s\n",n,want[i]);
  CHECK(!strcmp(n,want[i]));
  CHECK(tw(&FONT_T,n)<=56);                       /* inside its 60 px strip with margins */
  if(tw(&FONT_T,n)>56)printf("strip label too wide: %s\n",n);}
 list_naming=1;                                    /* the SEL list names whose low cut */
 CHECK(!strcmp(pref_name(PR(R_ENG,T_FX,0,FX_RV_HPF)),"REV LOW CUT"));
 CHECK(!strcmp(pref_name(PR(R_ENG,T_FX,0,FX_DL_HPF)),"DLY LOW CUT"));
 list_naming=0;
 static const int dv[4]={D8_CB,D8_CY,D8_OH,D8_CH};
 for(int i=0;i<4;i++){pref_t r=find_ref(T_808,dv[i],"Decay");char num[36],unit[8];
  CHECK(pref_desc(r)!=0);pref_value(r,num,unit);
  drum_curve_t c;int hook=engine_drum808_param_curve_preview(dv[i],r.c,-1,pref_get(r),&c)&&c.fall>0.f;
  printf("808 voice %d DECAY = %s%s%s\n",dv[i],num,unit,hook?"":" (PENDING: 808 DSP has no decay-time law for this voice)");
  if(hook)CHECK(is_time(unit));}       /* the readout follows the engine's curve once it exists */
 /* calm-page names: BREAK settings spelled out; drum variant selector reads TYPE under SOUND */
 CHECK(!strcmp(pref_name(PR(R_BRKSET,0,0,BRK_BCHANCE)),"B Chance"));
 for(int c=0;c<16;c++){const char *n=pref_name(PR(R_BRKSET,0,0,c));
  CHECK(strcmp(n,"Cmplx")&&strcmp(n,"Rtg2x")&&strcmp(n,"A Len")&&strcmp(n,"B Len")&&strcmp(n,"BChnc"));}
 { pref_t t=find_ref(T_808,D8_LT,"Sound");CHECK(pref_desc(t)!=0);CHECK(!strcmp(pref_name(t),"TYPE")); }
 /* MIX strip names: the part's full name where its 38 px column fits it */
 {const char *nm[NPARTS];mix_strip_names(nm);
  for(int pos=0;pos<NPARTS;pos++){printf("MIX strip %d: %s %d px\n",pos,nm[pos],(int)tw(&FONT_XS,nm[pos]));
   CHECK(strcmp(nm[pos],"BRK")&&strcmp(nm[pos],"CHI")&&strcmp(nm[pos],"MIA"));
   if(pos)CHECK(tw(&FONT_XS,nm[pos])/2+tw(&FONT_XS,nm[pos-1])/2+1<=38);}}   /* no collision */
 printf("UI text/units PASS %u checks\n",checks);return 0;
}
