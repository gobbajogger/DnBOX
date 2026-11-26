#define SMC_UI_MAIN inherited_smc_main
#include "home-host-base.inc"
struct window { uint32_t x,y,w,h; uint16_t payload[X0X_LCD_MAX_DMA_BYTES/2]; };
static struct frame {
 struct window win[45];
 uint16_t panel[240*240];
 uint32_t hash[sizeof main_tileh/sizeof main_tileh[0]];
 uint32_t valid, base, count, completions;
 uint8_t view;
} reference, candidate;
static struct frame *record;
static const uint16_t *pending;
static unsigned action, action_at;
static void home_dma_finish(void){
 if(!pending)return;
 struct window *a=&record->win[record->completions];
 CHECK(!memcmp(pending,a->payload,a->w*a->h*2u));
 for(unsigned j=0;j<a->h;j++)
     memcpy(fb+(a->y+j)*240+a->x,pending+j*a->w,a->w*2u);
 record->completions++;pending=NULL;
}
static void home_dma_emit(uint32_t x,uint32_t y,uint32_t w,uint32_t h,const uint16_t *p){
 home_dma_finish();
 CHECK(y>=MAIN_Y&&y+h<=KNOB_Y&&x+w<=240&&w*h*2<=X0X_LCD_MAX_DMA_BYTES);
 CHECK(p==cv_lcd_txbuf&&record->count<45);
 struct window *a=&record->win[record->count++];
 a->x=x;a->y=y;a->w=w;a->h=h;
 memcpy(a->payload,p,w*h*2);pending=p;
 if(record->count==action_at){
    if(action==1){ui.pbank^=1;ui.dirty^=1;palette_set(NPALETTES-1);}
    if(action==2)ui_init();          /* real cache invalidation, host chooses event boundary */
    if(action==3)held_btn=1u<<B_FX;  /* next real foreground cv_lcd_input() ingresses press */
 }
 if(action==3&&record->count==action_at+1)held_btn=0;
 now_ms++;                            /* force actual once/ms callback and button release */
}
static void kick(struct frame*a,int legacy,int expect_fit){
 record=a;
 a->count=a->completions=0;
 if(expect_fit)
  for(unsigned k=240*82;k<240*138;k++)cv_px[k]=0xf18c;
 home_strip_fixture_use_full=legacy;
 home_dma_enabled=1;
 draw_main();
 lcd_sync();
 CHECK(!pending&&a->count==a->completions);
 home_dma_enabled=0;
 if(expect_fit)
  for(unsigned k=240*82;k<240*138;k++)CHECK(cv_px[k]==0xf18c);
 a->valid=cv_tile_valid[1];a->base=blit_hash[1];a->view=ui.view;
 memcpy(a->hash,main_tileh,sizeof a->hash);
 memcpy(a->panel,fb,sizeof a->panel);
}
static void eq(void){
 CHECK(reference.count==candidate.count);
 CHECK(reference.base==candidate.base&&reference.valid==candidate.valid&&reference.view==candidate.view);
 CHECK(!memcmp(reference.hash,candidate.hash,sizeof reference.hash));
 CHECK(!memcmp(reference.panel,candidate.panel,sizeof reference.panel));
 for(unsigned k=0;k<reference.count;k++){
   struct window *a=reference.win+k,*b=candidate.win+k;
   CHECK(a->x==b->x&&a->y==b->y&&a->w==b->w&&a->h==b->h);
   CHECK(!memcmp(a->payload,b->payload,a->w*a->h*2));
 }
}
static void begin(unsigned theme,unsigned bank){
 home_strip_fixture_use_full=1;
 setup(V_HOME,PART_BRK);
 CHECK(!ui.home_live&&ui.overlay==O_NONE);
 palette_set(theme);ui.pbank=(uint8_t)bank;
 ui.frame|=16u;                           /* blink phase retained in source frame */
 for(unsigned k=0;k<32;k++){
    pattern_t *p=&proj.pat[k];
    sm_set(p->drum[k&1].hit[k%NDRUM],k%16);
    sm_set(p->drum[k&1].accent,k%16);
    p->bass[k&1].step[k%16]=(bstep_t){(uint8_t)(36+k),1,(uint8_t)(55+k)};
    p->brk.steps|=1u<<(k%16);
 }
 seq.chain_a=1;seq.chain_b=4;             /* test actual chain highlight */
 memset(fb,0x37,sizeof fb);blit_hash[1]=cv_tile_valid[1]=0;
}
int main(void){
 unsigned scenes=0,dirty=0;
 for(unsigned theme=0;theme<NPALETTES;theme++){
  for(int event=0;event<=3;event++){
   action=event;action_at=event==2?25:1;  /* ui reset exactly at old row 4, just before split */
   begin(theme,theme%2);kick(&reference,1,0);
   begin(theme,theme%2);kick(&candidate,0,1);
   eq();CHECK(candidate.count==45);
   if(event==2){
     CHECK(candidate.valid==0&&candidate.base==0);
     CHECK(ui.view==V_SEQ);
     action=0;home_strip_fixture_use_full=1;
     set_view(V_HOME);ui.touched=-1;
     kick(&candidate,0,1);CHECK(candidate.count==45&&candidate.valid==1);
   }
   if(event==3)CHECK(candidate.view==V_FX); /* real press/release was serviced during DMA */
   ++scenes;
  }
 }
 action=0;
 begin(2,0);kick(&reference,1,0);
 kick(&reference,1,0);CHECK(!reference.count);
 ui.pbank=1;kick(&reference,1,0);dirty=reference.count;
 begin(2,0);kick(&candidate,0,1);
 kick(&candidate,0,1);CHECK(!candidate.count);
 ui.pbank=1;kick(&candidate,0,1);eq();CHECK(dirty>0&&dirty<45);
 printf("Whole draw_main entry: %u cold/callback full RGB565 frames and exact windows/hashes, 45 cold/0 stable/%u dirty. Real key ingress and reset cache preservation, fenced pending DMA, 82-row canaries. Home snapshot %zu host B.\n",scenes,dirty,sizeof(home_snapshot));
}
