/* SPDX-License-Identifier: GPL-3.0-only */
#include "chicago_mp4.h"
#include "binary32_classify.h"
#include <math.h>
#include "binary32_math.h"
#include <string.h>
#ifdef ORIGINAL_USE_SOFT_FMA
#include "fused_reference.h"
#define ref_fma original_rne_fmaf
#else
#define ref_fma fmaf
#endif
static float clamp(float x,float lo,float hi){return x<lo?lo:x>hi?hi:x;}
static void stage(original_ramp *r,float target,float reciprocal){r->target=target;r->delta=(target-r->current)*reciprocal;}
static float advance(original_ramp *r){float q=r->current+r->delta;if(r->delta>0){if(q>=r->target)q=r->target;}else if(r->delta<0){if(q<=r->target)q=r->target;}else q=r->target;r->current=q;return q;}
int original_chicago_feedback(float input,float *out){
 if(!out||!original_binary32_isfinite(input))return 0;
 float u=original_binary32_abs(input)>=1?0:1-original_binary32_abs(input);float u2=u*u,u4=u2*u2,u5=u*u4;float v=1-u5;*out=input>=0?v:-v;return 1;
}
int original_chicago_mp4_init(original_chicago_mp4 *s,float a0,float e0){
 if(!s||!original_binary32_isfinite(a0)||!original_binary32_isfinite(e0)||a0<0||a0>1||e0< -1||e0>1)return 0;
 memset(s,0,sizeof *s);s->a.current=s->a.target=a0;s->e.current=s->e.target=e0;return 1;
}
int original_chicago_mp4_prepare_reader(original_chicago_mp4 *s,original_mp4_reader read,const void *context,
 float cutoff,float depth,float env,float md,float mc,int note,float track,float resonance,float mr,uint32_t duration,unsigned kick,float *postgain){
 if(!s||!read||!postgain||!duration||duration>2147482880u||kick>1||note<0||note>127)return 0;
 float controls[]={cutoff,depth,env,md,mc,track,resonance,mr};for(unsigned i=0;i<8;i++)if(!original_binary32_isfinite(controls[i]))return 0;
 float sum=depth+md;if(!original_binary32_isfinite(sum)||!original_binary32_isfinite(resonance+mr))return 0;
 float d=clamp(sum,-1,1),z=ref_fma(env,d,cutoff),q=z*16384.0f,key=(float)(note-48)*128.0f;
 q=ref_fma(key,track,q);q=ref_fma(mc,16384.0f,q);if(!original_binary32_isfinite(q))return 0;
 q=clamp(q,0,16384);unsigned j=(unsigned)q;float a,e;if(!read(context,j,&a,&e))return 0;
 if(!original_binary32_isfinite(a)||a<0||a>1||!original_binary32_isfinite(e)||e< -1||e>1)return 0;
 float R=clamp(resonance+mr,0,1),reciprocal=1.0f/(float)duration;
 original_chicago_mp4 next=*s;stage(&next.a,a,reciprocal);stage(&next.e,e,reciprocal);stage(&next.resonance,R,reciprocal);
 if(kick){float r2=R*R,r4=r2*r2,r8=r4*r4;next.history[0]=R>.2f?r8:0;}
 next.prepared=1;*s=next;*postgain=clamp(ref_fma(resonance+mr,4,1),1,4);return 1;
}
static int raw_table_read(const void *context,uint32_t index,float *a,float *e){
 const original_mp4_tables *t=context;if(!t||!t->a||!t->e||index>16384)return 0;*a=t->a[index];*e=t->e[index];return 1;
}
int original_chicago_mp4_prepare(original_chicago_mp4 *s,const original_mp4_tables *tables,
 float cutoff,float depth,float env,float md,float mc,int note,float track,float resonance,float mr,uint32_t duration,unsigned kick,float *postgain){
 return original_chicago_mp4_prepare_reader(s,raw_table_read,tables,cutoff,depth,env,md,mc,note,track,resonance,mr,duration,kick,postgain);
}
int original_chicago_mp4_sample(original_chicago_mp4 *s,float input,float *out){
 if(!s||!out||!s->prepared||!original_binary32_isfinite(input))return 0;
 original_chicago_mp4 n=*s;float A=advance(&n.a),E=advance(&n.e),R=advance(&n.resonance);
 float tmp=n.history[3]+E*n.history[4],d=n.history[4]-E*tmp,nonlinear;
 if(!original_chicago_feedback(d,&nonlinear))return 0;
 float v=input-R*nonlinear,z1=v+A*(n.history[0]-v),z2=z1+A*(n.history[1]-z1),z3=z2+A*(n.history[2]-z2),z4=z3+A*(n.history[3]-z3);
 if(!original_binary32_isfinite(z1)||!original_binary32_isfinite(z2)||!original_binary32_isfinite(z3)||!original_binary32_isfinite(z4)||!original_binary32_isfinite(tmp))return 0;
 n.history[0]=z1;n.history[1]=z2;n.history[2]=z3;n.history[3]=z4;n.history[4]=tmp;*s=n;*out=z4;return 1;
}

