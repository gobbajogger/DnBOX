/* SPDX-License-Identifier: GPL-3.0-only */
#include "miami_ms20.h"
#include "binary32_classify.h"
#include "miami_n513_private.h"
#include <math.h>
#include "binary32_math.h"
#include "miami_pow2_fma.h"
#include <string.h>
#ifdef ORIGINAL_USE_SOFT_FMA
#include "fused_reference.h"
#include "miami_interpolation.h"
#define ref_fma original_rne_fmaf
/* Exact optimized interpolation was previously qualified against the
 * independent reference. Preserve the full software FMA elsewhere. */
#define interpolate original_miami_interpolate_rne
#else
#define ref_fma fmaf
#define interpolate fmaf
#endif
static float fl(uint32_t u){return original_math_float(u);}
static float clamp(float x,float lo,float hi){return x<lo?lo:x>hi?hi:x;}
static float lookup(const float *t,float q){if(q<=fl(0x3727c5ac))return t[0];if(q>=128)return t[128];unsigned j=(unsigned)q;return interpolate(t[j+1]-t[j],q-(float)j,t[j]);}
static void stage(original_ramp *r,float target,float reciprocal){r->target=target;r->delta=(target-r->current)*reciprocal;}
static float advance(const original_ramp *r){float q=r->current+r->delta;if(r->delta>0){if(q>=r->target)q=r->target;}else if(r->delta<0){if(q<=r->target)q=r->target;}else q=r->target;return q;}
int original_miami_nonlinear(float x,float *out){
 if(!out||!original_binary32_isfinite(x))return 0;
 float u=512.0f*original_binary32_abs(x),result;
 if(u<512){unsigned j=(unsigned)u;float a=fl(original_miami_n513_bits[j]),b=fl(original_miami_n513_bits[j+1]);float g=interpolate(b-a,u-(float)j,a);result=x*g;}
 else result=clamp(x,-1,1)*fl(original_miami_n513_bits[512]);
 *out=result;return 1;
}
int original_miami_mg_shape(unsigned ordinal,float phase,float *b,float *u){
 if(!b||!u||b==u||ordinal>3||!original_binary32_isfinite(phase)||phase< -1||phase>1)return 0;
 float bipolar,unipolar;
 if(ordinal==0){bipolar=original_miami_mg_twice_plus_one(phase<0?phase:-phase);unipolar=original_miami_mg_half_plus_half(bipolar);}
 else if(ordinal==1){bipolar=-phase;unipolar=original_miami_mg_half_plus_half(-phase);}
 else if(ordinal==2){bipolar=phase;unipolar=original_miami_mg_half_plus_half(phase);}
 else {bipolar=phase>=0?-1:1;unipolar=phase>=0?0:1;}
 *b=bipolar;*u=unipolar;return 1;
}
int original_miami_ms20_init(original_miami_ms20 *s,unsigned index,float a0,float b0,float cap0){
 if(!s||index>=128||!original_binary32_isfinite(a0)||!original_binary32_isfinite(b0)||!original_binary32_isfinite(cap0))return 0;
 memset(s,0,sizeof *s);s->noise=index+1;s->noise_gain.current=fl(0x3727c5ac);s->a.current=s->a.target=a0;s->b=b0;s->cap=cap0;return 1;
}
int original_miami_ms20_note_reset(original_miami_ms20 *s,float a0){
 if(!s||!original_binary32_isfinite(a0))return 0;
 memset(s->history,0,sizeof s->history);s->a.current=s->a.target=a0;s->a.delta=0;
 s->resonance=(original_ramp){0,0,0};s->prepared=0;return 1;
}
int original_miami_ms20_prepare(original_miami_ms20 *s,const original_ms20_tables *t,
 float cutoff,float resonance,float depth,float env,float md,float mc,float mr,float noise,float mn,uint32_t duration){
 if(!s||!t||!t->a||!t->b||!t->r||!t->cap||!duration||duration>2147482880u)return 0;
 float controls[]={cutoff,resonance,depth,env,md,mc,mr,noise,mn};for(unsigned i=0;i<9;i++)if(!original_binary32_isfinite(controls[i]))return 0;
 for(unsigned i=0;i<129;i++)if(!original_binary32_isfinite(t->a[i])||!original_binary32_isfinite(t->b[i])||!original_binary32_isfinite(t->r[i])||!original_binary32_isfinite(t->cap[i]))return 0;
 float r=resonance+mr;if(!original_binary32_isfinite(r)||r<0||r>1)return 0;
 float d=clamp(depth+md,-1,1),z=ref_fma(env,d,cutoff),q=ref_fma(mc,128,z*128.0f);
 if(!original_binary32_isfinite(q))return 0;
 float a=lookup(t->a,q),b=lookup(t->b,q),cap=lookup(t->cap,q),R;
 if(r==1)R=t->r[128];else{float rq=r*128.0f;unsigned j=(unsigned)rq;R=interpolate(t->r[j+1]-t->r[j],rq-(float)j,t->r[j]);}
 float target=R<=fl(0x3727c5ac)?fl(0x3727c5ac):(R<cap?R:cap);
 float ng=clamp(noise+mn,0,1);ng=ng*ng+fl(0x3727c5ac);float reciprocal=1.0f/(float)duration;
 stage(&s->a,a,reciprocal);stage(&s->resonance,target,reciprocal);stage(&s->noise_gain,ng,reciprocal);s->b=b;s->cap=cap;s->prepared=1;return 1;
}
int original_miami_ms20_sample(original_miami_ms20 *s,float osc,float gain,float *out){
 if(!s||!out||!s->prepared||!original_binary32_isfinite(osc)||!original_binary32_isfinite(gain))return 0;
 /* Calculate without publishing state: a failed sample must leave every field
  * unchanged. Only the ramp currents, noise and history change on success;
  * avoid copying the complete filter object twice on this sample path. */
 float g=advance(&s->a),r=advance(&s->resonance),ng=advance(&s->noise_gain);
 unsigned choose=(s->noise&0x40000000u)==0;uint32_t next_noise=(s->noise+s->noise)^(choose?8393u:0);
 float noise=choose?.5f:-.5f;float v=ng*noise+osc;v=gain*v;
 float x=v*fl(0x3fa47678);x=x*fl(0x3dcccccd);
 float a=x+g*(s->history[0]-x);float b=a+g*(s->history[1]-a),nonlinear;
 if(!original_miami_nonlinear(s->history[2]+b,&nonlinear))return 0;
 float feedback=r*nonlinear,difference=feedback-s->history[4];
 float d=difference+s->b*(s->history[3]+difference);float c=d+s->b*(s->history[2]-d);
 if(!original_binary32_isfinite(a)||!original_binary32_isfinite(b)||!original_binary32_isfinite(c)||!original_binary32_isfinite(d)||!original_binary32_isfinite(feedback))return 0;
 s->a.current=g;s->resonance.current=r;s->noise_gain.current=ng;s->noise=next_noise;
 s->history[0]=a;s->history[1]=b;s->history[2]=c;s->history[3]=d;s->history[4]=feedback;
 /* The output may alias a float field of s, as before: publish it last. */
 *out=nonlinear;return 1;
}

