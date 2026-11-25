/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include <stdint.h>
/* Build9 cutoff: round64(base + round64(square * span)), narrow to
 * binary32. square >= 0, base>=20, 20<=base+span<=20000.
 * The product of a binary32 significand and a uint16 is exact in
 * binary64. Positive Q49 holds all retained double bits for base>=20;
 * a sticky bit records any smaller fraction for the double rounding. */
static inline float sj6_cutoff_port_round(float square,uint32_t base,uint32_t span)
{
    union {float f;uint32_t u;} s={.f=square}, result;
    unsigned k=s.u>>23;
    uint64_t p=(uint64_t)((s.u&0x7fffffu)|(k?0x800000u:0u))*span;
    int shift=k ? (int)k-101 : -100;
    uint64_t q=(uint64_t)base<<49;
    unsigned sticky=0;
    if(shift>=0) q+=p<<(unsigned)shift;
    else{
        unsigned r=(unsigned)-shift;
        uint64_t retained=r>=64u?0:p>>r;
        q+=retained;
        sticky=p!=(r>=64u?0:retained<<r);
    }
    unsigned e=31u-(unsigned)__builtin_clz((uint32_t)(q>>49));
    unsigned d=e-3;
    uint64_t dv=q>>d, rem=q-(dv<<d), half=UINT64_C(1)<<(d-1);
    dv+=rem>half||(rem==half&&(sticky||(dv&1u)));
    q=dv<<d;
    e=31u-(unsigned)__builtin_clz((uint32_t)(q>>49));
    d=e+26;
    uint64_t fv=q>>d;
    rem=q-(fv<<d);half=UINT64_C(1)<<(d-1);
    fv+=rem>half||(rem==half&&(fv&1u));
    result.u=((e+126u)<<23)+(uint32_t)fv; /* carry into the next exponent naturally */
    return result.f;
}
