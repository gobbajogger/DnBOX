/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include <stdint.h>
/* Target-specific exact original fmaf for the verified SMK domains:
 * factor = one of 128 original rounded (Double(u) * 1.1) detents,
 * velocity = Float32(note_velocity / 127), offset = Float32(1-factor).
 * Every nonzero operand has a binary32 exponent >= -10, and each nonzero
 * product operand exponent >= -7. The FP product plus offset therefore
 * fits an exact signed Q62 before one nearest-even Float32 rounding.
 */
static inline float sj6_velocity_fma(float factor,float velocity,float offset)
{
    union { float f; uint32_t u; } a={.f=factor}, v={.f=velocity}, b={.f=offset}, out;
    uint32_t av=a.u&0x7fffffff, vv=v.u&0x7fffffff,bv=b.u&0x7fffffff;
    if(!av || !vv) return offset;
    const unsigned ae=av>>23,ve=vv>>23,be=bv>>23;
    if(ae<120u||ae>127u||ve<120u||ve>127u||(bv&&be<117u))return offset;
    uint64_t qprod=(uint64_t)((av&0x7fffffu)|0x800000u)*((vv&0x7fffffu)|0x800000u);
    qprod<<=ae+ve-238u;
    uint64_t qoffset=bv?(uint64_t)((bv&0x7fffffu)|0x800000u)<<(be-88u):0;
    int64_t q=(int64_t)qprod+((b.u>>31)?-(int64_t)qoffset:(int64_t)qoffset);
    if(!q)return 0.f;
    const uint64_t p=q<0?(uint64_t)-q:(uint64_t)q;
    uint32_t hi=(uint32_t)(p>>32);
    unsigned h=hi?63u-(unsigned)__builtin_clz(hi):31u-(unsigned)__builtin_clz((uint32_t)p);
    uint64_t f;
    if(h<=23) f=p<<(23u-h);
    else{
        unsigned r=h-23u;
        f=p>>r;
        uint64_t rem=p-(f<<r),half=UINT64_C(1)<<(r-1u);
        f+=rem>half||(rem==half&&(f&1u));
    }
    out.u=(q<0?0x80000000u:0u)+((h+64u)<<23)+(uint32_t)f;
    return out.f;
}
