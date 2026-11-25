/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include <stdint.h>
static inline int sj6_msb64(uint64_t n) {
    uint32_t hi=(uint32_t)(n>>32), lo=(uint32_t)n;
    return hi ? 63-__builtin_clz(hi) : lo ? 31-__builtin_clz(lo) : -1;
}
static inline uint64_t sj6_qshift(uint64_t n,int shift,unsigned *tail) {
    if(shift>=0) return n<<shift;
    unsigned r=(unsigned)-shift;
    uint64_t q=r<64 ? n>>r:0;
    *tail=n!=(r<64?q<<r:0);
    return q;
}
/* Exact binary32 endpoint of the original Double sum, for positive
 * normalized base and depth and integer triangle sample -64..64. */
static inline float sj6_lfo_port(float base,float depth,int sample) {
    union {float f; uint32_t u;} b={.f=base}, d={.f=depth},out;
    if(!sample||!d.u) return base;
    unsigned be=b.u>>23, de=d.u>>23;
    uint32_t bm=(b.u&0x7fffffu)|(be?0x800000u:0u);
    uint32_t dm=((d.u&0x7fffffu)|(de?0x800000u:0u))*(uint32_t)(sample<0?-sample:sample);
    int bx=be?(int)be-150:-149,dx=(de?(int)de-150:-149)-7;
    int bh=bm?bx+sj6_msb64(bm):-200, dh=dx+sj6_msb64(dm);
    int scale=60-(bh>dh?bh:dh);
    unsigned bt=0, dt=0;
    uint64_t bv=bm?sj6_qshift(bm,bx+scale,&bt):0;
    uint64_t dv=sj6_qshift(dm,dx+scale,&dt);
    int64_t pl=(int64_t)bv+(sample<0?-(int64_t)dv:(int64_t)dv);
    if(pl<=0) return 0.f;
    uint64_t q=(uint64_t)pl;
    int sticky=(int)bt+(sample<0?-(int)dt:(int)dt);
    int n=sj6_msb64(q)-52;
    if(n>0) {
        uint64_t top=q>>n, rem=q-(top<<n), half=UINT64_C(1)<<(n-1);
        top+=rem>half||(rem==half&&(sticky>0||(sticky==0&&(top&1u))));
        q=top<<n;
    }
    int h=sj6_msb64(q), e=h-scale;
    if(e>=0) return 1.f;
    n=e>=-126 ? h-23 : scale-149;
    uint64_t top;
    if(n>=64) return 0.f;
    if(n<=0) top=q<<-n;
    else {
        top=q>>n;uint64_t rem=q-(top<<n), half=UINT64_C(1)<<(n-1);
        top+=rem>half||(rem==half&&(top&1u));
    }
    out.u=(e>=-126?(uint32_t)(e+126)<<23:0)+(uint32_t)top;
    return out.f;
}
