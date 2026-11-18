/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
#include "ui_drum_mix_selected_voice_refs.inc"
static uint32_t sound_hash(void){uint32_t h=2166136261u;const unsigned char*p=(const unsigned char*)&proj.sound;for(unsigned i=0;i<sizeof proj.sound;i++)h=(h^p[i])*16777619u;return h;}
static int same_ref(pref_t r,const unsigned char e[4]){return r.kind==e[0]&&r.a==e[1]&&r.b==e[2]&&r.c==e[3];}
int main(void)
{
 for(int kit=0;kit<2;kit++){
  setup(V_MIX,kit);ui.page[V_MIX][0]=kit+1;build_pages();
  seq.playing=1;proj.set.keysound=0;
  for(int voice=0;voice<NDRUM;voice++){
   uint32_t before=sound_hash();int other=ui.sel[1-kit];
   held_keys=1u<<BLACK_KEY[voice];input();held_keys=0;input();
   CHECK(ui.sel[kit]==voice);CHECK(ui.sel[1-kit]==other);CHECK(sound_hash()==before);
   CHECK(drum_mix_page()==kit);
   for(int cell=0;cell<4;cell++){
    pref_t r=pg.r[cur_page()][cell];CHECK(same_ref(r,expected_mix_refs[kit][voice][cell]));
    CHECK(pref_desc(r));
    pref_set(r,64);int old=pref_get(r);
    sound_t saved=proj.sound;enc_acc[EN_K1+cell]++;ui_input_only();
    CHECK(pref_get(r)!=old);
    int changed=pref_get(r);pref_set(r,old);CHECK(!memcmp(&proj.sound,&saved,sizeof saved));pref_set(r,changed);
   }
   button_tap(B_SEL);CHECK(ui.overlay==O_LIST);
   for(int cell=0;cell<4;cell++){
    int found=0;for(int i=0;i<ui.list_n;i++)found|=same_ref(list_rows[i],expected_mix_refs[kit][voice][cell]);CHECK(found);
   }
   button_tap(B_HOME);CHECK(ui.overlay==O_NONE);
   ui.touched=-1;ui.msg_until=0;draw_header();draw_main();draw_knobs();
   char name[48];snprintf(name,sizeof name,"mix-%s-selected-%02d",kit?"808":"909",voice);shot(name);
   for(int edge=0;edge<3;edge++){
    const int values[3]={0,64,127};const char *pan[3]={"L50","C","R50"};
    for(int cell=0;cell<4;cell++){pref_t r=pg.r[cur_page()][cell];CHECK(pref_desc(r)->max==127);pref_set(r,values[edge]);}
    char pn[32],unit[8];pref_value(pg.r[cur_page()][1],pn,unit);CHECK(!strcmp(pn,pan[edge]));CHECK(!unit[0]);
    ui.touched=-1;ui.msg_until=0;draw_header();draw_main();draw_knobs();
    snprintf(name,sizeof name,"mix-%s-selected-%02d-edge%d",kit?"808":"909",voice,edge);shot(name);
   }
   printf("MIX_SELECTED kit=%d voice=%d refs=",kit,voice);
   for(int cell=0;cell<4;cell++){pref_t r=pg.r[cur_page()][cell];char n[32],u[8];pref_value(r,n,u);printf("%u:%u:%u:%u=%s%s%s",r.kind,r.a,r.b,r.c,n,u,cell==3?"\n":",");}
  }
 }
 puts("PASS all22 real selected-voice MIX bindings, physical selection/knob/list ownership");
 return 0;
}