/* Block kernel for one active Miami voice: oscillators (sync 0), MS-20 filter
 * and stereo gain, added into L/R. The same float operations in the same
 * order as original_voice_gain_oscillator + original_miami_ms20_sample +
 * original_voice_gain_stereo per sample, with the state kept in locals.
 * Finite checks run once at the end of the block (0: fault). */
#include "voice_gain.h"
#include "oscillator_motion.h"
static inline float k_saw(float p,float correction){return p-clamp((p*correction)*128.0f,-1,1);}
static inline float k_wave(unsigned shape,float p,float width,float correction,float triangle){
 if(shape==0){float t=2.0f*(.5f-original_binary32_abs(p));float t2=t*t,t3=t*t2;return t+triangle*t3;}
 float value=k_saw(p,correction);
 if(shape>=2){float q=p+(width+width);if(q>1)q=q-2;value=value-k_saw(q,correction);}
 return value;
}
static inline float k_phase(uint32_t u){return (float)(int32_t)u*0x1p-31f;}
static inline float k_gain_advance(original_ramp *r){
 float n=r->current+r->delta;if((r->delta>0)!=(r->target>n))n=r->target;r->current=n;return n;
}
/* The kernel runs in passes over the span (n <= 64), each with only its own state live:
 * oscillator 0, 1, 2, 3 (each pass specialised on its constant shape), then the MS-20,
 * then the stereo gain. Per sample the same float operations in the same order as the
 * one-pass loop (the mix sums term 0, 1, 2, 3 in turn; each oscillator's gain ramp only
 * feeds that oscillator; osc 1 / 3 read the phase step osc 0 / 2 left in step[]): the
 * same samples and state. Fewer live values: far fewer stack spills a sample. */
