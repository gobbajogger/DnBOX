/* Actual native UI dispatch and framebuffer against the hash-pinned Python oracle. */
#define X0X_GFINAL_CHANCE_PROBE 1
#define SMC_UI_MAIN inherited_chance_fixture
#include "smc_ui_test.c"
#include "../../firmware/src/app/gfinal/ui_chance_exact.c"
static uint16_t oracle_pixel(FILE *file){int lo=fgetc(file),hi=fgetc(file);CHECK(lo>=0&&hi>=0);return (uint16_t)(lo|(hi<<8));}
int main(void){
 const char *path=getenv("CHANCE_ORACLE");CHECK(path);FILE *oracle=fopen(path,"rb");CHECK(oracle);
 setup(V_PART,PART_BRK);pref_t ref=PR(R_BRKSET,0,0,BRK_BCHANCE);
 CHECK(pg.r[0][2].kind==ref.kind&&pg.r[0][2].c==ref.c);
 for(int v=0;v<=100;v++){
  pref_set(ref,v);draw_header();draw_main();draw_knobs();CHECK(chance_knob_status[2]==CHANCE_DRAWN);CHECK(pref_get(ref)==v);
  for(int y=171;y<195;y++)for(int x=129;x<169;x++)CHECK(fb[y*240+x]==swap16(oracle_pixel(oracle)));
  if(v==0||v==50||v==100){char name[40];snprintf(name,sizeof name,"break-canonical-chance-%d",v);shot(name);}
 }
 CHECK(fgetc(oracle)==EOF);fclose(oracle);
 const x0x_param_t *d=pref_desc(ref);CHECK(d&&d->max==100);
 cv_begin(240,KNOB_H,0x1234);cv_oy=-13;
 CHECK(chance_native_knob(ref,d,50,149)==CHANCE_DRAWN);
 for(int y=0;y<KNOB_H;y++)for(int x=0;x<240;x++)
  if(x<129||x>=169||y<13||y>=37)CHECK(cv_px[y*240+x]==swap16(0x1234));
 x0x_param_t wrong=*d;wrong.max=99;
 cv_begin(240,KNOB_H,0x1234);
 CHECK(chance_native_knob(ref,&wrong,50,149)==CHANCE_NATIVE_LAW_PENDING);
 CHECK(chance_native_knob(ref,d,-1,149)==CHANCE_NATIVE_LAW_PENDING);
 CHECK(chance_native_knob(ref,d,101,149)==CHANCE_NATIVE_LAW_PENDING);
 CHECK(chance_native_knob(PR(R_BRKSLOT,0,0,0),d,50,149)==CHANCE_NOT_TARGET);
 for(int i=0;i<240*KNOB_H;i++)CHECK(cv_px[i]==swap16(0x1234));
 proj.set.palette=NPALETTES-1;palette_set(proj.set.palette);draw_header();draw_main();draw_knobs();
 CHECK(ui_theme_light);CHECK(chance_knob_status[2]==CHANCE_LIGHT_PENDING);CHECK(pref_get(ref)==100);
 shot("break-chance-light-pending");
 puts("CHANCE actual native UI:101 exact oracle strips, bounds/domain guards PASS; light explicitly pending");return 0;
}
