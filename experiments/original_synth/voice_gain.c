/* SPDX-License-Identifier: GPL-3.0-only */
#include "voice_gain.h"
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
static const float control_span_inverse[64] = {
    0x1p+0f,     0x1p-1f,     0x1.555556p-2f,     0x1p-2f,
    0x1.99999ap-3f,     0x1.555556p-3f,     0x1.24924ap-3f,     0x1p-3f,
    0x1.c71c72p-4f,     0x1.99999ap-4f,     0x1.745d18p-4f,     0x1.555556p-4f,
    0x1.3b13b2p-4f,     0x1.24924ap-4f,     0x1.111112p-4f,     0x1p-4f,
    0x1.e1e1e2p-5f,     0x1.c71c72p-5f,     0x1.af286cp-5f,     0x1.99999ap-5f,
    0x1.861862p-5f,     0x1.745d18p-5f,     0x1.642c86p-5f,     0x1.555556p-5f,
    0x1.47ae14p-5f,     0x1.3b13b2p-5f,     0x1.2f684cp-5f,     0x1.24924ap-5f,
    0x1.1a7b96p-5f,     0x1.111112p-5f,     0x1.08421p-5f,     0x1p-5f,
    0x1.f07c2p-6f,     0x1.e1e1e2p-6f,     0x1.d41d42p-6f,     0x1.c71c72p-6f,
    0x1.bacf92p-6f,     0x1.af286cp-6f,     0x1.a41a42p-6f,     0x1.99999ap-6f,
    0x1.8f9c18p-6f,     0x1.861862p-6f,     0x1.7d05f4p-6f,     0x1.745d18p-6f,
    0x1.6c16c2p-6f,     0x1.642c86p-6f,     0x1.5c9882p-6f,     0x1.555556p-6f,
    0x1.4e5e0ap-6f,     0x1.47ae14p-6f,     0x1.414142p-6f,     0x1.3b13b2p-6f,
    0x1.3521dp-6f,     0x1.2f684cp-6f,     0x1.29e412p-6f,     0x1.24924ap-6f,
    0x1.1f7048p-6f,     0x1.1a7b96p-6f,     0x1.15b1e6p-6f,     0x1.111112p-6f,
    0x1.0c9714p-6f,     0x1.08421p-6f,     0x1.041042p-6f,     0x1p-6f,

};
static float duration_inverse(uint32_t D){return D>=1u&&D<=64u?control_span_inverse[D-1u]:1.0f/(float)D;}
static float clamp(float x,float lo,float hi){return x<lo?lo:x>hi?hi:x;}
static void target(original_ramp *r,float t,float inverse){r->target=t;r->delta=inverse*(t-r->current);}
static float advance(original_ramp *r){
 float n=r->current+r->delta;
 /* Original strict comparisons and XOR selection. In particular, a rounded
  * zero delta does not imply an unconditional assignment of the target. */
 if((r->delta>0)!=(r->target>n))n=r->target;
 r->current=n;return n;
}
void original_voice_gain_init(original_voice_gain *s){if(s)memset(s,0,sizeof *s);}
void original_voice_gain_miami_full_note(original_voice_gain *s){if(s)memset(s->stereo,0,sizeof s->stereo);}
/* Chicago builds a zero-modulation oscillator/pan view; Miami supplies its
 * stored modulation controls. Live envelope, gain and velocity are per voice. */
