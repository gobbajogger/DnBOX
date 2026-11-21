/* Refactor oracle: every real drum bank in both palettes, with actual settled owners. */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
int main(void){int count=0;for(int kit=0;kit<2;kit++)for(int voice=0;voice<NDRUM;voice++)for(int theme=0;theme<2;theme++){
 setup(V_PART,kit);ui.sel[kit]=voice;seq.playing=0;engine_guard_enable(0);pref_set(PR(R_PALETTE,0,0,0),theme?5:0);
 int32_t audio[512];for(int i=0;i<12;i++)engine_render(audio,256);build_pages();
 for(int page=0;page<pg.n;page++){*page_of(V_PART)=page;draw_header();draw_main();draw_knobs();draw_footer();
  char name[80];snprintf(name,sizeof name,"kit%d-voice%02d-page%d-theme%d",kit,voice,page,theme);shot(name);count++;
 }
 }printf("PASS actual all-drum-bank equivalence baseline %d captures\n",count);return 0;}
