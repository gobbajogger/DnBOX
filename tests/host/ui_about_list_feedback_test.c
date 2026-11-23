/* SPDX-License-Identifier: GPL-3.0-only */
/* Release display configuration fixture only; all drawing and navigation real.
 * The current qualification recipe has no --release, hence actual label is empty.
 * Historical short and synthetic long labels test the pre-existing fit policy. */
static const char *fixture_release_label="";
#define X0X_RELEASE_LABEL fixture_release_label
#define SMC_UI_MAIN retained_smc_fixture_main
#include "smc_ui_test.c"
int main(void){
 setup(V_HOME,PART_909);button_tap(B_GLO);
 const char *labels[]={"","0.0.24_openbeta","0.0.24_openbeta_openbeta_openbeta_openbeta"};
 for(int palette=0;palette<2;palette++)for(int selected=0;selected<2;selected++)for(int label=0;label<3;label++){
  pref_set(PR(R_PALETTE,0,0,0),palette?5:0);global_index();ui.list_sel=5;button_tap(B_SEL);CHECK(ui.glo_section==5);
  CHECK(list_rows[1].kind==R_ACT&&list_rows[1].a==ACT_ABOUT);ui.list_sel=selected?1:0;fixture_release_label=labels[label];
  int32_t x=0,w=0;int fits=release_label_box(fixture_release_label,selected,&x,&w);
  CHECK(fits==(label==1));if(fits){CHECK(x>=8+tw(selected?&FONT_B:&FONT_S,"ABOUT DnBOX")+4);CHECK(x+w<=226-tw(&FONT_XS,action_key_cue())-8);}
  ui.msg_until=0;ui_frame();char name[80];snprintf(name,sizeof name,"about-palette%d-%s-label%d",palette,selected?"selected":"idle",label);shot(name);
  if(selected){button_tap(B_SEL);CHECK(ui.overlay==O_LIST&&ui.msg_until>now_ms);ui.msg_until=0;}
 }
 printf("ABOUT release fit PASS %u checks; current empty, historical short, synthetic long\n",checks);
 unsigned prior_checks=checks;
 for(int palette=0;palette<2;palette++){
  pref_set(PR(R_PALETTE,0,0,0),palette?5:0);global_index();ui.list_sel=2;button_tap(B_SEL);CHECK(ui.glo_section==2&&ui.overlay==O_LIST);
  for(int i=0;i<5;i++){
   int row=3+i;pref_t r=list_rows[row];CHECK(r.kind==R_PARTON&&r.a==part_at(i));
   ui.list_sel=0;ui.list_top=0;list_move(row);
   for(int on=0;on<2;on++){
    pref_set(r,on);uint8_t prior_bits=seq.part_off;now_ms+=80;enc_acc[EN_ALGO]=on?-1:1;input();
    CHECK(ui.overlay==O_LIST&&ui.list_sel==row);CHECK(pref_get(r)==!on);CHECK((seq.part_off^prior_bits)==(1u<<r.a));
   }
  }
 }
 printf("physical PARTON input PASS %u supplementary checks: both palettes,5 parts,ON/OFF,real EN_ALGO input\n",checks-prior_checks);return 0;
}