static inline __attribute__((always_inline)) int prepare_gain(original_voice_gain *s,
 const original_voice_gain_controls *c,const int32_t active[4],uint32_t D,int chicago,
 const float level[4],float output,float pan,uint8_t curve,uint8_t ms20,
 float envelope,float mod_gain,float velocity,float filter_gain,uint8_t unison_flag){
 const float mod_level=chicago?0.0f:c->mod_level;
 const float mod_pan=chicago?0.0f:c->mod_pan;
 const float spread=chicago?0.0f:c->spread;
 if(curve>2||ms20>1)return 0;
 if(!original_binary32_isfinite(mod_level)||!original_binary32_isfinite(mod_gain)||!original_binary32_isfinite(pan)||!original_binary32_isfinite(mod_pan))return 0;
 if(!(envelope>=-1&&envelope<=1&&output>=0&&output<=1&&velocity>=0&&velocity<=1&&spread>=0&&spread<=1&&filter_gain>=1&&filter_gain<=4)|| (ms20&&filter_gain!=1))return 0;
 for(unsigned i=0;i<4;i++)if(!original_binary32_isfinite(level[i])||(!chicago&&!original_binary32_isfinite(c->mod_specific[i]))||!(s->oscillator[i].current>=0&&s->oscillator[i].current<=ORIGINAL_OSCILLATOR_GAIN_MAX))return 0;
 for(unsigned i=0;i<2;i++)if(!(s->stereo[i].current>=0&&s->stereo[i].current<=4))return 0;
 float inverse=duration_inverse(D);
 for(unsigned i=0;i<4;i++) {
  if(!active[i]) {s->oscillator[i].target=0;s->oscillator[i].delta=(-s->oscillator[i].current)*inverse;continue;}
  float q=level[i]+mod_level;q=q+(chicago?0.0f:c->mod_specific[i]);q=clamp(q,0,1);
  if(curve==0)q=q*q;
  else if(curve==2){float u=1-q;q=ref_fma(-u,u,1);}
  /* Chicago's inverted-quadratic curve reaches unity. The 1.3333
   * normalization belongs to the other route, including Miami curve 0. */
  float volume=curve==2?q:q*ORIGINAL_OSCILLATOR_GAIN_MAX;
  target(&s->oscillator[i],volume,inverse);
 }
 float a=clamp(ref_fma(mod_gain,.5f,.5f),0,1),g=filter_gain*(a*a);
 g=(envelope*envelope)*g;g=output*g;g=g*(velocity*velocity);g=clamp(g,0,2);
 float p=clamp(ref_fma(mod_pan,2,pan),-1,1),u=p+1,v=2-u;
 float left=g*original_binary32_min_one_finite(v*v),right=g*original_binary32_min_one_finite(u*u);
 /* Miami unison split. Chicago selected mono uses flag1 and no split;
  * callers pass ms20=0, flag255 to retain its original full L/R weights. */
 if(unison_flag!=255) {
  float l=.5f,r=.5f;
  if(unison_flag==0){l=ref_fma(-spread,.5f,.5f);r=1-l;}
  else if(unison_flag==2){r=ref_fma(-spread,.5f,.5f);l=1-r;}
  left=l*left;right=r*right;
 }
 if(ms20){left=left+left;right=right+right;}
 target(&s->stereo[0],left,inverse);target(&s->stereo[1],right,inverse);return 1;
}
int original_voice_gain_prepare(original_voice_gain *s,const original_voice_gain_controls *c,const int32_t active[4],uint32_t D){
 if(!s||!c||!active||!D||D>2147482880u)return 0;
 return prepare_gain(s,c,active,D,0,c->level,c->output,c->pan,c->curve,c->ms20,c->envelope,c->mod_gain,c->velocity,c->filter_gain,c->unison_flag);
}
int original_voice_gain_prepare_chicago(original_voice_gain *s,const original_chicago_voice_gain_controls *c,const int32_t active[4],uint32_t D,
                                       float envelope,float mod_gain,float velocity,float filter_gain){
 if(!s||!c||!active||!D||D>2147482880u)return 0;
 return prepare_gain(s,0,active,D,1,c->level,c->output,c->pan,c->curve,c->ms20,envelope,mod_gain,velocity,filter_gain,c->unison_flag);
}
int original_voice_gain_prepare_miami(original_voice_gain *s,const original_voice_gain_controls *c,const int32_t active[4],uint32_t D,
                                     float envelope,float mod_gain,float velocity,uint8_t unison_flag){
 if(!s||!c||!active||!D||D>2147482880u)return 0;
 return prepare_gain(s,c,active,D,0,c->level,c->output,c->pan,c->curve,c->ms20,envelope,mod_gain,velocity,c->filter_gain,unison_flag);
}

int original_voice_gain_oscillator(original_voice_gain *s,original_oscillators *phase,const original_prepared_oscillators *p,unsigned sync,float *out){
 if(!s)return 0;
 float gain[4];for(unsigned i=0;i<4;i++)gain[i]=s->oscillator[i].current;
 if(!original_oscillator_sample_gains(phase,p,gain,sync,out))return 0;
 for(unsigned i=0;i<4;i++)(void)advance(&s->oscillator[i]);
 return 1;
}
int original_voice_gain_stereo(original_voice_gain *s,float filtered,float *left,float *right){
 if(!s||!left||!right||left==right||!original_binary32_isfinite(filtered))return 0;
 float l=advance(&s->stereo[0]),r=advance(&s->stereo[1]);*left=filtered*l;*right=filtered*r;return 1;
}
