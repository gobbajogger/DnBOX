#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
static uint16_t fb[240u*240u];
static void lcd_sync(void) {}
static void lcd_blit(uint32_t x,uint32_t y,uint32_t w,uint32_t h,const uint16_t *buf)
{
    assert(x+w<=240u && y+h<=240u);
    for(unsigned i=0;i<h;i++)memcpy(fb+(y+i)*240u+x,buf+i*w,2*w);
}
#include "gfx.c"
int main(void)
{
    uint32_t rng=49u;
    for(unsigned theme=0;theme<6;theme++) for(unsigned n=0;n<4;n++) {
        palette_set((uint8_t)theme);
        uint8_t custom[BOOT_REC_MAX]={BOOT_KIND_CUSTOM,BOOT_F_PIC,5,5};
        memcpy(custom+4,"DnBOX",5);memcpy(custom+16,"STILL",5);
        boot_p16(custom+40,240);boot_p16(custom+42,96);
        for(unsigned i=BOOT_PAY_FIXED;i<sizeof custom;i++) {
            rng=rng*1664525u+1013904223u; custom[i]=(uint8_t)(rng>>24u);
        }
        if(n==1) memset(custom+BOOT_PAY_FIXED,255,sizeof custom-BOOT_PAY_FIXED);
        if(n==2) memset(custom+BOOT_PAY_FIXED,0,sizeof custom-BOOT_PAY_FIXED);
        memset(fb,0xa5,sizeof fb);
        draw_boot_screen(n?custom:NULL);
        assert(fwrite(fb,1,sizeof fb,stdout)==sizeof fb);
    }
}
