#ifndef SJ6_CRUSH_CONVERTER
#define SJ6_CRUSH_CONVERTER
#include "base.h"
#include "correct.generated.h"
/* Cold/control-rate only. Input is the canonical public binary32 knob word,
 * not a normalized roundtrip from an arbitrary continuous source. */
static int sj6_crush_percent(unsigned id,unsigned q,float incoming,float *out) {
 if(q>127u || (id!=108u && id!=109u) || !out)return 0;
 fm_bits_t got={.f=incoming}, grid={.f=unit_control(q)};
 if(got.u!=grid.u)return 0;
 const unsigned j=id-108u, bit=q*3u, off=bit>>3, shift=bit&7u;
 unsigned packed=sj6_fx_fix[j][off];
 if(shift>5u)packed|=(unsigned)sj6_fx_fix[j][off+1u]<<8u;
 const unsigned fix=(packed>>shift)&7u;
 fm_bits_t y;
 y.u=(j?sj6_base109(q):sj6_base108(q))+fix-(j?1u:2u);
 *out=y.f;
 return 1;
}
/* Applied physical out tuple. Connected provenance is external; a UI
 * gate edit never fills it or restores an invalidated proof. */
enum { SJ6_WET=1, SJ6_CADENCE=2, SJ6_BOOSTER=4, SJ6_ROUNDING_ZERO=8 };
typedef struct {
 float wet_pct,cadence_pct,booster;
 uint8_t wet_seen,cadence_seen,booster_seen;
} sj6_crush_applied;
static int sj6_crush_commit(sj6_crush_applied *s,unsigned id,unsigned q,float raw){
 float result;
 if(!s)return 0;
 if(id==107u){
   fm_bits_t input={.f=raw};
   if((q!=0u&&q!=127u)||input.u!=(q==0u?0u:0x3f800000u))return 0;
   s->wet_pct=q?30.f:0.f;s->booster=q?1.3f:1.f;s->wet_seen=s->booster_seen=1;
   return SJ6_WET|SJ6_BOOSTER;
 }
 if(!sj6_crush_percent(id,q,raw,&result))return 0;
 if(id==108u){s->wet_pct=result;s->wet_seen=1;}
 else{s->cadence_pct=result;s->cadence_seen=1;}
 return id==108u?SJ6_WET:(SJ6_CADENCE|SJ6_ROUNDING_ZERO);
}
#endif
