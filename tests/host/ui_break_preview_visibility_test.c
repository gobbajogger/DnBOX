/* Actual chooser state remains visible after its toast expires. */
#define SMC_UI_MAIN inherited_smc_fixture
#include "smc_ui_test.c"
static void preview_metadata(const char *text){
 CHECK(strlen(text)<40);CHECK(8+tw(&FONT_T,text)<234);
 CHECK(8+tw(&FONT_T,"SEL CHOOSE")<132);CHECK(132+tw(&FONT_T,"HOME CANCEL")<234);
 cv_begin(240,BAND_H,C_BLACK);night_panel(3,1,234,65,0);
 cv_text(8,44,&FONT_T,text,C_WHITE);cv_text(8,55,&FONT_T,"SEL CHOOSE",C_WHITE);
 cv_text(132,55,&FONT_T,"HOME CANCEL",C_WHITE);
 for(int y=44;y<BAND_H;y++)for(int x=0;x<240;x++)
  CHECK(fb[(MAIN_Y+BAND_H+y)*240+x]==cv_px[y*240+x]);
}
int main(void){
 for(int bank=0;bank<2;bank++){
  setup(V_PART,PART_BRK);for(int i=0;i<16;i++)audio_block();
  brkpart_t before=cur_pat()->brk;CHECK(engine_brk_nslots()>1);
  static uint16_t normal[240*(BAND_H-44)],assigned[240*44];draw_main();
  memcpy(assigned,fb+(MAIN_Y+BAND_H)*240,sizeof assigned);
  memcpy(normal,fb+(MAIN_Y+BAND_H+44)*240,sizeof normal);
  CHECK(brk_choice_turn(PR(R_BRKSLOT,bank,0,0),1));audio_block();
  CHECK(engine_brk_preview_active());CHECK(!memcmp(&before,&cur_pat()->brk,sizeof before));
  now_ms+=1500;draw_header();draw_main();draw_knobs();CHECK(ui.msg_until==0);
  char label[40];snprintf(label,sizeof label,"PREVIEW %c %s",bank?'B':'A',engine_brk_slot_names()[ui.brk_choice_slot]);
  shot(bank?"break-preview-B-persistent":"break-preview-A-persistent");
  preview_metadata(label);
  CHECK(!memcmp(assigned,fb+(MAIN_Y+BAND_H)*240,sizeof assigned));
  /* The real knob touch must not cover candidate/actions with a value popup. */
  ui.touched=bank;ui.touch_until=now_ms+1000;draw_main();preview_metadata(label);
  ui.touch_until=now_ms;draw_main();preview_metadata(label);
  for(int slot=0;slot<engine_brk_nslots();slot++){
   int n=snprintf(label,sizeof label,"PREVIEW B %s",engine_brk_slot_names()[slot]);
   CHECK(n>=0&&n<(int)sizeof label);CHECK(8+tw(&FONT_T,label)<234);
  }
  CHECK(brk_choice_turn(PR(R_BRKSLOT,bank,0,0),1));audio_block();now_ms+=1500;
  draw_header();draw_main();draw_knobs();
  snprintf(label,sizeof label,"PREVIEW %c %s",bank?'B':'A',engine_brk_slot_names()[ui.brk_choice_slot]);
  preview_metadata(label);CHECK(!memcmp(&before,&cur_pat()->brk,sizeof before));
  button_tap(B_HOME);audio_block();CHECK(!ui.brk_choice_on);CHECK(!engine_brk_preview_active());
  CHECK(!memcmp(&before,&cur_pat()->brk,sizeof before));
  engine_brk_flush();draw_main();CHECK(!memcmp(normal,fb+(MAIN_Y+BAND_H+44)*240,sizeof normal));
  CHECK(brk_choice_turn(PR(R_BRKSLOT,bank,0,0),1));int chosen=ui.brk_choice_slot;
  button_tap(B_SEL);audio_block();CHECK(!ui.brk_choice_on);CHECK(!engine_brk_preview_active());
  CHECK((bank?cur_pat()->brk.slot_b:cur_pat()->brk.slot_a)==chosen);
 }
 puts("BREAK persistent candidate/choose/cancel visibility PASS");return 0;
}
