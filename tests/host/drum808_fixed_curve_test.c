/* Parameter-owner curve must not alias the selected track's circuit. */
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "drum808.h"
extern int drum808_param_curve(const drum808_t*,int,int,drum_curve_t*) __attribute__((weak));
#define REQUIRE(x) do{if(!(x)){fprintf(stderr,"FIXED_OWNER_CURVE_FAIL %d: %s\n",__LINE__,#x);return 1;}}while(0)
static drum808_t d,before;
int main(void){
 REQUIRE(drum808_param_curve!=0);
 drum808_init(&d);
 for(int selected=0;selected<2;selected++)for(int raw=0;raw<128;raw++){
  drum808_set(&d,D8_CP,4,selected);drum808_set(&d,D8_CP,3,raw);
  drum_curve_t v={0},sel={0};before=d;
  REQUIRE(drum808_param_curve(&d,D8_CP,3,&v));
  REQUIRE(!memcmp(&before,&d,sizeof d));
  double a=(double)raw/32,ms=fmax(.2,1.2*a*a)*log(100.);
  REQUIRE(fabs(v.rise_end-ms)<.0001 && v.delay==0 && v.window==350);
  REQUIRE(drum808_curve(&d,D8_CP,&sel));
  REQUIRE(selected?sel.delay==0:sel.delay>0);
 }
 /* Preview targets resolve the requested sound, without changing the
  * selected/audio owner or any latched circuit field. */
 for(int selection=0;selection<2;selection++)for(int raw=0;raw<128;raw++){
  drum808_set(&d,D8_CP,4,!selection);before=d;drum_curve_t v;
  REQUIRE(drum808_param_curve_preview(&d,D8_CP,3,selection,raw,&v)==D8S_MA+1);
  double rise=fmax(.2,1.2*pow((double)raw/32,2))*log(100.);
  REQUIRE(fabs(v.rise_end-rise)<.0001 && !memcmp(&d,&before,sizeof d));
  REQUIRE(drum808_param_curve_preview(&d,D8_CP,2,selection,raw,&v)==(selection?D8S_MA:D8S_CP)+1);
  double seconds=(selection?.01:.35)*pow(selection?30.:8.,raw/127.);
  double tau=seconds*1000*(selection?.85/log(100.):.0385);
  REQUIRE(fabs(v.fall-tau)<.001 && !memcmp(&d,&before,sizeof d));
 }
 /* Trigger MA, then select/trigger CP: its fixed Attack keeps observing MA. */
 drum808_set(&d,D8_CP,3,32);drum808_set(&d,D8_CP,4,1);drum808_trigger(&d,D8_CP,1);
 float dry[256]={0},rev[256]={0},dly[256]={0};
 drum808_render(&d,dry,rev,dly,32);
 drum_curve_t ma,cp;REQUIRE(drum808_param_curve(&d,D8_CP,3,&ma)==D8S_MA+1);
 REQUIRE(ma.elapsed>=0 && ma.level>0 && d.ma.rising);
 float old_coefficient=d.ma.atk_c;
 drum808_set(&d,D8_CP,4,0);drum808_trigger(&d,D8_CP,1);
 REQUIRE(drum808_param_curve(&d,D8_CP,3,&cp)==D8S_MA+1);
 REQUIRE(cp.level==ma.level && cp.rise_end==ma.rise_end);
 drum808_set(&d,D8_CP,3,127);
 REQUIRE(drum808_param_curve(&d,D8_CP,3,&cp));
 REQUIRE(cp.rise_end>ma.rise_end && d.ma.atk_c==old_coefficient && cp.level==ma.level);
 /* The fixed Attack row must not label MA's later decay as active attack. */
 for(int n=0;n<4;n++)drum808_render(&d,dry,rev,dly,256);
 REQUIRE(d.ma.active && !d.ma.rising);
 REQUIRE(drum808_param_curve(&d,D8_CP,3,&cp));
 REQUIRE(cp.elapsed<0);
 /* Editing a target must not rewrite the sounding envelope's latched rate. */
 drum808_all_off(&d);REQUIRE(drum808_param_curve(&d,D8_CP,3,&cp));REQUIRE(cp.elapsed<0);
 drum808_set(&d,D8_CP,4,0);drum808_trigger(&d,D8_CP,1);
 REQUIRE(drum808_param_curve(&d,D8_CP,2,&cp)==D8S_CP+1);REQUIRE(cp.elapsed<0);
 for(int n=0;n<8;n++)drum808_render(&d,dry,rev,dly,256);
 REQUIRE(d.cp.active && d.cp.main_target==0 && d.cp.age>d.cp.main_open);
 REQUIRE(drum808_param_curve(&d,D8_CP,2,&cp));REQUIRE(cp.elapsed>=0);
 drum808_set(&d,D8_CP,4,1);drum808_trigger(&d,D8_CP,1);
 REQUIRE(drum808_param_curve(&d,D8_CP,2,&cp)==D8S_MA+1);REQUIRE(cp.elapsed<0);
 drum_curve_t v;memset(&v,0xa5,sizeof v);drum_curve_t saved=v;
 REQUIRE(!drum808_param_curve(&d,-1,3,&v));REQUIRE(!memcmp(&v,&saved,sizeof v));
 REQUIRE(!drum808_param_curve(&d,D8_CP,-1,&v));
 REQUIRE(!drum808_param_curve(&d,D8_CP,4,&v)); /* selector is not an envelope */
 REQUIRE(!drum808_param_curve(&d,D8_CP,3,0));
 REQUIRE(!drum808_param_curve_preview(&d,D8_CP,3,2,64,&v));
 REQUIRE(!drum808_param_curve_preview(&d,D8_CP,3,0,128,&v));
 REQUIRE(!drum808_param_curve_preview(&d,D8_CP,3,-2,64,&v));
 puts("PASS 256 applied +512 preview fixed-owner curves, selected circuit separation, read-only and invalid domains");return 0;
}
