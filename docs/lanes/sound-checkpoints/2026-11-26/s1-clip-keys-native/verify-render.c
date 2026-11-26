#include "actual-clip-key-host.inc"
#include "app/lcd_tiles.h"
#include <assert.h>
#define MAXWIN 50u
typedef struct { uint32_t x,y,w,h; uint16_t pixels[X0X_LCD_MAX_DMA_BYTES/2]; } window;
typedef struct { window q[MAXWIN]; uint32_t count,hashes[9*X0X_LCD_TILE_COLS]; } proof;
static proof ref,trial,*result;
static uint16_t cv_lcd_txbuf[X0X_LCD_MAX_DMA_BYTES/2u];
static uint32_t blit_hash[4],*main_tileh;
static uint8_t cv_tile_valid[4];
typedef struct {uint32_t x,y;} cv_lcd_pos_t;
static const uint16_t *pending;
static unsigned complete, inject;
static void lcd_blit(uint32_t x,uint32_t y,uint32_t w,uint32_t h,const uint16_t *px){
 assert(!pending&&result->count<MAXWIN&&w*h*2u<=X0X_LCD_MAX_DMA_BYTES&&px==cv_lcd_txbuf);
 window *q=&result->q[result->count++];q->x=x;q->y=y;q->w=w;q->h=h;
 memcpy(q->pixels,px,w*h*2u);pending=px;
}
static void lcd_sync(void){
 if(!pending)return;
 const window *q=&result->q[complete++];
 assert(!memcmp(q->pixels,pending,q->w*q->h*2u));pending=0;
}
static void cv_lcd_maybe_input(void){
 if(inject && result->count==inject){
    for(unsigned i=0;i<SMK_APC40_CLIP_COUNT;i++){apc_led_velocity[i]^=63u;apc_cell_flags[i]^=SMK_APC40_CELL_QUEUED;}
    ck.pg=0;proj.set.ck_root=4;palette_set(5);blit_hash[1]=cv_tile_valid[1]=0;
 }
}
#include "commit.inc"
static void initialize(proof *p){
 memset(p,0,sizeof *p);result=p;main_tileh=p->hashes;
 complete=pending!=0;assert(!pending);blit_hash[1]=cv_tile_valid[1]=0;
}
static void baseline(void){
 assert(smk_clip_lcd_draw(240,138));
 cv_lcd_pos_t pos={0,MAIN_Y};
 x0x_lcd_tile_commit(cv_px,138,main_tileh,1,cv_lcd_emit,&pos);lcd_sync();
}
static void pair_case(unsigned page,unsigned field,unsigned palette,unsigned mutation){
 fm1_ms+=2100;
 ck.pg=page ? page|(CK_PAGE_FIELDS[page-1u][field]<<4):0u;
 proj.set.ck_root=10;proj.set.ck_scale=3;proj.set.ck_rate=4;proj.set.ck_gate=2;ck.on=7u;
 apc_cols=8;apc_rows=5;apc_origin_col=(uint8_t)(field%3u);apc_origin_row=0;apc_view_has_clips=1;
 for(unsigned i=0;i<SMK_APC40_CLIP_COUNT;i++){apc_led_velocity[i]=(uint8_t)((i*11+field)&127u);apc_cell_flags[i]=(i&1)?SMK_APC40_CELL_QUEUED:0;}
 palette_set(palette);smk_clip_lcd_force();
 inject=0;initialize(&ref);baseline();
 const uint8_t pv=apc_led_velocity[0];const uint8_t saved_pg=ck.pg;
 inject=mutation;initialize(&trial);
 for(unsigned k=240u*82;k<240u*138;k++)cv_px[k]=0xe071u;
 fm1_ms+=2100;smk_clip_lcd_force();assert(smk_clip_render_strips());lcd_sync();
 assert(trial.count==ref.count && complete==trial.count);
 assert(!memcmp(trial.hashes,ref.hashes,sizeof ref.hashes));
 assert(!memcmp(trial.q,ref.q,sizeof ref.q));
 for(unsigned k=240u*82;k<240u*138;k++)assert(cv_px[k]==0xe071u);
 if(mutation){assert(blit_hash[1]==0&&cv_tile_valid[1]==0);assert(apc_led_velocity[0]!=(unsigned)pv);}
 else {assert(ck.pg==saved_pg&&apc_led_velocity[0]==pv);}
}
static void waiting(unsigned p){
    palette_set(p);inject=0;initialize(&ref);
    cv_begin(240,138,C_BLACK);cv_band(0,138);
    text_c(120,34,&FONT_B,"WAITING FOR",C_WHITE);
    text_c(120,56,&FONT_B,"LOOPY PRO",C_WHITE);
    text_c(120,86,&FONT_XS,"USB OR BLUETOOTH",C_GRAY);
    cv_lcd_pos_t pos={0,MAIN_Y};x0x_lcd_tile_commit(cv_px,138,main_tileh,1,cv_lcd_emit,&pos);lcd_sync();
    initialize(&trial);inject=25;
    const uint16_t ink[]={C_WHITE,C_GRAY};
    commit_full_strips(C_BLACK,paint_clip_wait,ink);lcd_sync();
    assert(complete==trial.count&&trial.count==ref.count);
    assert(!memcmp(trial.q,ref.q,sizeof ref.q));
    assert(!memcmp(trial.hashes,ref.hashes,sizeof ref.hashes));
    assert(!blit_hash[1]&&!cv_tile_valid[1]);
}
static void throttles(void) {
    inject=0;initialize(&trial);
    fm1_ms+=1001;ck.pg=0;smk_clip_lcd_force();
    assert(smk_clip_render_strips());lcd_sync();unsigned before=trial.count;
    for(unsigned i=0;i<SMK_APC40_CLIP_COUNT;i++)apc_led_velocity[i]^=23u;
    assert(smk_clip_render_strips());lcd_sync();assert(trial.count==before);
    fm1_ms+=SMK_CLIP_LCD_MIN_MS;
    trial.count=complete=0;assert(smk_clip_render_strips());lcd_sync();assert(trial.count>0);
    trial.count=complete=0;assert(smk_clip_render_strips());lcd_sync();assert(!trial.count);
    ck.pg=1;assert(smk_clip_render_strips());lcd_sync();assert(!trial.count);
    fm1_ms+=SMK_CLIP_LCD_MIN_MS;
    assert(smk_clip_render_strips());lcd_sync();assert(trial.count>0);
    ck.pg=0;trial.count=complete=0;
    assert(smk_clip_render_strips());lcd_sync();assert(trial.count>0);
}
int main(void){
 unsigned n=0;
 for(unsigned p=0;p<NPALETTES;p++)for(unsigned page=0;page<4;++page)
  for(unsigned field=0;field<(page==0?8u:page==3?9u:4u);++field)
    for(unsigned k=0;k<2;++k){pair_case(page,field,p,k?27u:0u);++n;}
 for(unsigned p=0;p<NPALETTES;++p)waiting(p);throttles();
 puts("6 WAITING palettes byte-exact; frame throttles and immediate Grid return exercised");
 printf("%u full target caller Clip/Keys host comparisons: every DMA window/payload, dirty hash and >82-row canary byte-exact; simulated callback invalidation retained\n",n);
}