/* One Chicago voice sample: oscillators (sync 1), noise, MP4 filter and
 * stereo gain. The same float operations in the same order as
 * original_voice_gain_oscillator + the noise step + original_chicago_mp4_sample
 * + original_voice_gain_stereo, without the per-call state copies.
 * Returns 0 (state unchanged except as the original would fail) on a
 * nonfinite filter value. */
#include "voice_gain.h"
#include "oscillator_motion.h"
static inline float ck_clamp(float x,float lo,float hi){return x<lo?lo:x>hi?hi:x;}
static inline float ck_saw(float p,float correction){return p-ck_clamp((p*correction)*128.0f,-1,1);}
static inline float ck_wave(unsigned shape,float p,float width,float correction,float triangle){
 if(shape==0){float t=2.0f*(.5f-original_binary32_abs(p));float t2=t*t,t3=t*t2;return t+triangle*t3;}
 float value=ck_saw(p,correction);
 if(shape>=2){float q=p+(width+width);if(q>1)q=q-2;value=value-ck_saw(q,correction);}
 return value;
}
/* Each slave uses the original rounded binary32 increment when its master
 * is silent or cross modulation is either sign of zero. Cache that exact
 * integer once per span; master sync still observes every phase wrap. */
typedef struct {float base[2];uint32_t plain[2];} ck_slaves;
static inline uint32_t ck_phase_delta(float step){
 return (uint32_t)(step>=0x1p31f?INT32_MAX:(int32_t)step);
}
static inline float ck_phase(uint32_t u){return (float)(int32_t)u*0x1p-31f;}
static inline float ck_xadvance(original_ramp *r){float n=r->current+r->delta;if((r->delta>0)!=(r->target>n))n=r->target;r->current=n;return n;}
/* One sample. Locals stand in for the voice state; the caller loads and
 * stores them once per span. Commit order matches the original: phases,
 * gain ramps and noise advance before the filter checks; the filter state
 * changes only when every filter value is finite. */
/* Shared by held and moving ramps. Every oscillator, waveform, add and
 * sync remains in its original order on the general path. */
