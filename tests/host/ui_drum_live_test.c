/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN unused_smc_ui_main
#include "smc_ui_test.c"
static void drain_ms(int ms) { int32_t a[512]; for (int i=0;i<ms;i+=6)engine_render(a,256);audio_due_ms=now_ms;run_ms(ms); }
static uint32_t live_glyph(pref_t r,int value) {
 cv_begin(240,KNOB_H,C_BLACK);dv_glyph(r,(float)value,10,10,1,1,C_HI);
 uint32_t h=2166136261u;
 for(int y=8;y<32;y++)for(int x=8;x<45;x++)h=(h^cv_px[y*240+x])*16777619u;
 return h;
}
int main(void) {
 int seen[32]={0};
 for(int part=PART_909;part<=PART_808;part++)for(int voice=0;voice<NDRUM;voice++) {
  setup(V_PART,part);ui.sel[part]=(uint8_t)voice;build_pages();
  for(int p=0;p<pg.n;p++)for(int k=0;k<4;k++) {
   pref_t r=pg.r[p][k];int role=drum_strip_role(r);
   if(role!=DG_SOUND && role!=DG_TUNE && role!=DG_BDSWEEP && role!=DG_LPF && role!=DG_ACCENT && role!=DG_SHELLS && role!=DG_SNAPPY)continue;
   if(seen[role] && !(part==PART_909 && voice==0 && (r.c==4 || r.c==5)))continue;
   int value=pref_desc(r)->max/2;pref_set(r,value);
   drain_ms(12000);uint32_t idle=live_glyph(r,value);
   engine_drum(part,voice,1.f);drain_ms(12);
   uint32_t age;float env;CHECK(drum_live(r,&age,&env));
   uint32_t hit=live_glyph(r,value);
   drain_ms(40);uint32_t later=live_glyph(r,value);
   if(hit==idle && later==idle)fprintf(stderr,"no live change role=%d kit=%d voice=%d\n",role,part,voice);
   CHECK(hit!=idle || later!=idle);
   drain_ms(12000);live_glyph(r,value);drain_ms(DG_BACK_MS+20);
   CHECK(live_glyph(r,value)==idle);
   if(role==DG_SHELLS || role==DG_SNAPPY) {
    /* Engine is idle; exercise the production return clock at exact phases. */
    dg_last[r.a==T_808][r.b]=now_ms|1u;
    uint32_t start=live_glyph(r,value);
    now_ms+=DG_BACK_MS/2;uint32_t middle=live_glyph(r,value);
    CHECK(start!=middle);CHECK(middle!=idle);
    now_ms+=DG_BACK_MS;CHECK(live_glyph(r,value)==idle);
   }
   if(role==DG_SOUND) {
    uint32_t old=live_glyph(r,0);
    for(int v=1;v<=pref_desc(r)->max;v++){uint32_t next=live_glyph(r,v);CHECK(next!=old);old=next;}
   }
   seen[role]=1;
  }
 }
 CHECK(seen[DG_ACCENT]);CHECK(seen[DG_SOUND]);CHECK(seen[DG_TUNE]);CHECK(seen[DG_BDSWEEP]);CHECK(seen[DG_LPF]);CHECK(seen[DG_SHELLS]);CHECK(seen[DG_SNAPPY]);
 printf("PASS R22 production dv_glyph live/idle and discrete SOUND (%u checks)\n",checks);return 0;
}
