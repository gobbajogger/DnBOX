#include <assert.h>
#define SMC_UI_MAIN inherited_smc_main
#include "smc_ui_test.c"
enum { HCUT=80 };
static void render_home_strip(int top,int size,const home_snapshot *h){
 cv_begin(240,size,h->color[5]);
 for(int band=0;band<2;++band){
     int a=band*BAND_H-top;
     cv_oy=a;cv_y0=a>0?a:0;cv_y1=a+BAND_H<size?a+BAND_H:size;
     if(cv_y0<cv_y1)draw_home_snapshot(band,h);
 }
}
static unsigned hwrites,hit,inject,cold_windows,dirty_windows;
static uint16_t main_reference[240*138];
static uint16_t payload[X0X_LCD_MAX_DMA_BYTES/2];
static uint16_t payload_copy[X0X_LCD_MAX_DMA_BYTES/2];
static unsigned xp,yp,wp,hp;
static uint32_t coords[45], reference_coords[45];
static void finish(void) {
 if(!wp)return;
 CHECK(!memcmp(payload,payload_copy,wp*hp*2));
 for(unsigned r=0;r<hp;r++)memcpy(fb+(MAIN_Y+yp+r)*240+xp,payload+r*wp,wp*2);
 wp=0;
}
static void cb(void *u,const uint16_t *data,uint32_t stride,uint32_t x,uint32_t y,uint32_t w,uint32_t h){
 finish(); x0x_lcd_tile_pack(payload,data,stride,x,y,w,h);
 memcpy(payload_copy,payload,w*h*2);xp=x;yp=y+*(unsigned *)u;wp=w;hp=h;
 if(inject && hwrites==0){ui.pbank^=1;ui.dirty^=1;palette_set(NPALETTES-1);hit++;}
 CHECK(hwrites<45);coords[hwrites++]=x | ((y+*(unsigned *)u)<<8) | (w<<16) | (h<<24);
}
static void home_fixture_frame(int split,int force,int coherent,uint32_t *hash){
 unsigned o=0;hwrites=0;
 if(split) {
  home_snapshot snap; take_home(&snap);
  for(unsigned i=240*82;i<240*138;i++)cv_px[i]=0xd76a;
  for(;o<138;o+=HCUT) {
    const unsigned n=138-o<HCUT?138-o:HCUT;
    finish();                /* same buffer ownership fence as cv_begin() platform contract */
    if(!coherent && o)take_home(&snap);
    render_home_strip(o,n,&snap);
    x0x_lcd_tile_commit(cv_px,n,hash+(o/16)*15,force,cb,&o);
    for(unsigned i=240*82;i<240*138;i++)CHECK(cv_px[i]==0xd76a);
  }
 } else {
    cv_begin(240,138,C_BLACK);
    for(int b=0;b<2;b++){cv_band(b*69,69);draw_home(b);}
    x0x_lcd_tile_commit(cv_px,138,hash,force,cb,&o);
 }
 finish();
}
int main(void){
 uint32_t a[135]={0},b[135]={0};
 setup(V_HOME,PART_BRK);
 CHECK(!ui.home_live && ui.overlay==O_NONE);
 const unsigned original_dirty=ui.dirty,origbank=ui.pbank,theme=proj.set.palette;
 inject=1;home_fixture_frame(0,1,1,a);
 memcpy(reference_coords,coords,hwrites*sizeof *coords);
 cold_windows=hwrites;CHECK(cold_windows==45);memcpy(main_reference,fb+MAIN_Y*240,sizeof main_reference);
 ui.pbank=origbank; ui.dirty=original_dirty;palette_set(theme);
 home_fixture_frame(1,1,1,b);
 CHECK(!memcmp(reference_coords,coords,hwrites*sizeof *coords));
 CHECK(hit==2 && hwrites==cold_windows && !memcmp(a,b,sizeof a));
 CHECK(!memcmp(main_reference,fb+MAIN_Y*240,sizeof main_reference));
 ui.pbank=origbank;ui.dirty=original_dirty;palette_set(theme);
 inject=0;home_fixture_frame(1,0,1,b);CHECK(hwrites==0);
 ui.pbank^=1;
 home_fixture_frame(0,0,1,a);dirty_windows=hwrites;memcpy(reference_coords,coords,hwrites*sizeof *coords);memcpy(main_reference,fb+MAIN_Y*240,sizeof main_reference);
 home_fixture_frame(1,0,1,b);
 CHECK(!memcmp(reference_coords,coords,hwrites*sizeof *coords));
 CHECK(dirty_windows && dirty_windows==hwrites && !memcmp(a,b,sizeof a));
 CHECK(!memcmp(main_reference,fb+MAIN_Y*240,sizeof main_reference));
 ui.pbank=origbank;ui.dirty=original_dirty;palette_set(theme);
 inject=1;home_fixture_frame(0,1,1,a);memcpy(main_reference,fb+MAIN_Y*240,sizeof main_reference);
 ui.pbank=origbank;ui.dirty=original_dirty;palette_set(theme);
 home_fixture_frame(1,1,0,b); CHECK(memcmp(main_reference,fb+MAIN_Y*240,sizeof main_reference));
 printf("Actual default Home: 33120 exact pixels, %u cold / 0 stable / %u bank changed DMA windows, snapshot sizeof %zu; negative-control tears\n",cold_windows,dirty_windows,sizeof(home_snapshot));
 return 0;
}
