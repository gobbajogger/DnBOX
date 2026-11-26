#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "app/smk_apc40.h"
#include "app/apc40_lcd565.h"
#include "app/apc40_lcd_idle.h"
#include "app/smk_clip_colour.h"
#include "app/lcd_tiles.h"
#define W 240u
#define H 138u
#define CUT 80u
#define C_WHITE 0xffffu
#define C_BLACK 0u
#define C_DIM 0x4208u
static uint16_t cv_px[W*(STRIP?CUT:H)];
static uint32_t first_y,rows,hashes[9u*X0X_LCD_TILE_COLS];
static uint64_t digest=14695981039346656037ull;
static unsigned windows;
static FILE *raw;
static uint16_t swap16(uint32_t x){return (uint16_t)((x>>8)|(x<<8));}
static void cv_rect(int x,int y,int w,int h,uint16_t px){
    if(w<=0||h<=0)return;
    for(int j=y;j<y+h;j++)if(j>=(int)first_y&&j<(int)(first_y+rows))
      for(int i=x;i<x+w;i++)if(i>=0&&i<(int)W)
        cv_px[(j-first_y)*W+i]=swap16(px);
}
static void word(uint32_t value){
  if(fwrite(&value,1,sizeof value,raw)!=sizeof value)__builtin_trap();
  for(unsigned i=0;i<4;i++)digest=(digest^((value>>(i*8))&255u))*1099511628211ull;
}
static void emit(void *ctx,const uint16_t *px,uint32_t stride,uint32_t x,uint32_t y,uint32_t w,uint32_t h){
    (void)ctx;uint16_t tx[X0X_LCD_MAX_DMA_BYTES/2u];
    x0x_lcd_tile_pack(tx,px,stride,x,y,w,h);
    if(w*h*2u>X0X_LCD_MAX_DMA_BYTES)__builtin_trap();
    word(x);word(y+first_y);word(w);word(h);++windows;
    for(unsigned i=0;i<w*h;i++)word(tx[i]);
}
typedef struct {
    uint8_t cols,nrows,ox,oy,has_clips;
    uint8_t vel[SMK_APC40_CLIP_COUNT], flags[SMK_APC40_CLIP_COUNT], in_view[SMK_APC40_CLIP_COUNT];
} state;
static void paint(const state *s,uint32_t w,uint32_t h){
    uint8_t apc_cols=s->cols,apc_rows=s->nrows,apc_origin_col=s->ox,apc_origin_row=s->oy,apc_view_has_clips=s->has_clips;
    const uint8_t *in_view=s->in_view,*apc_led_velocity=s->vel,*apc_cell_flags=s->flags;
#include "clip-painter.inc"
}
static void frame(state live,int mutate,int force){
    const state frozen=live;
    for(first_y=0;first_y<H;first_y+=rows){
      rows=STRIP?(H-first_y<CUT?H-first_y:CUT):H;
      memset(cv_px,0,rows*W*sizeof(uint16_t));paint(&frozen,W,H);
      x0x_lcd_tile_commit(cv_px,rows,hashes+(first_y/16u)*X0X_LCD_TILE_COLS,force,emit,0);
      if(mutate){
        // Between physical paints simulate USB entry publication. Expected
        // next frame can see it; this frame has a single frozen input version.
        volatile uint8_t *p=live.vel;
        for(unsigned i=0;i<SMK_APC40_CLIP_COUNT;i++)p[i]=(uint8_t)(p[i]^63u);
      }
    }
    for(unsigned i=0;i<sizeof hashes/sizeof hashes[0];i++)word(hashes[i]);
}
int main(int argc,char **argv){
  if(argc!=2 || !(raw=fopen(argv[1],"wb")))return 2;
  for(unsigned c=0;c<100;c++){
    state s={.cols=(uint8_t)(1+c%16),.nrows=(uint8_t)(1+c%8),.ox=(uint8_t)(c%3),.oy=(uint8_t)(c%2),.has_clips=(uint8_t)(c%5!=0)};
    memset(hashes,0,sizeof hashes);
    for(unsigned i=0;i<SMK_APC40_CLIP_COUNT;i++){
      s.vel[i]=(uint8_t)((i*11+c*13)&127u);
      s.flags[i]=(i+c)%3==0?SMK_APC40_CELL_QUEUED:0;
      s.in_view[i]=(uint8_t)((i+c)%4<2);
    }
    frame(s,1,1); unsigned n=windows;
    frame(s,0,0); if(windows!=n)return 1;
    s.vel[(c*11)%SMK_APC40_CLIP_COUNT]^=23u;frame(s,1,0);
  }
  printf("100 Clip Grid cold/stable/edit sequences: ordered window bytes + tile hashes %016llx, count %u\n",(unsigned long long)digest,windows);
  printf("Render scratch %zu B, frozen data %zu B, DMA staging unchanged\n",sizeof cv_px,sizeof(state));
  return fclose(raw)!=0;
}
