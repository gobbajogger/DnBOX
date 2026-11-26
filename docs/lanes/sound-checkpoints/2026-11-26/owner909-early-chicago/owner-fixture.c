#define _XOPEN_SOURCE 700
#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "drum909.h"
#include "drum808.h"
#include "fastmath.h"
#include "chicago_mp4.h"
#include "voice_gain.h"
#include "oscillator_motion.h"
#include "binary32_math.h"
#include "exact-glide.inc"
extern int original_chicago_voice_span(original_voice_gain *,original_oscillators *,const original_prepared_oscillators *,original_ramp *,uint32_t *,float,original_chicago_mp4 *,uint32_t *,uint32_t *,uint32_t);
enum {N=256,C=192};
struct voice {original_voice_gain gain;original_oscillators phase;original_prepared_oscillators ctrl;original_ramp noise;uint32_t noise_rng;original_chicago_mp4 filter;};
struct case_data {drum909_t d9;drum808_t d8;struct voice v[2];float k9[4][N],k8[4][N],metal[4][N],bass[2][4][N],out[4][N];glide_t gl[4][4];};
static struct case_data a,b;
static pthread_barrier_t start, overlap, done;
static unsigned run_n, aud9, aud8;
static struct {uint64_t start,end;} remote_time,local_time;
static uint64_t now(void){struct timespec t;assert(clock_gettime(CLOCK_MONOTONIC,&t)==0);return (uint64_t)t.tv_sec*1000000000u+(unsigned)t.tv_nsec;}
static void kit9(struct case_data *c,unsigned n){float (*k)[N]=c->k9;
 memset(k,0,sizeof c->k9);
 if(!drum909_active(&c->d9)){drum909_advance_noise(&c->d9,n);drum909_pan_settle(&c->d9);return;}
 drum909_render_st_cleared(&c->d9,k[0],k[1],k[2],k[3],n,aud9);
}
static void prefix(struct case_data*c,unsigned n){float (*k)[N]=c->k8;memset(k,0,sizeof c->k8);
 drum808_render_st_lanes(&c->d8,k[0],k[1],k[2],k[3],n,aud8,D8_LANES_BODY);}
static void metal(struct case_data*c,unsigned n){drum808_metal_prepare(&c->d8,c->metal,n,aud8);}
static void finish8(struct case_data*c,unsigned n){float (*k)[N]=c->k8;
 drum808_finish_rest(&c->d8,c->metal,k[0],k[1],k[2],k[3],n,aud8);}
