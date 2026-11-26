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

typedef struct {juno6_t s;int16_t tape[2048];} Stock;
void *stock_new(unsigned rate,unsigned route){Stock*p=calloc(1,sizeof *p);if(!p)return 0;
p->s.typed=1;p->s.route=route;p->s.sample_rate=rate;juno6_delay_native_arm(&p->s,p->tape);return p;}
void stock_free(void*v){free(v);}
void stock_set(void*v,unsigned raw,unsigned fb,unsigned mx,unsigned sync,float bpm){
Stock*p=v;uint8_t b[4]={1+2*sync,raw,fb,mx};p->s.lfo_tempo=bpm;juno6_delay_native_config(&p->s,b,bpm);}
void stock_process(void*v,size_t n,const float*l,const float*r,float*L,float*R){
memcpy(L,l,n*sizeof(float));memcpy(R,r,n*sizeof(float));juno6_delay24_process(&((Stock*)v)->s,L,R,n);}
void stock_info(void*v,float *a){
Stock*p=v;a[0]=p->s.delay24.stride;a[1]=p->s.delay24.filled;a[2]=p->s.delay24.phase;
a[3]=p->s.delay24.current;a[4]=p->s.delay24.target;a[5]=p->s.delay24.sum[0];a[6]=p->s.delay24.ramp[0];
a[7]=p->s.delay24.clock_refused;a[8]=p->s.delay24.pos;}
void stock_ring(void*v,int16_t*out){memcpy(out,((Stock*)v)->tape,sizeof ((Stock*)v)->tape);}
int stock_can(void*v,unsigned t,float bpm,unsigned sync){return juno6_delay25_can(&((Stock*)v)->s,t,bpm,sync);}