#define K_SPAN 64
static inline __attribute__((always_inline)) float k_wave_c(unsigned shape,float p,float width,float correction,float triangle,uint8_t zero_triangle){
 if(shape==0){
  float t=2.0f*(.5f-original_binary32_abs(p));
  /* p is a finite phase in [-1,1], so t and t3=t*(t*t) are finite.
   * The only zero t is +0 (exact 0.5f-0.5f). With either signed
   * triangle zero, t+triangle*t3 is bitwise t+0.0f, including t=+0.
   * Retain an addition: returning t would not generally preserve -0. */
  if(zero_triangle)return t+0.0f;
  float t2=t*t,t3=t*t2;return t+triangle*t3;
 }
 float value=k_saw(p,correction);
 if(shape>=2){float q=p+(width+width);if(q>1)q=q-2;value=value-k_saw(q,correction);}
 return value;
}
/* even oscillator i: its wave (or 0), the phase step for osc i+1, the first mix term (i==0) or add (i==2) */
static inline __attribute__((always_inline)) void k_osc_even(unsigned shape,unsigned first,uint32_t *phase_p,original_ramp *og_p,
 uint32_t inc,float width,float correction,float triangle,uint8_t zero_triangle,float xmod,float b,int32_t pstep,float *mix,int32_t *step,uint32_t n){
 uint32_t phase=*phase_p;original_ramp og=*og_p;
 for(uint32_t j=0;j<n;j++){
  float gain=og.current,w;int32_t step_i;
  phase-=inc;
  if(gain!=0.0f||xmod!=0.0f){
   w=k_wave_c(shape,k_phase(phase),width,correction,triangle,zero_triangle);
   float a=w*xmod;float st=clamp(a+b,-1,1)*0x1p31f;step_i=st>=0x1p31f?INT32_MAX:(int32_t)st;
  }else{w=0.0f;step_i=pstep;}
  step[j]=step_i;
  if(first)mix[j]=gain!=0.0f?w*(gain*.5f):0.0f;
  else if(gain!=0.0f)mix[j]=mix[j]+w*(gain*.5f);
  (void)k_gain_advance(&og);
 }
 *phase_p=phase;*og_p=og;
}
/* odd oscillator: steps by step[], adds its mix term */
static inline __attribute__((always_inline)) void k_osc_odd(unsigned shape,uint32_t *phase_p,original_ramp *og_p,
 float width,float correction,float triangle,uint8_t zero_triangle,float *mix,const int32_t *step,uint32_t n){
 uint32_t phase=*phase_p;original_ramp og=*og_p;
 for(uint32_t j=0;j<n;j++){
  float gain=og.current;
  phase-=(uint32_t)step[j];
  if(gain!=0.0f){float w=k_wave_c(shape,k_phase(phase),width,correction,triangle,zero_triangle);mix[j]=mix[j]+w*(gain*.5f);}
  (void)k_gain_advance(&og);
 }
 *phase_p=phase;*og_p=og;
}

/* A signed-zero or sub-ulp step can leave a gain unchanged throughout the span. */
static int k_fixed_gain(original_ramp r){
 uint32_t before=original_math_bits(r.current);
 return before==original_math_bits(k_gain_advance(&r));
}
static inline int32_t k_mod_delta(float wave,float xmod,float base){
 float st=clamp(wave*xmod+base,-1,1)*0x1p31f;
 return st>=0x1p31f?INT32_MAX:(int32_t)st;
}
/* The unity-corrected, half-width pulse uses two saturated saws away from
 * either discontinuity. Preserve the positive half's rounded phase addition. */