static void voices(struct case_data *c,unsigned n){
 for(unsigned i=0;i<2;i++){
  struct voice *v=&c->v[i]; uint32_t l[N],r[N];
  assert(original_chicago_voice_span(&v->gain,&v->phase,&v->ctrl,&v->noise,&v->noise_rng,0.72f,&v->filter,l,r,n));
  /* Preserve the engine's per-owner logical L/R in the order it mixes them; the
     full runtime's and 808 scratch pointers are not emulated by this fixture. */
  for(unsigned k=0;k<n;k++){
   float L=original_math_float(l[k]),R=original_math_float(r[k]);
   c->bass[i][0][k]=L;c->bass[i][1][k]=R;
   c->bass[i][2][k]=c->bass[i][3][k]=.5f*(L+R);
  }
 }
}
static void mix(struct case_data*c,unsigned n){
 memcpy(c->out,c->k9,sizeof c->out);
 for(unsigned j=0;j<4;j++)scale_glide(c->out[j],&c->gl[0][j],n);
 for(unsigned j=0;j<4;j++)add_glide(c->out[j],c->k8[j],&c->gl[1][j],n);
 for(unsigned i=0;i<2;i++)for(unsigned j=0;j<4;j++)add_glide(c->out[j],c->bass[i][j],&c->gl[i+2][j],n);
}
static void *runner(void *unused){(void)unused;
 for(unsigned k=0;k<C;k++){
  pthread_barrier_wait(&start);
  prefix(&b,run_n);
  pthread_barrier_wait(&overlap);
  remote_time.start=now();kit9(&b,run_n);remote_time.end=now();
  pthread_barrier_wait(&done);
 }
 return NULL;
}
static void init(struct case_data *c){
 memset(c,0,sizeof *c);drum909_init(&c->d9);drum808_init(&c->d8);
 for(unsigned i=0;i<2;i++){
  struct voice *v=&c->v[i];
  v->noise_rng=0xa41+c->d9.noise+i;v->noise=(original_ramp){0.035f,0.025f,-0.000125f};
  v->filter.prepared=1;v->filter.a=(original_ramp){.25f,.25f,0.f};
  v->filter.e=(original_ramp){.16f,.16f,0.f};v->filter.resonance=(original_ramp){.125f,.13f,.0001f};
  v->ctrl.ready=1;
  for(unsigned j=0;j<4;j++){
   v->ctrl.controls.increment[j]=0x01012345u+j*0x43567u+i*0x12013u;
   v->ctrl.controls.shape[j]=j;v->ctrl.controls.width[j]=.5f;
   v->ctrl.correction[j]=.0078125f;v->ctrl.triangle[j]=.05f;
   v->gain.oscillator[j]=(original_ramp){.2f,.27f,.001f};
  }
  v->gain.stereo[0]=(original_ramp){.8f,.8f,0.f};v->gain.stereo[1]=(original_ramp){.71f,.71f,0.f};
 }
 for(unsigned i=0;i<4;i++)for(unsigned j=0;j<4;j++)c->gl[i][j]=(glide_t){.6f-i*.12f,.65f-j*.13f};
}
static void triggers(struct case_data*c,unsigned k){
 if(k%7==0){drum909_trigger(&c->d9,DR_BD,1.f);drum909_trigger(&c->d9,DR_RD,.85f);drum909_trigger(&c->d9,DR_CH,.77f);drum808_trigger(&c->d8,D8_BD,1.f);}
 if(k%9==0){drum909_trigger(&c->d9,DR_SD,.7f);drum909_trigger(&c->d9,DR_OH,.7f);drum808_trigger(&c->d8,D8_CH,.55f);drum808_trigger(&c->d8,D8_SD,.58f);}
 if(k%11==0){drum808_trigger(&c->d8,D8_CY,.85f);drum808_trigger(&c->d8,D8_CP,.75f);drum909_trigger(&c->d9,DR_LT,.8f);}
}
static void eq(const void*x,const void*y,size_t z,unsigned k,const char *s){
 if(memcmp(x,y,z)){fprintf(stderr,"diff on span %u: %s\n",k,s);assert(0);}
}
#define E(field) eq(&a.field,&b.field,sizeof a.field,k,#field)
int main(void){
 init(&a); memcpy(&b,&a,sizeof a);
 pthread_t thread;assert(!pthread_barrier_init(&start,NULL,2));assert(!pthread_barrier_init(&overlap,NULL,2));assert(!pthread_barrier_init(&done,NULL,2));
 assert(!pthread_create(&thread,NULL,runner,0)); size_t compared=0,samples=0;unsigned overlapped=0,active_spans[4]={0},final_send_spans=0;uint64_t best_overlap=0;
 for(unsigned k=0;k<C;k++){
  static const unsigned lens[]={256,128,1,32,63,129,255,7};
  run_n=lens[k%8]; aud9=(k%3)?0x7ff:0x533; aud8=(k%5)?0x7ff:0x627;
  triggers(&a,k);triggers(&b,k);
  /* Baseline: real kit and prefix/rest helpers, then Chicago A/B, then ordered accumulation. */
  kit9(&a,run_n);prefix(&a,run_n);metal(&a,run_n);finish8(&a,run_n);voices(&a,run_n);mix(&a,run_n);
  pthread_barrier_wait(&start);
  pthread_barrier_wait(&overlap);
  local_time.start=now();voices(&b,run_n);local_time.end=now();
  pthread_barrier_wait(&done);
  metal(&b,run_n);finish8(&b,run_n);mix(&b,run_n);
  E(d9);E(d8);E(v);E(k9);E(k8);E(metal);E(bass);E(out);E(gl);
  for(unsigned j=0;j<run_n;j++){
   if(a.k9[0][j]!=0.f){active_spans[0]++;break;}
  }
  for(unsigned j=0;j<run_n;j++){
   if(a.k8[0][j]!=0.f){active_spans[1]++;break;}
  }
  for(unsigned i=0;i<2;i++)for(unsigned j=0;j<run_n;j++){
   if(a.bass[i][0][j]!=0.f){active_spans[i+2]++;break;}
  }
  for(unsigned j=0;j<run_n;j++){
   if(a.out[2][j]!=0.f&&a.out[3][j]!=0.f){final_send_spans++;break;}
  }
  compared+=sizeof a;samples+=run_n;
  uint64_t lo=remote_time.start>local_time.start?remote_time.start:local_time.start;
  uint64_t hi=remote_time.end<local_time.end?remote_time.end:local_time.end;
  if(hi>lo){overlapped++;if(hi-lo>best_overlap)best_overlap=hi-lo;}
 }
 assert(!pthread_join(thread,NULL)); assert(overlapped);
 assert(active_spans[0]>C/2&&active_spans[1]>C/2&&active_spans[2]==C&&active_spans[3]==C&&final_send_spans>C/2);
 printf("Active owner spans 909=%u 808=%u Chicago A=%u Chicago B=%u; both mixed sends present=%u\n",active_spans[0],active_spans[1],active_spans[2],active_spans[3],final_send_spans);
 printf("%u spans, %zu frames, %zu exact compared state/bus bytes; %u actual overlapping host renderer intervals (max %" PRIu64 " ns; not target timing)\n",C,samples,compared,overlapped,best_overlap);
}
