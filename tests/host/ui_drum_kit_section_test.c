/* Real production CP/MA 240x240 section/page association. */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static unsigned failures,states;
static void text_region(const char *text,int x,int y,int w,uint16_t ink,const char *reason)
{
 cv_begin(240,FONT_T.h,SOUND_FACE);cv_text(x,0,&FONT_T,text,ink);
 for(int yy=y;yy<y+FONT_T.h;yy++)for(int xx=x;xx<x+w;xx++)
  if(fb[yy*240+xx]!=cv_px[(yy-y)*240+xx]){
   fprintf(stderr,"%s theme=%u page=%d choice=%d state=%u pixel=%d,%d\n",reason,ui_theme_light,cur_page(),pref_get(pg.r[0][2]),states,xx,yy);failures++;return;
  }
}
static void render_check(int choice,int state)
{
 ui.touched=-1;ui.msg_until=0;draw_header();draw_main();draw_knobs();
 int page=cur_page();CHECK(pg.n==2);CHECK(pg.group[page]==(page?GR_KIT:GR_TRACK));
 CHECK(pg.r[1][0].kind==R_ENG&&pg.r[1][0].a==T_808&&pg.r[1][0].b==NDRUM&&pg.r[1][0].c==1);
 CHECK(pg.r[0][0].b==D8_CP&&pg.r[0][1].b==D8_CP&&pg.r[0][2].b==D8_CP&&pg.r[0][3].b==D8_CP);
 states++;
 char name[80];snprintf(name,sizeof name,"cpma-theme%u-choice%d-page%d-state%d",ui_theme_light,choice,page,state);shot(name);
 text_region(page?"KIT":"SOUND",11,28,40,page?SOUND_GRAY:SOUND_INK,"KIT_SECTION_PAGE_ASSOCIATION_WRONG");
 cv_begin(240,FONT_T.h,SOUND_FACE);text_r(232,0,&FONT_T,page?"2/2":"1/2",SOUND_GRAY);
 for(int y=28;y<28+FONT_T.h;y++)for(int x=214;x<232;x++)CHECK(fb[y*240+x]==cv_px[(y-28)*240+x]);
 text_region("KEY SOUND OFF",91,28,80,SOUND_GRAY,"KIT_KEY_SOUND_CUE_LOST");
 text_region("K5 TUNE",63-text_w(&FONT_T,"K5 TUNE")/2,40,text_w(&FONT_T,"K5 TUNE"),SOUND_GRAY,"KIT_TUNE_CONTROL_LOST");
 text_region("K7 SOUND",177-text_w(&FONT_T,"K7 SOUND")/2,40,text_w(&FONT_T,"K7 SOUND"),SOUND_GRAY,"KIT_SOUND_CONTROL_LOST");
 text_region("KIT ACCENT",11,180,text_w(&FONT_T,"KIT ACCENT"),page?SOUND_INK:SOUND_GRAY,"KIT_ACCENT_CONTROL_LOST");
 /* Independently composed complete sixteen-step lane, including absolute labels. */
 cv_begin(240,16,SOUND_BASE);cv_rect(3,0,234,16,SOUND_FACE);
 for(int s=0;s<16;s++){cv_rect(9+s*14,0,11,4,s==0?C_WHITE:(s&1)?part_col():SOUND_LINE);char n[4];put_i(n,s+1);text_c(14+s*14,6,&FONT_T,n,SOUND_GRAY);}
 CHECK(!memcmp(fb+212*240,cv_px,240*16*sizeof(uint16_t)));
}
int main(void)
{
 for(int theme=0;theme<2;theme++)for(int choice=0;choice<2;choice++){
  setup(V_PART,PART_808);ui.sel[PART_808]=D8_CP;build_pages();proj.set.keysound=0;
  proj.set.palette=theme?5:0;palette_set(proj.set.palette);CHECK(ui_theme_light==(unsigned)theme);
  pref_set(pg.r[0][2],choice);seq.playing=1;eng_step[TRK_DRUM+PART_808]=0;dpart_t *dp=&cur_pat()->drum[PART_808];dp->len=16;memset(dp->hit,0,sizeof dp->hit);
  for(int s=1;s<16;s+=2)sm_set(dp->hit[D8_CP],s);
  *page_of(V_PART)=0;
  for(int turn=0;turn<4;turn++){
   CHECK(cur_page()==turn%2);render_check(choice,0);
   held_keys=1u<<BLACK_KEY[D8_CP];input();render_check(choice,1);
   held_keys=0;input();render_check(choice,2);
   enc_acc[EN_SELECT]=1;ui_input_only();
  }
 }
 fprintf(stderr,"KIT_SECTION_STATES=%u failures=%u checks=%u\n",states,failures,checks);
 CHECK(!failures);puts("PASS CP/MA real framebuffer SOUND/KIT page association, both palettes, physical navigation/held/released and full step lane");return 0;
}