static __attribute__((noinline)) void k_pulse_fixed(unsigned first,uint32_t *phase_p,float gain,
 uint32_t inc,float xmod,float base,float *mix,int32_t *step,uint32_t n){
 uint32_t phase=*phase_p;
 const int32_t plus=k_mod_delta(1.0f,xmod,base),minus=k_mod_delta(-1.0f,xmod,base);
 const float scale=gain*.5f;
 for (uint32_t j=0;j<n;j++){
  phase-=inc;
  const uint32_t half=phase&0x7fffffffu;
  float wave; int32_t delta;
  if(half-0x02000000u<=0x7c000000u){
   if((int32_t)phase<0){wave=1.0f;delta=plus;}
   else{
    float q=k_phase(phase);
    wave=(q-1.0f)-(((q+1.0f)-2.0f)+1.0f);
    delta=wave==-1.0f?minus:k_mod_delta(wave,xmod,base);
   }
  }else{
   wave=k_wave_c(2,k_phase(phase),.5f,1.0f,0.0f,0);
   delta=k_mod_delta(wave,xmod,base);
  }
  step[j]=delta;
  if(gain!=0.0f){float term=wave*scale;mix[j]=first?term:mix[j]+term;}
  else if(first)mix[j]=0.0f;
 }
 *phase_p=phase;
}
#define K_SHAPES(sh,CALL) do{ if((sh)==0){enum{K_S=0};CALL;} else if((sh)==1){enum{K_S=1};CALL;} else {enum{K_S=2};CALL;} }while(0)
static inline __attribute__((always_inline)) void k_odd_fallback(const original_prepared_oscillators *p,
 unsigned odd,uint32_t *phase,original_ramp *og,const uint8_t *zero_triangle,
 float *mix,const int32_t *step,uint32_t n){
 const original_osc_controls *c=&p->controls;
  K_SHAPES(c->shape[odd],k_osc_odd(K_S,&phase[odd],&og[odd],c->width[odd],p->correction[odd],p->triangle[odd],zero_triangle[odd],mix,step,n));
}
static __attribute__((noinline)) void k_odd_pass(const original_prepared_oscillators *p,
 unsigned odd,uint32_t *phase,original_ramp *og,const uint8_t *zero_triangle,
 float *mix,int32_t *step,uint32_t n,int32_t pstep,int constant){
 const original_osc_controls *c=&p->controls;
 const int held=k_fixed_gain(og[odd]);
 if(held && og[odd].current==0.0f){
  uint32_t delta=0;
  if(constant)delta=(uint32_t)pstep*n;
  else for(uint32_t j=0;j<n;j++)delta+=(uint32_t)step[j];
  phase[odd]-=delta;
 }else if(held && c->shape[odd]==1){
  uint32_t at=phase[odd];float scale=og[odd].current*.5f,correction=p->correction[odd];
  if(constant)for(uint32_t j=0;j<n;j++){at-=(uint32_t)pstep;mix[j]+=k_saw(k_phase(at),correction)*scale;}
  else for(uint32_t j=0;j<n;j++){at-=(uint32_t)step[j];mix[j]+=k_saw(k_phase(at),correction)*scale;}
  phase[odd]=at;
 }else{
  if(constant)for(uint32_t j=0;j<n;j++)step[j]=pstep;
  k_odd_fallback(p,odd,phase,og,zero_triangle,mix,step,n);
 }
}
static __attribute__((noinline)) void k_even_fallback(unsigned first,const original_prepared_oscillators *p,
 unsigned even,uint32_t *phase,original_ramp *og,const uint8_t *zero_triangle,float base,int32_t pstep,
 float *mix,int32_t *step,uint32_t n){
 const original_osc_controls *c=&p->controls;
 uint32_t inc=c->increment[even];
 K_SHAPES(c->shape[even],k_osc_even(K_S,first,&phase[even],&og[even],inc,c->width[even],p->correction[even],p->triangle[even],zero_triangle[even],c->xmod,base,pstep,mix,step,n));
}
/* A monotonically decreasing phase cannot cross a pulse edge when its
 * first and final samples are both inside this negative half and the
 * unsigned distance fits between them. In that interval wave is +1. */
