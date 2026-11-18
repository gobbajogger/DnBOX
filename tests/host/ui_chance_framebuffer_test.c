/* Actual native UI dispatch and framebuffer against the hash-pinned Python oracle. */
#define SMC_UI_MAIN inherited_chance_fixture
#include "smc_ui_test.c"
#if !X0X_GFINAL_CHANCE_PROBE
#error Actual default host UI must enable configured CHANCE rendering
#endif
static uint16_t oracle_pixel(FILE *file){int lo=fgetc(file),hi=fgetc(file);CHECK(lo>=0&&hi>=0);return (uint16_t)(lo|(hi<<8));}
int main(void){
 const char *path=getenv("CHANCE_ORACLE");CHECK(path);FILE *oracle=fopen(path,"rb");CHECK(oracle);
 static const struct {int view,page,slot,param;const char *name;} binding[]={
  {V_PART,0,2,BRK_BCHANCE,"b-chance"},
  {V_SEQ,1,2,BRK_ROLL,"roll"},
  {V_SEQ,2,0,BRK_R2,"r2"},{V_SEQ,2,1,BRK_R3,"r3"},
  {V_SEQ,2,2,BRK_R4,"r4"},{V_SEQ,2,3,BRK_R8,"r8"}
 };
 static const int cx[4]={32,90,149,207};
 for(unsigned k=0;k<sizeof binding/sizeof binding[0];k++){
  setup(binding[k].view,PART_BRK);ui.page[ui.view][PART_BRK]=(uint8_t)binding[k].page;build_pages();
  pref_t ref=PR(R_BRKSET,0,0,binding[k].param);int slot=binding[k].slot,x0=cx[slot]-20;
  CHECK(pg.r[cur_page()][slot].kind==ref.kind&&pg.r[cur_page()][slot].c==ref.c);
  rewind(oracle);
  for(int v=0;v<=100;v++){
   pref_set(ref,v);draw_header();draw_main();draw_knobs();CHECK(chance_knob_status[slot]==CHANCE_DRAWN);CHECK(pref_get(ref)==v);
   /* Canonical dark background quantizes to the same shared panel face:
    * no individually shaded glyph tile may reappear on the common plane. */
   CHECK(!ui_theme_light && SOUND_FACE==0x10c3);
   for(int y=171;y<195;y++) {
    CHECK(fb[y*240+x0-1]==swap16(SOUND_FACE));
    CHECK(fb[y*240+x0+40]==swap16(SOUND_FACE));
    for(int x=x0;x<x0+40;x++)CHECK(fb[y*240+x]==swap16(oracle_pixel(oracle)));
   }
   if(v==0||v==50||v==100){char name[64];snprintf(name,sizeof name,"break-canonical-%s-%d",binding[k].name,v);shot(name);}
  }
  CHECK(fgetc(oracle)==EOF);
 }
 fclose(oracle);
 setup(V_PART,PART_BRK);pref_t ref=PR(R_BRKSET,0,0,BRK_BCHANCE);pref_set(ref,100);
 const x0x_param_t *d=pref_desc(ref);CHECK(d&&d->max==100);
 for(int pos=0;pos<4;pos++){
  cv_begin(240,KNOB_H,0x1234);cv_oy=-13;
  CHECK(chance_native_knob(ref,d,50,cx[pos])==CHANCE_DRAWN);
  for(int y=0;y<KNOB_H;y++)for(int x=0;x<240;x++)
   if(x<cx[pos]-20||x>=cx[pos]+20||y<13||y>=37)CHECK(cv_px[y*240+x]==swap16(0x1234));
 }
 x0x_param_t wrong=*d;wrong.max=99;
 cv_begin(240,KNOB_H,0x1234);
 CHECK(chance_native_knob(ref,&wrong,50,149)==CHANCE_NATIVE_LAW_PENDING);
 CHECK(chance_native_knob(ref,0,50,149)==CHANCE_NATIVE_LAW_PENDING);
 CHECK(chance_native_knob(PR(R_BRKSET,1,0,BRK_BCHANCE),d,50,149)==CHANCE_NOT_TARGET);
 CHECK(chance_native_knob(PR(R_BRKSET,0,1,BRK_BCHANCE),d,50,149)==CHANCE_NOT_TARGET);
 CHECK(chance_native_knob(ref,d,-1,149)==CHANCE_NATIVE_LAW_PENDING);
 CHECK(chance_native_knob(ref,d,101,149)==CHANCE_NATIVE_LAW_PENDING);
 CHECK(chance_native_knob(PR(R_BRKSLOT,0,0,0),d,50,149)==CHANCE_NOT_TARGET);
 CHECK(chance_native_knob(PR(R_BRKSET,0,0,BRK_ANCHOR),d,50,149)==CHANCE_NOT_TARGET);
 CHECK(chance_native_knob(PR(R_BRKSET,0,0,BRK_COMPLEX),d,50,149)==CHANCE_NOT_TARGET);
 CHECK(chance_native_knob(PR(R_BRKSET,0,0,BRK_FILL),d,50,149)==CHANCE_NOT_TARGET);
 for(int i=0;i<240*KNOB_H;i++)CHECK(cv_px[i]==swap16(0x1234));
 proj.set.palette=NPALETTES-1;palette_set(proj.set.palette);draw_header();draw_main();draw_knobs();
 CHECK(ui_theme_light);CHECK(chance_knob_status[2]==CHANCE_LIGHT_PENDING);CHECK(pref_get(ref)==100);
 shot("break-chance-light-pending");
 setup(V_SEQ,PART_BRK);ui.page[V_SEQ][PART_BRK]=2;build_pages();
 proj.set.palette=NPALETTES-1;palette_set(proj.set.palette);draw_header();draw_main();draw_knobs();
 for(int pos=0;pos<4;pos++)CHECK(chance_knob_status[pos]==CHANCE_LIGHT_PENDING);
 shot("break-retrig-light-pending");
 puts("CHANCE actual native UI:606 exact oracle strips across six real configured-probability controls, bounds/domain guards PASS; light explicitly pending");return 0;
}
