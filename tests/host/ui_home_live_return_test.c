/* SPDX-License-Identifier: GPL-3.0-only */
/* Five simultaneous real parts, reached through the existing SELECT action list.
 * A selected-part footer is deliberately not an acceptable substitute. */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static uint16_t matrix_frame[240*240],live_frame[240*240];
static uint32_t project_hash(void){uint32_t h=2166136261u;const uint8_t *p=(const uint8_t*)&proj;for(unsigned i=0;i<sizeof proj;i++)h=(h^p[i])*16777619u;return h;}
static void select_action(const char *label)
{
 button_tap(B_SEL);CHECK(ui.overlay==O_LIST);
 int index=-1;
 for(int i=0;i<ui.list_n;i++)if(list_rows[i].kind==R_ACT&&!strcmp(pref_name(list_rows[i]),label))index=i;
 if(index<0)fprintf(stderr,"HOME_LIVE_ROUTE_MISSING: %s\n",label);
 CHECK(index>=0);
 /* Navigate using actual SELECT detents, not a synthetic replacement action. */
 for(int guard=0;ui.list_sel!=index&&guard<LIST_MAX;guard++){enc_acc[EN_SELECT]++;ui_input_only();}
 CHECK(ui.list_sel==index);button_tap(B_SEL);CHECK(ui.overlay==O_NONE);
}
static void render(uint16_t *out){ui.touched=-1;ui.msg_until=0;draw_header();draw_main();draw_knobs();draw_footer();memcpy(out,fb,240*240*2);}
int main(void)
{
 setup(V_HOME,PART_909);
 pref_t perform[4];memcpy(perform,pg.r[0],sizeof perform);
 uint32_t before=project_hash();int dirty=ui.dirty;
 render(matrix_frame);shot("home-patterns");select_action("LIVE OVERVIEW");render(live_frame);shot("home-live");
 CHECK(project_hash()==before);CHECK(ui.dirty==dirty);CHECK(memcmp(matrix_frame,live_frame,sizeof live_frame));
 CHECK(!memcmp(perform,pg.r[0],sizeof perform));
 CHECK(!memcmp(matrix_frame+(HDR_H+2*BAND_H)*240,live_frame+(HDR_H+2*BAND_H)*240,KNOB_H*480));
 /* All five rows must react independently, not merely the selected part. */
 for(int p=0;p<NPARTS;p++) {
  pattern_t *pt=pat_of(p);
  if(p<NKIT)memset(pt->drum[p].hit,0,sizeof pt->drum[p].hit);
  else if(p==PART_BRK)pt->brk.steps=0;
  else for(int s=0;s<64;s++)pt->bass[p-NKIT].step[s].flags=0;
 }
 render(live_frame);
 for(int p=0;p<NPARTS;p++) {
  pattern_t *pt=pat_of(p);
  if(p<NKIT)sm_set(pt->drum[p].hit[0],3);
  else if(p==PART_BRK)pt->brk.steps=1u<<3;
  else pt->bass[p-NKIT].step[3].flags=G_NOTE;
  render(matrix_frame);
  CHECK(memcmp(live_frame,matrix_frame,sizeof live_frame));
  for(int pos=0;pos<NPARTS;pos++)if(part_at(pos)!=p) {
   int y=HDR_H+18+pos*24;
   CHECK(!memcmp(live_frame+y*240,matrix_frame+y*240,22*480));
  }
  memcpy(live_frame,matrix_frame,sizeof live_frame);
 }
 /* Short and nonmultiple lengths must not fabricate enabled steps. */
 const int lengths[]={1,8,17,31,64};
 for(unsigned j=0;j<sizeof lengths/sizeof lengths[0];j++) {
  int len=lengths[j];pat_of(PART_909)->drum[0].len=(uint8_t)len;
  for(int ph=0;ph<len;ph+=16) {
   seq.playing=1;eng_step[TRK_DRUM]=ph;render(matrix_frame);
   int pos=0;while(part_at(pos)!=PART_909)pos++;
   int y=HDR_H+18+pos*24,base=(ph/16)*16;
   for(int c=0;c<16;c++) {
    int x=85+c*9;
    CHECK(fb[(y+5)*240+x]==swap16(base+c<len?SOUND_EDGE:SOUND_SIDE));
   }
  }
  seq.playing=0;render(matrix_frame);
  shot(j==0?"home-live-one":j==2?"home-live-17":"home-live-length");
 }
 select_action("PATTERNS");CHECK(ui.view==V_HOME);CHECK(!memcmp(perform,pg.r[0],sizeof perform));
 button_tap(B_HOME);CHECK(ui.view==V_SONG);button_tap(B_HOME);CHECK(ui.view==V_HOME);
 puts("PASS reachable five-part live HOME and retained pattern route");return 0;
}
