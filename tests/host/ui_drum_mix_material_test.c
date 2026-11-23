/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN unused_smc_ui_main
#include "smc_ui_test.c"
/* Actual engine telemetry and production renderer. No device claims. */
static void mix_render_ms(int ms)
{
 int32_t audio[512];
 for(int elapsed=0;elapsed<ms;elapsed+=6) {
  now_ms+=6;engine_render(audio,256);
 }
}
int main(void)
{
 for(int kit=0;kit<2;kit++)for(int state=0;state<3;state++) {
  setup(V_MIX,kit);*page_of(V_MIX)=kit+1;ui.sel[kit]=state==2?10:state==1?6:0;build_pages();
  int t=kit?T_808:T_909;
  for(int v=0;v<NDRUM;v++) {
   pref_t lv=find_ref(t,v,"Level"),pn=find_ref(t,v,"Pan");
   CHECK(lv.kind && pn.kind);CHECK(tw(&FONT_XS,engine_voice_name(t,v))<=17);
   proj.sound.v[t][v][lv.c]=state==0?127:state==1?0:12*v;
   proj.sound.v[t][v][pn.c]=state==0?0:state==1?64:127;
  }
  engine_apply_sound(&proj.sound);mix_render_ms(300);
  if(state!=1) {
   for(int v=0;v<NDRUM;v++)if(state==0 || v)
    engine_drum(kit,v,state==0?1.0f:23.0f*v/255.0f);
   mix_render_ms(6);
  }
  if(state==2)seq.vmute=1u<<(kit*NDRUM+6);
  project_t *copy=malloc(sizeof proj);CHECK(copy);memcpy(copy,&proj,sizeof proj);
  ui_frame();CHECK(!memcmp(copy,&proj,sizeof proj));free(copy);
  CHECK(NDRUM==11);if(kit)CHECK(!strcmp(engine_voice_name(t,6),"CP"));
  char name[80];snprintf(name,sizeof name,"mix%d-%s",kit?808:909,state==0?"full-left-live-hit":state==1?"zero-center-idle":"levels-right-live-muted");shot(name);
 }
 /* Direct production-band assertions: top captions, 96px rail, mute/hit
  * separation, clipping, and no drawing into the spaces between strips. */
 for(int kit=0;kit<2;kit++)for(int muted=0;muted<2;muted++)for(int hit=0;hit<2;hit++) {
  setup(V_MIX,kit);*page_of(V_MIX)=kit+1;ui.sel[kit]=0;build_pages();
  int t=kit?T_808:T_909;pref_t lv=find_ref(t,0,"Level");
  proj.sound.v[t][0][lv.c]=127;
  mix_render_ms(300);
  uint32_t age=0;float envelope=0;
  CHECK(!engine_drum_view(kit,0,&age,&envelope));
  if(hit) {
   engine_drum(kit,0,1.0f);mix_render_ms(6);
   CHECK(engine_drum_view(kit,0,&age,&envelope));
   CHECK(envelope>.99f);
  }
  seq.vmute=muted?(1u<<(kit*NDRUM)):0;
  /* 48651ff77/44a9cfee6 (approved R22 drum MIX): one 1 px fader rail at x+7 from
   * y=43 to 106, the live hit fills it in the part colour from the bottom
   * (never while muted), a white cap for the selected strip at 107-level/2,
   * a grey mute slash, and blank columns between strips. */
  uint16_t col=PART_COL[kit];
  cv_begin(240,BAND_H,C_BLACK);draw_drum_mix(0,kit);
  for(int y=44;y<BAND_H;y++)CHECK(cv_px[y*240+15]==swap16(hit&&!muted?col:C_LINE));   /* y=43 is under the cap */
  CHECK(cv_px[43*240+12]==swap16(C_WHITE)&&cv_px[43*240+20]==swap16(C_WHITE)); /* level 127: cap at the top */
  /* the blended slash (1ec2d61a8) changes the strip's top when muted */
  uint32_t slash=2166136261u;for(int y=26;y<=37;y++)for(int x=8;x<=23;x++)slash=(slash^cv_px[y*240+x])*16777619u;
  static uint32_t unmuted_slash[2][2];if(!muted)unmuted_slash[kit][hit]=slash;else CHECK(slash!=unmuted_slash[kit][hit]);
  for(int y=43;y<BAND_H;y++)for(int v=0;v<NDRUM-1;v++)
   CHECK(cv_px[y*240+8+v*21+16]==swap16(SOUND_FACE));
  cv_begin(240,BAND_H,C_BLACK);draw_drum_mix(1,kit);
  for(int y=BAND_H;y<107;y++)
   CHECK(cv_px[(y-BAND_H)*240+15]==swap16(hit&&!muted?col:C_LINE));       /* the rail continues */
  CHECK(cv_px[(107-BAND_H)*240+15]!=swap16(col));                            /* and ends at 106 */
 }
 test_distinct_maps(); /* Both kits, both banks: level and pan mapping unchanged. */
 printf("drum_mix_material PASS (%u checks)\n",checks);return 0;
}
