/* SPDX-License-Identifier: GPL-3.0-only */
/* Portable equivalent-state capture. Install this harness file alone into an
 * isolated copy of either source; never add new product components to baseline.
 * Uses normal ui_frame, actual existing page inventory and typed control IDs.
 * This captures old layouts without pretending the R22-specific glyph tracer
 * applies to a source that predates ui_drum_voice.inc. */
#define SMC_UI_MAIN unused_equivalent_capture_main
#include "smc_ui_test.c"
static unsigned recorded;
static void capture_state(const char *id){
 ui.msg_until=0;ui.touched=-1;
 for(int f=0;f<12;f++){now_ms+=16;ui.dirty=1;ui_frame();}
 shot(id);printf("EQUIVALENT,%s,%d,%d,%d,%d",id,ui.view,ui.part,cur_page(),pg.n);
 for(int k=0;k<4;k++){pref_t r=pg.r[cur_page()][k];printf(",%d:%d:%d:%d=%d",r.kind,r.a,r.b,r.c,r.kind?pref_get(r):0);}puts("");recorded++;
}
int main(void){
 for(int kit=0;kit<2;kit++)for(int voice=0;voice<NDRUM;voice++){
  setup(V_PART,kit);ui.sel[kit]=(uint8_t)voice;build_pages();
  for(int step=0;step<16;step+=4)sm_set(cur_pat()->drum[kit].hit[voice],step);
  int pages=pg.n;
  for(int page=0;page<pages;page++){*page_of(V_PART)=(uint8_t)page;build_pages();char id[96];snprintf(id,sizeof id,"kit%d-voice%02d-page%02d",kit,voice,page);capture_state(id);}
 }
 const int views[]={V_HOME,V_MIX,V_FX,V_SEQ,V_SONG,V_GLO,V_GEN,V_PERF};
 for(unsigned i=0;i<sizeof views/sizeof views[0];i++){setup(views[i],PART_909);char id[80];snprintf(id,sizeof id,"view%02d-part909",views[i]);capture_state(id);}
 printf("EQUIVALENT_COMPLETE,%u\n",recorded);return 0;
}
