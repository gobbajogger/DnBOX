/* SPDX-License-Identifier: GPL-3.0-only */
/* Existing real status messages must retain every native-font glyph. */
#define SMC_UI_MAIN unused_r024_toast_main
#include "smc_ui_test.c"
static void expected_wrap(const char *msg,char *a,char *b){
 put_s(a,msg);b[0]=0;int last=-1;
 for(int n=0;a[n];n++){char save=a[n+1];a[n+1]=0;int width=tw(&FONT_T,a);a[n+1]=save;if(width>117)break;if(a[n]==' ')last=n;}
 CHECK(last>0);a[last]=0;put_s(b,msg+last+1);CHECK(tw(&FONT_T,a)<=117);CHECK(tw(&FONT_T,b)<=117);
}
static void line_exact(const uint16_t *actual,const char *s,int sy){
 /* Exact native AA against the actual rounded header including its border.
  * Earlier flat-face observer failed at the rounded bottom edge, retained. */
 cv_begin(240,20,C_BLACK);night_panel(0,0,240,HDR_H,0);uint16_t background[240*20];memcpy(background,cv_px,sizeof background);cv_text(4,sy,&FONT_T,s,SOUND_INK);unsigned ink=0;
 for(int y=sy;y<sy+FONT_T.h&&y<20;y++)for(int x=4;x<125;x++)if(cv_px[y*240+x]!=background[y*240+x]){ink++;if(actual[y*240+x]!=cv_px[y*240+x])fprintf(stderr,"TOAST_DIFF text=%s x%d y%d actual%x expected%x fonth%d\n",s,x,y,actual[y*240+x],cv_px[y*240+x],FONT_T.h);CHECK(actual[y*240+x]==cv_px[y*240+x]);}
 CHECK(ink>0);
}
int main(void){
 const char *messages[]={"MOTION FULL: NOT ALL PASTED","PROJECT RESET: SAMPLES KEPT"};
 for(unsigned i=0;i<2;i++){
  setup(V_PART,PART_909);put_s(ui.msg,messages[i]);ui.msg_until=now_ms+1000;draw_header();shot("r024-complete-toast");uint16_t actual[240*20];memcpy(actual,fb,sizeof actual);char a[sizeof ui.msg],b[sizeof ui.msg];expected_wrap(messages[i],a,b);line_exact(actual,a,1);line_exact(actual,b,10);
 }
 puts("R024 complete existing status-message glyphs PASS");return 0;
}