static __attribute__((noinline)) float oscillator_mix(const original_prepared_oscillators *p,
 const ck_slaves *slaves,uint32_t phase[4],const float gain[4]){
 const original_osc_controls *c=&p->controls;float w[4];
 for(unsigned i=0;i<4;i+=2){
  uint32_t old=phase[i];phase[i]-=c->increment[i];
  uint32_t delta=slaves->plain[i/2];
  w[i]=0.0f;
  if(gain[i]!=0.0f||c->xmod!=0.0f){
   w[i]=ck_wave(c->shape[i],ck_phase(phase[i]),c->width[i],p->correction[i],p->triangle[i]);
   if(c->xmod!=0.0f||!original_binary32_isfinite(w[i])){
    float a=w[i]*c->xmod;
    delta=ck_phase_delta(ck_clamp(a+slaves->base[i/2],-1,1)*0x1p31f);
   }
  }
  phase[i+1]-=delta;
  if((int32_t)phase[i]>(int32_t)old)phase[i+1]=0x7fffffffu;
  w[i+1]=gain[i+1]!=0.0f?ck_wave(c->shape[i+1],ck_phase(phase[i+1]),c->width[i+1],p->correction[i+1],p->triangle[i+1]):0.0f;
 }
 float mix=gain[0]!=0.0f?w[0]*(gain[0]*.5f):0.0f;for(unsigned i=1;i<4;i++)if(gain[i]!=0.0f)mix=mix+w[i]*(gain[i]*.5f);
 return mix;
}
static inline __attribute__((always_inline)) int ck_step(const original_prepared_oscillators *p,const ck_slaves *slaves,int parked,int filter_held,float vco_gain,
 uint32_t phase[4],original_ramp og[4],original_ramp st[2],original_ramp *ngr,uint32_t *nsp,
 float H[5],original_ramp *ra,original_ramp *re,original_ramp *rr,uint32_t *lo,uint32_t *ro){
 float gain[4];
 for(unsigned i=0;i<4;i++)gain[i]=og[i].current;
 float mix=oscillator_mix(p,slaves,phase,gain);
 if (!parked) for(unsigned i=0;i<4;i++)(void)ck_xadvance(&og[i]);
 float ng=parked?ngr->current:ck_xadvance(ngr);uint32_t ns=*nsp;unsigned choose=(ns&0x40000000u)==0;*nsp=(ns+ns)^(choose?8393u:0);
 float input=ng*(choose?.5f:-.5f)+mix;input=vco_gain*input;
 if(!original_binary32_isfinite(input))return 0;
 original_ramp a2=*ra,e2=*re,r2=*rr;
 float A=filter_held?ra->target:advance(&a2);
 float E=filter_held?re->target:advance(&e2);
 float R=filter_held?rr->target:advance(&r2);
 float tmp=H[3]+E*H[4],d=H[4]-E*tmp;
 if(!original_binary32_isfinite(d))return 0;
 float u=original_binary32_abs(d)>=1?0:1-original_binary32_abs(d);float u2=u*u,u4=u2*u2,u5=u*u4;float v5=1-u5;float nonlinear=d>=0?v5:-v5;
 float v=input-R*nonlinear,z1=v+A*(H[0]-v),z2=z1+A*(H[1]-z1),z3=z2+A*(H[2]-z2),z4=z3+A*(H[3]-z3);
 /* Each nonfinite cascade stage propagates to z4. The finite d check also covers tmp. */
 if(!original_binary32_isfinite(z4))return 0;
 *ra=a2;*re=e2;*rr=r2;H[0]=z1;H[1]=z2;H[2]=z3;H[3]=z4;H[4]=tmp;
 float l=parked?st[0].current:ck_xadvance(&st[0]),r=parked?st[1].current:ck_xadvance(&st[1]);
 *lo=original_math_bits(z4*l);*ro=original_math_bits(z4*r);return 1;
}
static inline int ramp_held(const original_ramp *r){return r->delta==0.0f&&r->current==r->target;}

/* An exact binary32 fixed point stays fixed throughout the span, even when
 * its tiny nonzero delta rounds away. Preserve the sign of a zero ramp. */
static int ck_fixed(original_ramp r){
    const uint32_t before = original_math_bits(r.current);
    return original_math_bits(ck_xadvance(&r)) == before;
}

/* A positive-zero gain stays positive zero for either sign of zero
 * increment. Exclude nonphysical prepared waveforms to retain the original
 * exceptional phase-delta path when cross modulation multiplies a NaN. */
static unsigned single_moving_oscillator(const original_ramp gain[4],const original_prepared_oscillators *p) {
 if (p->controls.xmod!=0.0f) return 4;
 unsigned selected=4;
 for (unsigned i=0;i<4;++i) {
  if (original_math_bits(gain[i].current)==0 && gain[i].delta==0.0f && gain[i].target==0.0f) continue;
  if (selected<4) return 4;
  selected=i;
 }
 if (selected==4) return 4;
 float width=p->controls.width[selected];
 if (!(width>=-1.0f && width<=1.0f) || !original_binary32_isfinite(p->correction[selected]) || !original_binary32_isfinite(p->triangle[selected])) return 4;
 return selected;
}

