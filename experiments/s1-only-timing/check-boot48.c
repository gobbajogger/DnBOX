/* Compile real gfx.c and source-derived candidate, with pending mock DMA. */
#define X0X_HOST 1
#define X0X_UI_SMALL_CANVAS 1
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static void lcd_sync(void);
static void lcd_blit(uint32_t,uint32_t,uint32_t,uint32_t,const uint16_t *);
#include "gfx.c"
#include "boot-candidate.inc"
enum { W=240,H=240 };
static uint16_t panel[W*H], pixel_reference[W*H], awaiting[W*120];
static const uint16_t *pending;
static uint32_t x,y,w,h,wins,closes,largest,bytes;
static int sliced;
static void lcd_sync(void){
 if(!pending)return;
 assert(!memcmp(pending,awaiting,w*h*2));
 for(unsigned i=0;i<h;++i)memcpy(panel+(y+i)*W+x,pending+i*w,w*2);
 pending=0;++closes;
}
static void lcd_blit(uint32_t x_,uint32_t y_,uint32_t w_,uint32_t h_,const uint16_t *p){
 lcd_sync();assert(w_&&h_&&x_+w_<=W&&y_+h_<=H&&p==cv_px);
 if(sliced)assert(w_*h_<=240u*48u);
 else assert(w_*h_<=240u*60u);
 x=x_;y=y_;w=w_;h=h_;pending=p;
 memcpy(awaiting,p,w*h*2);++wins;uint32_t size=w*h*2;
 bytes+=size;if(size>largest)largest=size;
}
static void contrast(const uint8_t *p){
 wins=closes=largest=bytes=0;memset(panel,0x5a,sizeof panel);sliced=0;
 draw_boot_screen(p);lcd_sync();
 memcpy(pixel_reference,panel,sizeof panel);unsigned k=wins,n=largest,b=bytes;
 assert(wins==closes&&n==28800);
 wins=closes=largest=bytes=0;sliced=1;memset(panel,0x59,sizeof panel);
 for(unsigned i=240*48;i<CV_MAX;++i)cv_px[i]=0x4db2;
 draw_boot_screen_sliced(p);lcd_sync();
 assert(wins==closes&&wins==k+1&&largest==23040&&bytes==b);
 assert(!memcmp(panel,pixel_reference,sizeof panel));
 for(unsigned i=240*48;i<CV_MAX;++i)assert(cv_px[i]==0x4db2);
}
int main(void){
 uint8_t p[BOOT_REC_MAX]={0};unsigned cases=0;
 for(unsigned fonts=0;fonts<2;++fonts)for(unsigned theme=0;theme<NPALETTES;++theme){
  gfx_xdata_ok=fonts;
  palette_set(theme);contrast(NULL);++cases;
  memset(p,0,sizeof p);p[0]=BOOT_KIND_FACTORY;
  assert(boot_payload_ok(p,BOOT_PAY_FIXED));contrast(p);++cases;
  p[0]=BOOT_KIND_CUSTOM;p[2]=12;p[3]=21;
  memcpy(p+4,"JunO_808-SM!",12);memcpy(p+16,"Display + MIDI / boot",21);
  assert(boot_payload_ok(p,BOOT_PAY_FIXED));contrast(p);++cases;
  p[1]=BOOT_F_PIC;boot_p16(p+40,BOOT_PIC_W);boot_p16(p+42,BOOT_PIC_H);
  assert(boot_payload_ok(p,BOOT_REC_MAX));
  for(unsigned style=0;style<4;++style){
   for(unsigned i=0;i<BOOT_PIC_BYTES;++i)
     p[BOOT_PAY_FIXED+i]=style==0?0:style==1?255:style==2?((i^(i/30))&1?0x55:0xaa):(uint8_t)(i*175u+(i>>5));
   contrast(p);++cases;
  }
 }
 printf("Cold boot: %u palette/validated payload cases, all 57,600 RGB565 pixels identical; pixel bytes unchanged; largest DMA 28,800 to 23,040 B, one extra boot-only window. Payload protected through DMA completion; above 48 rows untouched.\n",cases);
}