static inline int k_unity_pulse_span(uint32_t phase,uint32_t inc,uint32_t n){
 uint32_t first=phase-inc;
 return first>=0x82000000u && first<=0xfe000000u &&
        inc<=0x7c000000u/64u && inc*(n-1u)<=first-0x82000000u;
}
static inline __attribute__((always_inline)) void k_pulse_pair(unsigned first,
 uint32_t *phase,original_ramp *og,const original_prepared_oscillators *p,
 unsigned even,float base,int32_t pstep,float *mix,int32_t *step,uint32_t n,
 const uint8_t *zero_triangle){
 const original_osc_controls *c=&p->controls;
 unsigned odd=even+1;uint32_t inc=c->increment[even];int constant=0;
 const int fixed_pulse=c->shape[even]>=2 && c->width[even]==.5f &&
                       p->correction[even]==1.0f && k_fixed_gain(og[even]);
 if(fixed_pulse && k_unity_pulse_span(phase[even],inc,n)){
  constant=1;pstep=k_mod_delta(1.0f,c->xmod,base);
  phase[even]-=inc*n;
  float gain=og[even].current;
  if(first){float term=gain!=0.0f?gain*.5f:0.0f;for(uint32_t j=0;j<n;j++)mix[j]=term;}
  else if(gain!=0.0f){float term=gain*.5f;for(uint32_t j=0;j<n;j++)mix[j]+=term;}
 }else if(fixed_pulse)
  k_pulse_fixed(first,&phase[even],og[even].current,inc,c->xmod,base,mix,step,n);
 else k_even_fallback(first,p,even,phase,og,zero_triangle,base,pstep,mix,step,n);
 k_odd_pass(p,odd,phase,og,zero_triangle,mix,step,n,pstep,constant);
}
int original_miami_voice_render_add(original_miami_ms20 *f,original_voice_gain *vg,original_oscillators *ph,
 const original_prepared_oscillators *p,float vco_gain,float *L,float *R,uint32_t n){
 if(!f->prepared||!p->ready||!original_binary32_isfinite(vco_gain))return 0;
 const original_osc_controls *c=&p->controls;
 uint8_t zero_triangle[4];
 for(unsigned i=0;i<4;i++)
  zero_triangle[i]=(uint8_t)(c->shape[i]==0&&(original_math_bits(p->triangle[i])&0x7fffffffu)==0);
 original_voice_gain gains=*vg;original_ramp *og=gains.oscillator,*st=gains.stereo;
 original_oscillators osc=*ph;uint32_t *phase=osc.phase;
 const float b1=(float)c->increment[1]*0x1p-31f,b3=(float)c->increment[3]*0x1p-31f;
 const float st1=clamp(b1,-1,1)*0x1p31f,st3=clamp(b3,-1,1)*0x1p31f;
 const int32_t pstep1=st1>=0x1p31f?INT32_MAX:(int32_t)st1,pstep3=st3>=0x1p31f?INT32_MAX:(int32_t)st3;
 original_ramp ra=f->a,rr=f->resonance,rn=f->noise_gain;
 float h0=f->history[0],h1=f->history[1],h2=f->history[2],h3=f->history[3],h4=f->history[4];
 const float fb=f->b;uint32_t noise=f->noise;
 float mix[K_SPAN];int32_t step[K_SPAN];
 while(n){   /* callers pass control spans of <= 64 frames (render_span): one pass */
 const uint32_t m=n<K_SPAN?n:K_SPAN;
 k_pulse_pair(1,phase,og,p,0,b1,pstep1,mix,step,m,zero_triangle);
 k_pulse_pair(0,phase,og,p,2,b3,pstep3,mix,step,m,zero_triangle);
 int filter_held=original_math_bits(advance(&ra))==original_math_bits(ra.current)&&
  original_math_bits(advance(&rr))==original_math_bits(rr.current)&&original_math_bits(advance(&rn))==original_math_bits(rn.current);
 /* MS-20: mix[] -> the nonlinear output, in place */
 for(uint32_t j=0;j<m;j++){
  if(!filter_held){ra.current=advance(&ra);rr.current=advance(&rr);rn.current=advance(&rn);}
  float g=ra.current,r=rr.current,ng=rn.current;
  unsigned choose=(noise&0x40000000u)==0;noise=(noise+noise)^(choose?8393u:0);
  float nz=choose?.5f:-.5f;float v=ng*nz+mix[j];v=vco_gain*v;
  float x=v*fl(0x3fa47678);x=x*fl(0x3dcccccd);
  float a=x+g*(h0-x);float b=a+g*(h1-a);
  float in=h2+b,u=512.0f*original_binary32_abs(in),nl;
  if(u<512){unsigned k=(unsigned)u;float ta=fl(original_miami_n513_bits[k]),tb=fl(original_miami_n513_bits[k+1]);float gg=interpolate(tb-ta,u-(float)k,ta);nl=in*gg;}
  else nl=clamp(in,-1,1)*fl(original_miami_n513_bits[512]);
  float feedback=r*nl,difference=feedback-h4;
  float d=difference+fb*(h3+difference);float cc=d+fb*(h2-d);
  h0=a;h1=b;h2=cc;h3=d;h4=feedback;
  mix[j]=nl;
 }
 /* stereo gain */
 if(k_fixed_gain(st[0]) && k_fixed_gain(st[1])){
  for(uint32_t j=0;j<m;j++){L[j]=L[j]+mix[j]*st[0].current;R[j]=R[j]+mix[j]*st[1].current;}
 }else for(uint32_t j=0;j<m;j++){
  float l=k_gain_advance(&st[0]),rg=k_gain_advance(&st[1]);
  L[j]=L[j]+mix[j]*l;R[j]=R[j]+mix[j]*rg;
 }
 L+=m;R+=m;n-=m;
 }
 if(!original_binary32_isfinite(h0)||!original_binary32_isfinite(h1)||!original_binary32_isfinite(h2)||
    !original_binary32_isfinite(h3)||!original_binary32_isfinite(h4))return 0;
 *vg=gains;*ph=osc;
 f->a=ra;f->resonance=rr;f->noise_gain=rn;f->noise=noise;
 f->history[0]=h0;f->history[1]=h1;f->history[2]=h2;f->history[3]=h3;f->history[4]=h4;
 return 1;
}
