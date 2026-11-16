/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN unused_fx_caption_smc_main
#include "smc_ui_test.c"
/* Count actual opaque native-font shapes in final framebuffer, not source text. */
static int word_count(const uint16_t *screen,const char *word){
 const felucca_font_t *fonts[]={&FONT_T,&FONT_XS};int count=0,xy[12][2];
 for(unsigned f=0;f<2;f++){
  const felucca_font_t *font=fonts[f];int w=tw(font,word),h=font->h;
  cv_begin(240,69,C_BLACK);cv_text(0,0,font,word,C_WHITE);
  int mx[256],my[256],n=0;for(int y=0;y<h;y++)for(int x=0;x<w;x++)if(cv_px[y*240+x]==swap16(C_WHITE)&&n<256){mx[n]=x;my[n++]=y;}
  CHECK(n>12);
  for(int y=20;y<240-h;y++)for(int x=0;x<240-w;x++){
   int good=0;for(int i=0;i<n;i++){uint16_t p=screen[(y+my[i])*240+x+mx[i]];good+=p==swap16(C_GRAY)||p==swap16(SOUND_GRAY)||p==swap16(SOUND_INK)||p==swap16(C_WHITE);}
   if(good*100<n*98)continue;
   int near=0;for(int i=0;i<count&&i<12;i++)if(abs(x-xy[i][0])<=3&&abs(y-xy[i][1])<=3)near=1;
   if(!near){if(count<12){xy[count][0]=x;xy[count][1]=y;}count++;fprintf(stderr,"VISIBLE_WORD %s x%d y%d font%u\n",word,x,y,f);}
  }
 }
 return count;
}
int main(void){
 setup(V_FX,PART_909);*page_of(V_FX)=1;build_pages();ui.msg_until=0;ui.touched=-1;ui.dirty=1;for(int i=0;i<4;i++)blit_hash[i]=0;ui_frame();
 uint16_t actual[240*240];memcpy(actual,fb,sizeof actual);int count=word_count(actual,"FEEDBACK");fprintf(stderr,"FEEDBACK_COUNT %d\n",count);CHECK(count==1);
 puts("ActualFX feedback has exactlyone painted controlcaption PASS");return 0;
}
