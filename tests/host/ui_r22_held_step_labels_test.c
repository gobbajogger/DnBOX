/* SPDX-License-Identifier: GPL-3.0-only */
/* Constructed held-step display must name the controls physical knobs edit. */
#define SMC_UI_MAIN unused_held_label_smc_main
#include "smc_ui_test.c"
int main(void){
 setup(V_SEQ,PART_303A);ui.held_step=3;ui.touched=-1;ui.msg_until=0;
 draw_main();draw_knobs();draw_header();
 fprintf(stderr,"HELD_STATE view%d part%d held%d\n",ui.view,ui.part,ui.held_step);shot("held-step-labels");
 uint16_t actual[240*240];memcpy(actual,fb,sizeof actual);
 const char *names[]={"NOTE","GATE","ACCENT","SLIDE"};
 for(int i=0;i<4;i++){
  int cx=34+i*57;cv_begin(240,69,C_BLACK);text_c(cx,0,&FONT_T,names[i],C_WHITE);unsigned ink=0;
  if(i==0){int best=0,bx=0,by=0,n=0;for(int y=0;y<FONT_T.h;y++)for(int x=6;x<63;x++)n+=cv_px[y*240+x]==swap16(C_WHITE);
   for(int dy=-4;dy<=4;dy++)for(int dx=-3;dx<=3;dx++){int score=0;for(int y=0;y<FONT_T.h;y++)for(int x=6;x<63;x++)if(cv_px[y*240+x]==swap16(C_WHITE))score+=actual[(179+y+dy)*240+x+dx]==swap16(SOUND_INK);if(score>best){best=score;bx=dx;by=dy;}}
   fprintf(stderr,"LABEL_SEARCH matched%d/%d dx%d dy%d\n",best,n,bx,by);}
  for(int y=0;y<FONT_T.h;y++)for(int x=6+i*57;x<63+i*57;x++)if(cv_px[y*240+x]==swap16(C_WHITE)){
   ink++;if(actual[(179+y)*240+x]!=swap16(SOUND_INK))fprintf(stderr,"CAPTION_DIFF cell%d x%d y%d color%x expected%x\n",i,x,179+y,actual[(179+y)*240+x],swap16(SOUND_INK));CHECK(actual[(179+y)*240+x]==swap16(SOUND_INK));
  }CHECK(ink>0);
 }
 puts("Actual held-step knob captions match NOTE/GATE/ACCENT/SLIDE binding PASS");return 0;
}