/* With a single audible master and zero modulation the slave phase has
 * no effect on the rendered samples. Reconstruct both hard-sync pairs for
 * the number of attempted samples, including a failing sample. */
static void ck_commit_pairs(uint32_t phase[4],const original_osc_controls *c,const ck_slaves *slaves,uint32_t count){
 for(unsigned even=0;even<4;even+=2){
  uint32_t master=phase[even],slave=phase[even+1],inc=c->increment[even],delta=slaves->plain[even/2];
  if(inc<=0x7fffffffu){
   uint32_t rank=master^0x80000000u;
   uint32_t end=master-inc*count;
   if(inc && (uint64_t)inc*count>rank){
    uint32_t after=(UINT32_MAX-(end^0x80000000u))/inc;
    slave=0x7fffffffu-delta*after;
   }else slave-=delta*count;
   phase[even]=end; phase[even+1]=slave;
   continue;
  }
  for(uint32_t j=0;j<count;j++){
   uint32_t prev=master;master-=inc;slave-=delta;
   if((int32_t)master>(int32_t)prev)slave=0x7fffffffu;
  }
  phase[even]=master;phase[even+1]=slave;
 }
}
static float ck_fixed_pulse(uint32_t phase,float correction){
 uint32_t half=phase&0x7fffffffu;
 if(half-0x08000000u<=0x70000000u){
  if((int32_t)phase<0)return 1.0f;
  float q=ck_phase(phase);
  return (q-1.0f)-(((q+1.0f)-2.0f)+1.0f);
 }
 return ck_wave(2,ck_phase(phase),.5f,correction,0.0f);
}
static __attribute__((noinline)) int ck_pulse_span(const original_prepared_oscillators *p,
 const ck_slaves *slaves,unsigned even,float gain,float ng,float sl,float sr,
 float vco_gain,uint32_t phase[4],uint32_t *nsp,float H[5],
 original_ramp *ra,original_ramp *re,original_ramp *rr,uint32_t *lo,uint32_t *ro,uint32_t n){
 uint32_t attempted=0,at=phase[even],inc=p->controls.increment[even],noise=*nsp;
 float h0=H[0],h1=H[1],h2=H[2],h3=H[3],h4=H[4],scale=gain*.5f;
 original_ramp aa=*ra,ee=*re,rval=*rr;
 int ok=1;
 while(ok && attempted<n){
  original_ramp an=aa,en=ee,rn=rval;
  int held=(original_math_bits(advance(&an))==original_math_bits(aa.current) &&
            original_math_bits(advance(&en))==original_math_bits(ee.current) &&
            original_math_bits(advance(&rn))==original_math_bits(rval.current));
  uint32_t stop=n;
  if(!held && n-attempted>12)stop=attempted+12;
  while(attempted<stop){
  unsigned j=attempted++;
  at-=inc;float term=ck_fixed_pulse(at,p->correction[even])*scale;
  float mix=even==0?term:0.0f+term;
  unsigned choose=(noise&0x40000000u)==0;noise=(noise+noise)^(choose?8393u:0);
  float input=ng*(choose?.5f:-.5f)+mix;input=vco_gain*input;
  if(!original_binary32_isfinite(input)){ok=0;break;}
  original_ramp next_a=aa,next_e=ee,next_r=rval;
  float A=held?aa.current:advance(&next_a);
  float E=held?ee.current:advance(&next_e);
  float R=held?rval.current:advance(&next_r);
  float tmp=h3+E*h4,d=h4-E*tmp;
  if(!original_binary32_isfinite(d)){ok=0;break;}
  float u=original_binary32_abs(d)>=1?0:1-original_binary32_abs(d);
  float u2=u*u,u4=u2*u2,u5=u*u4,nonlinear=d>=0?1-u5:-(1-u5);
  float v=input-R*nonlinear,z1=v+A*(h0-v),z2=z1+A*(h1-z1),z3=z2+A*(h2-z2),z4=z3+A*(h3-z3);
  if(!original_binary32_isfinite(z4)){ok=0;break;}
  aa.current=A;ee.current=E;rval.current=R;
  h0=z1;h1=z2;h2=z3;h3=z4;h4=tmp;
  lo[j]=original_math_bits(z4*sl);ro[j]=original_math_bits(z4*sr);
  }
 }
 ra->current=aa.current;re->current=ee.current;rr->current=rval.current;
 H[0]=h0;H[1]=h1;H[2]=h2;H[3]=h3;H[4]=h4;*nsp=noise;
 ck_commit_pairs(phase,&p->controls,slaves,attempted);
 return ok;
}
/* n samples (n>=1) into bit-pattern outputs. Returns 0 at the first
 * nonfinite filter value, with state committed as the per-sample path. */
