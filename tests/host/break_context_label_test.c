/* Exact native title ink must survive both real main and knob strip blits. */
#define SMC_UI_MAIN retained_fixture
#include "smc_ui_test.c"
int main(void){
 static const char *const title[]={"LOOPS","SEQUENCE","GROOVE","RETRIG"};
 for(int theme=0;theme<2;theme++)for(int p=0;p<4;p++){
  setup(p?V_SEQ:V_PART,PART_BRK);palette_set(theme?5:0);
  ui.page[ui.view][PART_BRK]=(uint8_t)(p?p-1:0);build_pages();
  ui.msg_until=0;ui.touched=-1;ui.touch_until=0;
  draw_header();draw_main();draw_knobs();
  CHECK(!strcmp(pg.title[cur_page()],title[p]));
  cv_begin(240,20,C_BLACK);cv_text(10,0,&FONT_T,title[p],part_col());
  unsigned pixels=0,missing=0;
  for(int y=0;y<20;y++)for(int x=0;x<125;x++)if(cv_px[y*240+x]==swap16(part_col())){
   pixels++;if(fb[(147+y)*240+x]!=swap16(part_col())){missing++;fprintf(stderr,"BREAK_CONTEXT_MISSING title=%s theme=%d x=%d y=%d\n",title[p],theme,x,147+y);}
  }
  CHECK(pixels>0);CHECK(missing==0);
 }
 printf("BREAK context complete title ink: %u checks PASS\n",checks);return 0;
}
