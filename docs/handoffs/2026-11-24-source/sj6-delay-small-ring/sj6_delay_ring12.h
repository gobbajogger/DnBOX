/* SPDX-License-Identifier: GPL-3.0-only
 * Opt-in finite host DSP experiment; not a product allocator or profile. */
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

enum { SJ6_RING12_MIN=2, SJ6_RING12_MAX=5514, SJ6_RING12_FRAME_BYTES=3 };
typedef struct { uint8_t *data; unsigned frames; } sj6_ring12;

/* Bind two stereo lines atomically to caller-owned private workspace.
 * Refuse short capacity, already-bound state and excess profile; exact size
 * for each owner's half, no sample/other workspace allocation. */
static inline int sj6_ring12_pair(sj6_ring12 owners[2],void *memory,size_t n,unsigned frames)
{
    if(!owners || !memory || owners[0].data || owners[1].data ||
       frames<SJ6_RING12_MIN || frames>SJ6_RING12_MAX)return 0;
    size_t half=SJ6_RING12_FRAME_BYTES*(size_t)frames;
    if(n!=2u*half)return 0;
    unsigned char *b=memory;
    if((uintptr_t)b>UINTPTR_MAX-n)return 0;
    const uintptr_t a=(uintptr_t)owners, p=(uintptr_t)b;
    if(a>UINTPTR_MAX-2u*sizeof *owners || !(a+2u*sizeof *owners<=p || p+n<=a))return 0;
    memset(b,0,n);
    owners[0]=(sj6_ring12){b,frames};
    owners[1]=(sj6_ring12){b+half,frames};
    return 1;
}
/* Signed logarithmic Q19 segmentation retaining 7 magnitude bits at high
 * exponents. No log/pow/tables or sequential decoder dependency.
 * Each 12-bit word: sign : shift[4] : magnitude[7].
 * Headroom ±7.9375 for the feedback state; values beyond are outside this test
 * profile and never written as wraps. Non-finite input is mapped safely;
 * the owning runtime may elect to reject the audio profile entirely. */
static inline unsigned sj6_ring12_encode(float x)
{
    if(!isfinite(x))return 0;
    const unsigned sign=x<0.f?2048u:0u;
    float abs=fabsf(x);
    if(abs>=7.9375f)return sign|2047u;
    unsigned q=(unsigned)(abs*524288.f+.5f);
    if(q<128u)return q ? sign|q : 0;
    unsigned bits=31u-(unsigned)__builtin_clz(q);
    unsigned shift=bits-6u;
    unsigned mag=(q+(1u<<(shift-1u)))>>shift;
    if(mag>=128u){mag>>=1u;shift++;}
    if(shift>15u)return sign|2047u;
    return sign|(shift<<7u)|mag;
}
static inline float sj6_ring12_decode(unsigned q)
{
    const unsigned mag=(q&127u)<<((q>>7)&15u);
    const float f=(float)mag*(1.f/524288.f);
    return q&2048u ? -f:f;
}
static inline float sj6_ring12_read(const uint8_t *p,unsigned frame,unsigned right)
{
    p+=SJ6_RING12_FRAME_BYTES*(size_t)frame;
    return sj6_ring12_decode(right?(p[1]>>4)|(p[2]<<4):p[0]|((p[1]&15u)<<8));
}
static inline void sj6_ring12_write(uint8_t *p,unsigned frame,float left,float right)
{
    p+=SJ6_RING12_FRAME_BYTES*(size_t)frame;
    unsigned a=sj6_ring12_encode(left), b=sj6_ring12_encode(right);
    p[0]=(uint8_t)a; p[1]=(uint8_t)((a>>8)|(b<<4)); p[2]=(uint8_t)(b>>4);
}