int original_chicago_voice_span(original_voice_gain *vg,original_oscillators *ph,const original_prepared_oscillators *p,
 original_ramp *noise_gain,uint32_t *noise_state,float vco_gain,original_chicago_mp4 *f,uint32_t *left,uint32_t *right,uint32_t n){
 if(!p->ready||!f->prepared)return 0;
 uint32_t phase[4]={ph->phase[0],ph->phase[1],ph->phase[2],ph->phase[3]};
 original_ramp og[4]={vg->oscillator[0],vg->oscillator[1],vg->oscillator[2],vg->oscillator[3]},st[2]={vg->stereo[0],vg->stereo[1]};
 original_ramp ngr=*noise_gain,ra=f->a,re=f->e,rr=f->resonance;uint32_t ns=*noise_state;
 float H[5]={f->history[0],f->history[1],f->history[2],f->history[3],f->history[4]};
 ck_slaves slaves;
 for(unsigned i=0;i<2;i++){
  float b=(float)p->controls.increment[2*i+1]*0x1p-31f;
  slaves.base[i]=b;slaves.plain[i]=ck_phase_delta(ck_clamp(b,-1,1)*0x1p31f);
 }
 int ok=1;
 int filter_held=ramp_held(&ra)&&ramp_held(&re)&&ramp_held(&rr)&&
    ramp_held(&og[0])&&ramp_held(&og[1])&&ramp_held(&og[2])&&ramp_held(&og[3])&&
    ramp_held(&st[0])&&ramp_held(&st[1])&&ramp_held(&ngr);
 {
  unsigned single=filter_held?4:single_moving_oscillator(og,p);
  int parked=filter_held || (ck_fixed(og[0])&&ck_fixed(og[1])&&ck_fixed(og[2])&&ck_fixed(og[3])&&ck_fixed(st[0])&&ck_fixed(st[1])&&ck_fixed(ngr));
  if(parked && (single==0 || single==2) && og[single].current!=0.0f &&
     p->controls.shape[single]>=2 && p->controls.width[single]==.5f && p->correction[single]>=.25f)
   ok=ck_pulse_span(p,&slaves,single,og[single].current,ngr.current,st[0].current,st[1].current,
                    vco_gain,phase,&ns,H,&ra,&re,&rr,left,right,n);
  else
  for(uint32_t k=0;k<n;k++)if(!ck_step(p,&slaves,parked,filter_held,vco_gain,phase,og,st,&ngr,&ns,H,&ra,&re,&rr,left+k,right+k)){ok=0;break;}
 }
 for(unsigned i=0;i<4;i++){ph->phase[i]=phase[i];vg->oscillator[i]=og[i];}
 vg->stereo[0]=st[0];vg->stereo[1]=st[1];*noise_gain=ngr;*noise_state=ns;
 f->a=ra;f->e=re;f->resonance=rr;for(unsigned i=0;i<5;i++)f->history[i]=H[i];
 return ok;
}
int original_chicago_voice_kernel(original_voice_gain *vg,original_oscillators *ph,const original_prepared_oscillators *p,
 original_ramp *noise_gain,uint32_t *noise_state,float vco_gain,original_chicago_mp4 *f,float *left,float *right){
 uint32_t l,r;if(!original_chicago_voice_span(vg,ph,p,noise_gain,noise_state,vco_gain,f,&l,&r,1))return 0;
 *left=original_math_float(l);*right=original_math_float(r);return 1;
}
