/* Independent Edition2 M2 regression; real renderer and physical encoder path. */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static unsigned failures;
#define EXPECT(x, why) do { checks++; if(!(x)){failures++;fprintf(stderr,"M2_FAIL %s\n",why);} }while(0)
static int ref_eq(pref_t a,pref_t b){return a.kind==b.kind&&a.a==b.a&&a.b==b.b&&a.c==b.c;}
static uint16_t captured[240*240];
static void return_raster(void){
 if(cur_page()!=0 || ui.overlay!=O_NONE)return;
 memcpy(captured,fb,sizeof captured);
 const char *labels[1]={"REV RETURN"};
 for(int j=0;j<1;j++){
  int cell=3,cx=cell*60+30,ink=0,match=0,glyph=0;
  pref_t ref=PR(R_ENG,T_FX,0,j?FX_DL_LEVEL:FX_RV_LEVEL);
  if(!ref_eq(pg.r[0][cell],ref))continue; /* baseline reports the missing owner below */
  cv_begin(240,KNOB_H,C_BLACK);text_c(cx,vc(&FONT_XS,13,10),&FONT_XS,labels[j],ui.touched==cell?C_WHITE:C_GRAY);
  for(int y=13;y<23;y++)for(int x=cell*60;x<(cell+1)*60;x++)if(cv_px[y*240+x]==swap16(ui.touched==cell?C_WHITE:C_GRAY)){
   ink++;match+=captured[(KNOB_Y+y)*240+x]==cv_px[y*240+x];
  }
  EXPECT(ink>0&&match==ink,"RETURN_LITERAL_LABEL_RASTER");
  for(int y=28;y<49;y++)for(int x=cell*60+5;x<(cell+1)*60-5;x++)glyph+=captured[(KNOB_Y+y)*240+x]==swap16(part_col());
  EXPECT(glyph>0,"RETURN_REAL_GLYPH_MISSING");
  char num[16],unit[8];pref_value(ref,num,unit);CHECK(!strcmp(unit,"dB"));
  cv_begin(240,KNOB_H,C_BLACK);cell_value(cx,vc(&FONT_B,53,13),num,"dB",ui.touched==cell?C_WHITE:C_HI);
  ink=match=0;for(int y=53;y<66;y++)for(int x=cell*60;x<(cell+1)*60;x++)if(cv_px[y*240+x]==swap16(ui.touched==cell?C_WHITE:C_HI)){
   ink++;match+=captured[(KNOB_Y+y)*240+x]==cv_px[y*240+x];
  }
  EXPECT(ink>0&&match==ink,"RETURN_VALUE_DB_RASTER");
 }
}
static void render(const char *name){ui.msg_until=0;draw_header();draw_main();draw_knobs();shot(name);return_raster();}
static void list_contract(void){
 const pref_t refs[]={PR(R_ENG,T_FX,0,FX_RV_DECAY),PR(R_ENG,T_FX,0,FX_RV_TONE),PR(R_ENG,T_FX,0,FX_DL_TIME),PR(R_ENG,T_FX,0,FX_DL_FDBK),PR(R_ENG,T_FX,0,FX_DL_TONE),PR(R_TAPE,0,0,0),PR(R_HDR,GR_MORE,0,0),PR(R_ENG,T_FX,0,FX_RV_HPF),PR(R_ENG,T_FX,0,FX_RV_LEVEL),PR(R_ENG,T_FX,0,FX_DL_HPF),PR(R_ENG,T_FX,0,FX_DL_LEVEL),PR(R_ENG,T_FX,0,FX_DL_PING)};
 int page=cur_page(),part=ui.part;button_tap(B_SEL);CHECK(ui.overlay==O_LIST);CHECK(ui.list_n==12);
 for(unsigned i=0;i<12;i++){CHECK(ref_eq(list_rows[i],refs[i]));CHECK(list_skip(i)==(i==6));if(i!=6){CHECK(!pref_read_only(list_rows[i]));char value[32],unit[16];pref_value(list_rows[i],value,unit);CHECK(value[0]);printf("ROW index=%u ref=%u:%u:%u:%u value=%s unit=%s\n",i,refs[i].kind,refs[i].a,refs[i].b,refs[i].c,value,unit);}}
 for(unsigned i=0;i<12;i++){if(i==6)continue;ui.list_sel=i;ui.list_top=i>5?i-5:0;char name[80];snprintf(name,sizeof name,"list-theme%u-page%d-row%u",ui_theme_light,page,i);render(name);}
 button_tap(B_HOME);CHECK(ui.overlay==O_NONE);CHECK(cur_page()==page);CHECK(ui.part==part);
}
int main(void){
 for(int theme=0;theme<2;theme++)for(int page=0;page<2;page++){
  setup(V_FX,PART_909);*page_of(V_FX)=page;build_pages();pref_set(PR(R_PALETTE,0,0,0),theme?5:0);CHECK(pg.n==2);
  const pref_t expected[2][4]={{PR(R_ENG,T_FX,0,FX_RV_DECAY),PR(R_ENG,T_FX,0,FX_RV_TONE),PR(R_ENG,T_FX,0,FX_RV_HPF),PR(R_ENG,T_FX,0,FX_RV_LEVEL)},{PR(R_ENG,T_FX,0,FX_DL_TIME),PR(R_ENG,T_FX,0,FX_DL_FDBK),PR(R_ENG,T_FX,0,FX_DL_TONE),PR(R_TAPE,0,0,0)}};
  CHECK(!strcmp(pg.title[page],page?"DELAY":"REVERB"));list_contract();
  for(int focus=-1;focus<4;focus++){ui.touched=focus;ui.touch_until=now_ms+900;char name[80];snprintf(name,sizeof name,"page-theme%d-bank%d-focus%d",theme,page,focus);render(name);}
  for(int cell=0;cell<4;cell++){
   pref_t r=expected[page][cell];EXPECT(ref_eq(pg.r[page][cell],r),"FRONT_BANK_EXACT_OWNER");
   pref_set(r,pref_desc(r)->max/2);int before=pref_get(r);sound_t saved=proj.sound;
   char name[80];snprintf(name,sizeof name,"event-before-theme%d-bank%d-K%d",theme,page,cell+5);render(name);
   enc_acc[EN_K1+cell]=1;ui_input_only();
   int after=pref_get(r);printf("EVENT theme=%d bank=%d K%d before=%d after=%d\n",theme,page,cell+5,before,after);EXPECT(after!=before,cell==2&&!page?"K7_MISSING_REV_LOW_CUT":cell==3&&!page?"K8_MISSING_REV_RETURN":"PHYSICAL_KNOB_OWNER");
   snprintf(name,sizeof name,"event-after-theme%d-bank%d-K%d",theme,page,cell+5);render(name);
   pref_set(r,before);CHECK(!memcmp(&saved,&proj.sound,sizeof saved));pref_set(r,after);
  }
 }
 printf("M2_RESULT checks=%u failures=%u captures=96\n",checks,failures);return failures?1:0;
}
