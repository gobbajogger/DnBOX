/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static unsigned hz_count;
static uint16_t hz_color;
static int32_t observed_unit_text(int32_t x,int32_t y,const felucca_font_t *f,const char *s,uint16_t color)
{
 if(!strcmp(s,"Hz")){hz_count++;hz_color=color;}
 return cv_text(x,y,f,s,color);
}
#define cv_text observed_unit_text
#include "list_unit_current.inc"
#include "list_unit_before.inc"
#undef cv_text
static uint16_t actual[240*69],observed[240*69],prior[240*69];
static void render_list(void (*draw)(int),int band,uint16_t *dst)
{cv_begin(240,69,C_BLACK);cv_band(0,69);draw(band);memcpy(dst,cv_px,sizeof actual);}
int main(void)
{
 setup(V_HOME,PART_303A);button_tap(B_GLO);global_index();ui.list_sel=2;button_tap(B_SEL);CHECK(ui.overlay==O_LIST);
 int tuning=-1;for(int r=0;r<ui.list_n;r++){char n[36],u[8];pref_value(list_rows[r],n,u);if(!strcmp(u,"Hz")){CHECK(tuning<0);tuning=r;}}
 CHECK(tuning>=0&&tuning<6);ui.list_top=0;unsigned changes=0;
 for(int selected=0;selected<ui.list_n && selected<6;selected++)for(int band=0;band<2;band++){
  ui.list_sel=selected;screen_state_t before=screen_state();int value=pref_get(list_rows[tuning]);
  hz_count=0;render_list(draw_list,band,actual);render_list(observed_draw_list,band,observed);
  CHECK(!memcmp(actual,observed,sizeof actual));
  CHECK(hz_count==(unsigned)(tuning/3==band));
  if(hz_count)CHECK(hz_color==(selected==tuning?C_WHITE:C_GRAY));
  render_list(before_draw_list,band,prior);
  if(selected==tuning && tuning/3==band){CHECK(memcmp(actual,prior,sizeof actual));changes++;}
  else CHECK(!memcmp(actual,prior,sizeof actual));
  unchanged(before);CHECK(pref_get(list_rows[tuning])==value);
 }
 CHECK(changes==1);printf("PASS selected GLOBAL unit foreground, 12 row/band states, %u checks; no full contrast qualification\n",checks);return 0;
}
