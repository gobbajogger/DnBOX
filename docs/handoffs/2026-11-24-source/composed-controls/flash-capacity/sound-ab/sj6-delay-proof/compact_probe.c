/* Isolate the candidate's actual delay include without booting the firmware.
 * Layout fields are pinned to juno6.h in the source used for comparison. */
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#define X0X_SJ6_NATIVE_DELAY 1
#define X0X_SJ6_AUX_LFO_NATIVE 0
#define fm_powf powf
enum { SJ6_NATIVE_DELAY_CAP=1024 };
typedef struct {
  int typed;
  float sample_rate,lfo_tempo;
  struct {
    int16_t *tap;
    float current,target,fb,fb_target,wet,wet_target,inv_stride,sum[2],step[3];
    uint16_t ramp[3],stride,phase,pos,filled;
    uint8_t tick,mix,clock_refused,sync,enabled;
  } delay24;
} juno6_t;
#include "juno6_delay_native.inc"

/* Steady controls: finish the target's real startup ramp before injecting
 * a test tone. The target's start/stop or UI mapper is not under test. */
unsigned compact(unsigned raw,unsigned fb,unsigned mx,float *l,float *r,unsigned n) {
    int16_t storage[2*SJ6_NATIVE_DELAY_CAP]={0};
    juno6_t s={.sample_rate=44100,.typed=1};
    juno6_delay_native_arm(&s,storage);
    const uint8_t ctl[]={1,raw,fb,mx};
    juno6_delay_native_config(&s,ctl,120.f);
    float zero_l[128]={0},zero_r[128]={0};
    unsigned stride=s.delay24.stride;
    for(unsigned i=0;i<128;i++)juno6_delay24_process(&s,zero_l,zero_r,128);
    juno6_delay24_process(&s,l,r,n);
    return stride;
}
