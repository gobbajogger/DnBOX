#ifndef DNBX_S1_WIDEN_BFP_H
#define DNBX_S1_WIDEN_BFP_H
#include <stdint.h>
#include <math.h>
#include <stddef.h>
#include <string.h>

/* Host measurements only. Sample spacing + channel policy same as source.
   Sixteen Float32 pending samples can finalize a block long before reading.
   Whole groups are committed only after their prior turn has been read.
   Block exponent decoded with exact power of two; no preset/gain fit. */
enum { S1_W_DELAY=2205, S1_W_BLOCK=16,
       S1_W_GROUPS=(S1_W_DELAY+S1_W_BLOCK-1)/S1_W_BLOCK };
typedef struct { int8_t mantissa[S1_W_BLOCK], e; } s1_widen_bfp8_group;
typedef struct {
    s1_widen_bfp8_group group[S1_W_GROUPS];
    float stage[S1_W_BLOCK], stage_peak;
    uint16_t write, read;
    uint16_t age;
} s1_widen_bfp8;

static inline void s1_widen_bfp8_clear(s1_widen_bfp8 *w)
{
    memset(w,0,sizeof *w);
    w->read=S1_W_GROUPS*S1_W_BLOCK-S1_W_DELAY;
}
/* IEEE binary32 finite renderer, without exp/log/round library
   providers. Stamp nearest-even explicitly for independent rounding modes. */
static inline float s1_widen_pow2(int e)
{
    const union {float f;uint32_t bits;} u={.bits=(uint32_t)(e+127)<<23};
    return u.f;
}
static inline int s1_widen_quant(float v)
{
    int q=(int)v;
    const float diff=v-(float)q;
    if(diff>.5f||(diff==.5f&&(q&1)))++q;
    if(diff<-.5f||(diff==-.5f&&(q&1)))--q;
    if(q>127)q=127;
    if(q< -128)q=-128;
    return q;
}
static inline float s1_widen_bfp8_next(s1_widen_bfp8 *w,float in,float width)
{
    const unsigned limit=S1_W_GROUPS*S1_W_BLOCK;
    const s1_widen_bfp8_group *rd=&w->group[w->read/S1_W_BLOCK];
    const float delayed=w->age<S1_W_DELAY?0.0f:
        (float)rd->mantissa[w->read%S1_W_BLOCK]*s1_widen_pow2(rd->e);
    const unsigned k=w->write%S1_W_BLOCK;
    w->stage[k]=in;
    const float magnitude=in<0.f?-in:in;
    if(magnitude>w->stage_peak)w->stage_peak=magnitude;
    if(k+1==S1_W_BLOCK) {
        const union {float f;uint32_t bits;} u={.f=w->stage_peak};
        int exponent=u.bits?((int)(u.bits>>23)-133):-7;
        if(exponent< -125)exponent=-125;
        const float inv=s1_widen_pow2(-exponent);
        s1_widen_bfp8_group *g=&w->group[w->write/S1_W_BLOCK];
        g->e=(int8_t)exponent;
        for(unsigned i=0;i<S1_W_BLOCK;i++){
            g->mantissa[i]=(int8_t)s1_widen_quant(w->stage[i]*inv);
        }
        w->stage_peak=0.f;
    }
    if(++w->write==limit)w->write=0;
    if(++w->read==limit)w->read=0;
    if(w->age<S1_W_DELAY)++w->age;
    return width*delayed+(1.f-width)*in;
}
#endif
