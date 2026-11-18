/* SPDX-License-Identifier: GPL-3.0-only */
/* Presentation aliases must not rename stored/native parameter identities. */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
int main(void)
{
 setup(V_PART,PART_909);ui.sel[PART_909]=0;build_pages();
 pref_t depth=find_ref(T_909,0,"P.Dpth");CHECK(depth.kind==R_ENG);
 CHECK(!strcmp(pref_desc(depth)->name,"P.Dpth"));
 CHECK(!strcmp(pref_name(depth),"Depth"));CHECK(!strcmp(matrix_name(depth),"DEPTH"));
 CHECK(drum_strip_role(depth)==DG_BDSWEEP);
 int stored=pref_get(depth);
 for(int page=0;page<pg.n;page++) {
  *page_of(V_PART)=(uint8_t)page;build_pages();ui_frame();
  char name[80];snprintf(name,sizeof name,"view-01-part-00-voice-00-page-%02d",page);shot(name);
  for(int knob=0;knob<4;knob++) {
   pref_t r=pg.r[page][knob];const x0x_param_t *d=pref_desc(r);if(r.kind==R_NONE || !d)continue;
   printf("CONTROL {\"screen\":\"%s\",\"page\":%d,\"knob\":%d,\"kind\":%u,\"target\":%u,\"voice\":%u,\"slot\":%u,\"nativeName\":\"%s\",\"displayName\":\"%s\"}\n",name,page,knob,r.kind,r.a,r.b,r.c,d->name,pref_name(r));
  }
  CHECK(pref_get(depth)==stored);
 }
 button_tap(B_SEL);CHECK(ui.overlay==O_LIST);
 int found=0;
 for(int row=0;row<ui.list_n;row++) {
  pref_t r=list_rows[row];
  if(r.kind==depth.kind && r.a==depth.a && r.b==depth.b && r.c==depth.c){ui.list_sel=(uint8_t)row;ui.list_top=(uint8_t)(row>5?row-5:0);found=1;break;}
 }
 CHECK(found);ui_frame();shot("depth-deep-list");CHECK(pref_get(depth)==stored);
 printf("PRESENTATION native=P.Dpth display=DEPTH target=%u voice=%u slot=%u glyph=DG_BDSWEEP\n",depth.a,depth.b,depth.c);
 printf("PASS readable label and preserved identity/value: %u checks\n",checks);
 return 0;
}
