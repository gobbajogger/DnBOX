/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN retained_main
#include "smc_ui_test.c"
int main(void){unsigned states=0;
 for(int theme=0;theme<6;theme++)for(int kit=0;kit<NKIT;kit++)for(int voice=0;voice<NDRUM;voice++){
  setup(V_PART,kit);palette_set(theme);ui.sel[kit]=voice;build_pages();int pages=pg.n;
  for(int page=0;page<pages;page++){
   *page_of(V_PART)=page;build_pages();ui_ease_reset();memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();
   int y=26;for(int p=0;p<page;p++){int cells=0;for(int k=0;k<4;k++)if(pg.r[p][k].kind!=R_NONE)cells++;y+=cells>1?42:32;}
   CHECK(y+20<240);uint16_t wanted=swap16(theme==5?SOUND_SELECT:RGB(36,48,54));
   if(fb[(y+20)*240+7]!=wanted)fprintf(stderr,"ACTIVE_FILL_MISMATCH theme=%d kit=%d voice=%d page=%d x7 y%d actual=%04x expected=%04x\n",theme,kit,voice,page,y+20,fb[(y+20)*240+7],wanted);
   CHECK(fb[(y+20)*240+7]==wanted);
   uint32_t hash=2166136261u;for(unsigned i=0;i<sizeof fb;i++)hash=(hash^((uint8_t*)fb)[i])*16777619u;
   printf("FRAME theme=%d kit=%d voice=%d page=%d hash=%08x\n",theme,kit,voice,page,hash);
   CHECK(!seq.playing&&ui.overlay==O_NONE&&!ui.help);states++;
   if(theme==5&&kit==0&&voice==0&&page==0)shot("light-bd-page0");
  }
 }
 printf("Active material sweep: %u states, %u checks PASS\n",states,checks);return 0;
}
