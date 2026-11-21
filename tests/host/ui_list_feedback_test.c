/* SPDX-License-Identifier: GPL-3.0-only */
/* Literal font-mask oracle over real production framebuffer, never injected output. */
#define SMC_UI_MAIN retained_smc_fixture_main
#include "smc_ui_test.c"
static unsigned failures, expectations;
#define EXPECT(x) do {expectations++;if(!(x)){failures++;fprintf(stderr,"MISSING line %d: %s\n",__LINE__,#x);}}while(0)
static uint16_t captured[240*240];
static void capture(const char *name){ui.msg_until=0;ui_frame();memcpy(captured,fb,sizeof fb);if(name)shot(name);}
static int word(const char *s,int row){
 const felucca_font_t *fonts[]={&FONT_XS,&FONT_S,&FONT_B};
 /* The GLOBAL menu occupies 138 px above the lower controls. Its four- and
  * five-item sections center taller slabs in each row. */
 int pitch=ui.list_n<6 ? 138/ui.list_n : 23;
 int height=21+(pitch-23)/2;
 int y0=MAIN_Y+(row-ui.list_top)*pitch+(pitch-2-height)/2;
 for(unsigned f=0;f<3;f++){
  cv_begin(240,24,C_BLACK);cv_band(0,24);cv_text(0,0,fonts[f],s,C_WHITE);int w=tw(fonts[f],s),h=0;
  for(int y=0;y<24;y++)for(int x=0;x<w;x++)if(cv_px[y*240+x]!=swap16(C_BLACK)&&y+1>h)h=y+1;
  if(!h)continue;
  for(int y=y0;y+h<=y0+height+1;y++)for(int x=120;x+w<=228;x++){
   uint16_t ink=0;int ok=1;
   for(int j=0;j<h&&ok;j++)for(int i=0;i<w;i++)if(cv_px[j*240+i]==swap16(C_WHITE)){
    uint16_t p=captured[(y+j)*240+x+i];if(!ink)ink=p;else if(p!=ink){ok=0;break;}}
   if(ok)for(int j=0;j<h&&ok;j++)for(int i=0;i<w;i++)if(cv_px[j*240+i]==swap16(C_BLACK)&&captured[(y+j)*240+x+i]==ink){ok=0;break;}
   if(ok)return 1;
  }
 }return 0;
}
static void group(int g){global_index();ui.list_sel=g;button_tap(B_SEL);CHECK(ui.glo_section==g);}
static void cue_cases(int palette){
 char name[80];global_index();ui.list_sel=2;snprintf(name,sizeof name,"palette%d-global-categories",palette);capture(name);
 for(int r=0;r<6;r++)EXPECT(word("SCALE",r));
 button_tap(B_SEL);CHECK(ui.glo_section==2&&ui.list_n==10);button_tap(B_HOME);CHECK(ui.glo_section==-1&&ui.list_sel==2);
 group(4);ui.list_sel=3;capture(NULL);CHECK(word("OFF",2));for(int r=0;r<5;r++)EXPECT(word("SCALE",r)==(r!=2));
 button_tap(B_SEL);CHECK(ui.overlay==O_ASK&&ui.ask_act==ACT_CLEAR_PAT);button_tap(B_HOME);CHECK(ui.overlay==O_LIST&&ui.list_sel==3);
 group(5);ui.list_sel=1;snprintf(name,sizeof name,"palette%d-about-action",palette);capture(name);EXPECT(word("SCALE",1));
 clip_grid_mode=clip_on=1;capture(NULL);for(int r=0;r<ui.list_n&&r<6;r++)EXPECT(!word("SCALE",r));
 global_index();capture(NULL);for(int r=0;r<6;r++)EXPECT(!word("SCALE",r));clip_grid_mode=clip_on=0;
}
static void part_cases(int palette){
 static const int parts[]={PART_909,PART_808,PART_BRK,PART_303B,PART_303A};char name[100];group(2);CHECK(ui.list_n==10);
 for(int i=0;i<5;i++){
  int row=3+i,p=parts[i];CHECK(list_rows[row].kind==R_PARTON&&list_rows[row].a==p);
  ui.list_sel=0;ui.list_top=0;list_move(row);CHECK(ui.list_sel==row&&ui.list_top==(row>5?row-5:0));
  for(int selected=0;selected<2;selected++)for(int on=0;on<2;on++){
   ui.list_sel=selected?row:(row==3?4:3);ui.list_top=row>5?row-5:0;
   seq.part_off=0x15;uint8_t prior=seq.part_off;pref_set(list_rows[row],on);
   CHECK(seq.part_off==(uint8_t)(on?prior&~(1u<<p):prior|(1u<<p)));CHECK(pref_get(list_rows[row])==on);
   snprintf(name,sizeof name,"palette%d-part%d-%s-%s",palette,p,on?"on":"off",selected?"selected":"idle");capture(name);
   EXPECT(word(on?"ON":"OFF",row));EXPECT(!word(on?"OFF":"ON",row));
   uint16_t saved[240*23];int y=MAIN_Y+(row-ui.list_top)*23;memcpy(saved,captured+y*240,sizeof saved);
   seq.mute^=MUTE_MASK[p];capture(NULL);CHECK(!memcmp(saved,captured+y*240,sizeof saved));EXPECT(word(on?"ON":"OFF",row));seq.mute^=MUTE_MASK[p];pref_t level=PR(R_ENG,T_MIX,p,MX_LEVEL);int old=pref_get(level);pref_set(level,old?0:127);
   capture(NULL);CHECK(!memcmp(saved,captured+y*240,sizeof saved));EXPECT(word(on?"ON":"OFF",row));pref_set(level,old);
  }
  ui.list_sel=row;int prior=pref_get(list_rows[row]);uint8_t bits=seq.part_off;turn_ref(list_rows[row],prior?-1:1,EN_ALGO);CHECK(pref_get(list_rows[row])==!prior);CHECK((seq.part_off^bits)==(1u<<p));
 }
 ui.list_sel=0;ui.list_top=0;list_move(9);CHECK(ui.list_sel==9&&ui.list_top==4);button_tap(B_HOME);CHECK(ui.glo_section==-1);button_tap(B_SEL);CHECK(ui.list_sel==9&&ui.list_top==4);
}
int main(void){setup(V_HOME,PART_909);button_tap(B_GLO);for(int p=0;p<2;p++){pref_set(PR(R_PALETTE,0,0,0),p?5:0);cue_cases(p);part_cases(p);}printf("list feedback: %u expectations, %u missing; %u preserved behavior checks\n",expectations,failures,checks);return failures?1:0;}
