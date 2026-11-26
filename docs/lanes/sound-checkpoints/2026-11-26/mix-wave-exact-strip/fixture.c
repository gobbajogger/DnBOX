#define SMC_UI_MAIN inherited_smc_main
#include "home-host-base.inc"
struct window { uint32_t x,y,w,h; uint16_t payload[X0X_LCD_MAX_DMA_BYTES/2]; };
static struct frame {
 struct window win[72];
 uint16_t panel[240*240];
 uint32_t hash[sizeof main_tileh/sizeof main_tileh[0]];
 uint32_t valid, base, count, completions;
 uint8_t view;
} reference, candidate;
static struct frame *record;
static const uint16_t *pending;
static unsigned action, action_at;
void fixture_gr(float);
void fixture_guard(int);
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
 CHECK(p==cv_lcd_txbuf&&record->count<72);
 struct window *a=&record->win[record->count++];
 a->x=x;a->y=y;a->w=w;a->h=h;
 memcpy(a->payload,p,w*h*2);pending=p;
 if(record->count==action_at){
    if(action==1){
      for(unsigned i=0;i<SCOPE_N;i++)eng_scope[i]=(int16_t)(16384-i*33);
      eng_scope_w+=17; cpu_pct=99; fixture_gr(23.5f); fixture_guard(1);
      palette_set(NPALETTES-1);
    }
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

static unsigned epoch=5000;
static void begin(unsigned theme,unsigned mode){
 home_strip_fixture_use_full=1;
 setup(V_MIX,PART_BRK);
 CHECK(ui.view==V_MIX&&ui.overlay==O_NONE);
 ui.page[V_MIX][0]=MIX_FIRST_KIT_PAGE+NKIT+1;
 build_pages();
 CHECK(drum_mix_page()<0);
 palette_set(theme); cpu_pct=mode==2?91:42;
 fixture_guard(mode==2); fixture_gr(mode==0?0.25f:12.7f);
 eng_scope_w=17*mode;
 for(unsigned i=0;i<SCOPE_N;i++) eng_scope[i]=mode?((int)i*3003%34019)-16000:0;
 now_ms=(epoch+=2000); ui.touched=-1;
 memset(fb,0x57,sizeof fb);blit_hash[1]=cv_tile_valid[1]=0;
}
int main(void){
 unsigned frames=0;
 for(unsigned theme=0;theme<NPALETTES;theme++)for(unsigned mode=0;mode<3;mode++){
   for(unsigned event=0;event<=3;event++){
     action=event;action_at=event==2?25:1;
     begin(theme,mode);kick(&reference,1,0);
     begin(theme,mode);kick(&candidate,0,1);
     eq();CHECK(candidate.count==45);
     if(action==2)CHECK(candidate.valid==0&&candidate.base==0);
     if(action==3)CHECK(candidate.view==V_FX);
     frames++;
   }
 }
 action=0;
 begin(2,1);kick(&reference,1,0);kick(&reference,1,0);CHECK(!reference.count);
 cpu_pct=68;kick(&reference,1,0);
 begin(2,1);kick(&candidate,0,1);kick(&candidate,0,1);CHECK(!candidate.count);
 cpu_pct=68;kick(&candidate,0,1);eq();CHECK(candidate.count>0&&candidate.count<45);
 printf("WAVE: %u baseline/strip exact pairs, 45 cold, 0 stable, %u dirty load change, %zu bytes snapshot.\n",frames,candidate.count,sizeof(mix_wave_view));
}
