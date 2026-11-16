/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN retained_main
#include "smc_ui_test.c"
#include "fwui_glyph_cases.inc"
int main(void){unsigned states=0,bad=0;
 /* Exercise the observer against real font decoding. No RGB color exclusion. */
 cv_begin(240,20,C_BLACK);cv_band(0,20);
 for(unsigned i=0;i<240*20;i++){fwui_glyph_mask[i]=1;fwui_glyph_after[i]=cv_px[i];}
 fwui_glyph_hit_calls=0;cv_text(10,3,&FONT_T,"LABEL",C_WHITE);CHECK(fwui_glyph_hit_calls==1);CHECK(fwui_glyph_hit_pixels>0);fwui_text_commit_origin(0,0);
 cv_begin(240,20,C_BLACK);cv_band(0,20);
 for(unsigned i=0;i<240*20;i++){fwui_glyph_mask[i]=1;fwui_glyph_after[i]=cv_px[i];}
 cv_rect(0,0,240,20,C_WHITE);fwui_glyph_hit_calls=0;cv_text(10,3,&FONT_T,"LABEL",C_BLACK);CHECK(!fwui_glyph_hit_calls);fwui_text_commit_origin(0,0);
 cv_begin(240,20,C_BLACK);cv_band(0,20);fwui_glyph_hit_calls=0;cv_text(10,3,&FONT_T,"LABEL",C_WHITE);CHECK(!fwui_glyph_hit_calls);fwui_text_commit_origin(0,0);

 for(unsigned i=0;i<sizeof cases/sizeof cases[0];i++){
  const sweep_case *c=&cases[i];setup(V_PART,c->kit==1?PART_808:PART_909);ui.sel[ui.part]=c->selected_voice;build_pages();*page_of(V_PART)=c->page;build_pages();
  pref_t r=PR(c->kind,c->target,c->voice,c->slot);const x0x_param_t *d=pref_desc(r);CHECK(d&&!strcmp(d->name,c->name));int found=0;for(int k=0;k<4;k++)if(!memcmp(&r,&pg.r[c->page][k],sizeof r))found++;CHECK(found==1);
  for(int raw=0;raw<=d->max;raw++){
   pref_set(r,raw);int32_t audio[512];engine_render(audio,256);CHECK(pref_get(r)==raw);CHECK(matrix_value(r)==raw);ui_ease_reset();
   uint32_t age;float env;CHECK(!drum_live(r,&age,&env));CHECK(env==0);
   fwui_glyph_hit_calls=fwui_glyph_hit_pixels=0;memset(blit_hash,0,sizeof blit_hash);memset(cv_tile_valid,0,sizeof cv_tile_valid);ui_frame();
   if(fwui_glyph_hit_calls){fprintf(stderr,"LAYOUT_CASE target=%d voice=%d slot=%d raw=%d calls=%u pixels=%u\n",c->target,c->voice,c->slot,raw,fwui_glyph_hit_calls,fwui_glyph_hit_pixels);bad++;}
   CHECK(pref_get(r)==raw);states++;
  }
 }
 printf("GLYPH_LAYOUT states=%u bad=%u checks=%u outside=%llu\n",states,bad,checks,fwui_text_outside);return 0;
}
