#ifndef SJ6_APPLIED_FX_H
#define SJ6_APPLIED_FX_H
#include <stdint.h>
#include <math.h>
#include <string.h>
enum { SJ6_G_UNKNOWN, SJ6_G_CONNECTED, SJ6_G_BYPASSED };
enum { SJ6_G_MIX=1, SJ6_G_GAIN=2, SJ6_G_109=4, SJ6_G_KERNEL=8 };
/* Applied words, not UI display words. Graph + kernel provenance are trusted
 * input from an instrument readback, never a user-settable bank/profile flag. */
typedef struct {
 float mix, booster, rate_word;
 uint8_t graph, known;
} sj6_applied_fx;
static inline int sj6_fx_unit(float f) {return f>=0.f && f<=1.f;}
static inline void sj6_fx_legacy(sj6_applied_fx *s){memset(s,0,sizeof *s);}
static inline int sj6_fx_from_readback(sj6_applied_fx *s,unsigned route,
                                     float mix,float gain,float raw109,
                                     unsigned exact_kernel) {
 sj6_fx_legacy(s);
 if((route!=SJ6_G_CONNECTED&&route!=SJ6_G_BYPASSED) ||
    !sj6_fx_unit(mix)||!sj6_fx_unit(raw109)||!(gain==1.f||gain==1.3f)||
    exact_kernel>1)return 0;
 s->graph=route;s->mix=mix;s->booster=gain;s->rate_word=raw109;
 s->known=SJ6_G_MIX|SJ6_G_GAIN|SJ6_G_109|(exact_kernel?SJ6_G_KERNEL:0);
 return 1;
}
/* Consume only accepted APPLIED writes, in original order. Raw bank values are
 * never passed here without source-proven application order for that load. */
static inline int sj6_fx_write(sj6_applied_fx *s,unsigned id,float value) {
 if(!sj6_fx_unit(value))return 0;
 switch(id) {
 case 107:
  if(value!=0.f&&value!=1.f)return 0;
  s->mix=value==1.f?.3f:0.f;
  s->booster=value==1.f?1.3f:1.f;
  s->known|=SJ6_G_MIX|SJ6_G_GAIN;
  return 1;
 case 108:
  s->mix=(float)pow((double)value,1.5);
  s->known|=SJ6_G_MIX;
  return 1;
 case 109:
  s->rate_word=value;
  s->known|=SJ6_G_109;
  /* Known dry kernel only at its verified zero-mode, not a reconstruction of
   * arbitrary decimation. A physical trace is needed again after modulation. */
  s->known&=~SJ6_G_KERNEL;
  return 1;
 default:return 0;
 }
}
static inline int sj6_fx_zero_ready(const sj6_applied_fx *s){
 return s->graph==SJ6_G_CONNECTED && s->known==15u &&
        s->mix==0.f && s->booster==1.f && s->rate_word==0.f;
}
#endif
