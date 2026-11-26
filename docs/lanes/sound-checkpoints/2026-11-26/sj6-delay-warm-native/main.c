/* Isolate the candidate's actual delay include without booting the firmware.
 * Layout fields are pinned to juno6.h in the source used for comparison. */
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#define X0X_SJ6_NATIVE_DELAY 1
#define X0X_SJ6_AUX_LFO_NATIVE 1
#define SJ6_AUX_FX_ROUTES 1
#define fm_powf powf
enum { SJ6_NATIVE_DELAY_CAP=1024 };
typedef struct {
  int typed,route;
  float sample_rate,lfo_tempo;
  struct {
    int16_t *tap;
    float current,target,fb,fb_target,wet,wet_target,inv_stride,sum[2],step[3];
    uint16_t ramp[3],stride,phase,pos,filled;
    uint8_t tick,mix,clock_refused,sync,enabled;
  } delay24;
} juno6_t;
static unsigned juno6_aux_fx_route(const juno6_t *s,unsigned k){return s->route?25:0;}
static unsigned sj6_aux_destination_id(unsigned r){return r;}
#include <juno6_delay_native.inc>

#include "fm1_time.h"
#define TICKS (*(volatile uint32_t *)0x1eee218u)
#define RESULTS ((volatile uint32_t *)0x01c08000)
static juno6_t s;
static int16_t hist[2048];
static float left[480],right[480];
static void seed(unsigned len,unsigned st,unsigned pos){
 for(unsigned i=0;i<2048;i++) hist[i]=(int16_t)(((i*7919u+53u)&32767u)-16384u);
 s.typed=1;s.route=0;s.sample_rate=44100.f;s.lfo_tempo=120.f;
 s.delay24.tap=hist;s.delay24.stride=st;s.delay24.pos=pos;s.delay24.filled=len;
 s.delay24.phase=st-1u;s.delay24.inv_stride=1.f/(float)st;
 s.delay24.current=s.delay24.target=11025.f/(float)st;s.delay24.step[0]=s.delay24.step[1]=s.delay24.step[2]=0.f;
 s.delay24.fb=s.delay24.fb_target=.499f;s.delay24.wet=s.delay24.wet_target=.9f;
 s.delay24.tick=0;s.delay24.mix=127;s.delay24.clock_refused=0;s.delay24.sync=0;s.delay24.enabled=1;
 s.delay24.ramp[0]=s.delay24.ramp[1]=s.delay24.ramp[2]=0;
 s.delay24.sum[0]=-.131f;s.delay24.sum[1]=.224f;
}
static void run(unsigned row,unsigned len,unsigned st,unsigned pos,unsigned raw,unsigned sync,float bpm){
 seed(len,st,pos);
 uint8_t p[]={1+2*sync,raw,64,127};
 uint32_t start=fm1_ticks(),c=TICKS;
 juno6_delay_native_config(&s,p,bpm);
 uint32_t end=fm1_ticks(),d=TICKS;
 volatile uint32_t *r=RESULTS+4+row*10;
 r[0]=end-start;r[1]=d-c;r[2]=s.delay24.stride;r[3]=s.delay24.filled;r[4]=s.delay24.phase;r[5]=s.delay24.pos;r[6]=s.delay24.ramp[0];
 uint32_t hash=2166136261u;
 for(unsigned i=0;i<2048;i++)hash=(hash^(uint16_t)hist[i])*16777619u;
 r[7]=hash;
 for(unsigned j=0;j<480;j++){left[j]=0.f;right[j]=0.f;}
 start=fm1_ticks();
 juno6_delay24_process(&s,left,right,480);
 end=fm1_ticks();
 r[8]=end-start;
 uint32_t n=0;
 for(unsigned j=0;j<480;j++)n+=left[j]!=0.f||right[j]!=0.f;
 r[9]=n;
}
void bench(void){
 fm1_time_init();RESULTS[0]=0x534a3652;
 for(unsigned k=0;k<3;k++){
  unsigned offset=k*6;
  run(offset+0,1024,16,1023,42,0,120.f);
  run(offset+1,1023,16,1,42,0,120.f);
  run(offset+2,1024,16,1,127,0,120.f);
  run(offset+3,1024,16,163,127,1,120.f);
  run(offset+4,1024,16,77,127,1,12.f);
  run(offset+5,1024,16,507,21,0,120.f);
 }
 RESULTS[1]=0x600d600d;RESULTS[2]=fm1_ticks();RESULTS[3]=TICKS;
 for(;;)__asm__ volatile("" ::: "memory");
}
