#ifndef SJ6_DIAGNOSTIC_SHORT_RING_H
#define SJ6_DIAGNOSTIC_SHORT_RING_H
#include <stdint.h>
#include <string.h>
static inline uint32_t sj6_float_word(float x) {uint32_t bits;memcpy(&bits,&x,4);return bits;}
static inline float sj6_word_float(uint32_t bits) {float x;memcpy(&x,&bits,4);return x;}
/* Diagnostic signed E4M11; first positive normal is 2^-8, upper finite
 * limit 128-2^-5. For a sample strictly below 128, rounding can reach the
 * saturation endpoint. Quiet values use an exact 2^-19 grid. */
static inline uint16_t sj6_hist_encode(float x)
{
    uint32_t word=sj6_float_word(x), sign=(word>>16)&0x8000u, mag=word&0x7fffffffu;
    uint32_t q;
    if (mag<0x3b800000u) {
        uint32_t shift=131u-(mag>>23); /* round (1.M) * 2^(e-4) */
        if (shift>=25u) q=0;
        else {
            uint32_t sig=(mag&0x7fffffu)|0x800000u, half=1u<<(shift-1u);
            q=(sig+half-1u+((sig>>shift)&1u))>>shift;
        }
    } else if (mag<0x43000000u) {
        q=(mag+0x7ffu+((mag>>12)&1u)-0x3b000000u)>>12;
        if (q>32767u) q=32767u;
    } else {
        /* Out-of-support finite overdrive clips; nonfinite data gives silence. */
        q=mag>=0x7f800000u ? 0u:32767u;
    }
    return (uint16_t)(sign|q);
}
static inline float sj6_hist_decode(uint16_t x)
{
    uint32_t mag=x&0x7fffu, sign=(uint32_t)(x&0x8000u)<<16;
    if (mag<2048u) return sj6_word_float(sj6_float_word((float)mag*0x1p-19f)|sign);
    return sj6_word_float((0x3b000000u+(mag<<12))|sign);
}
#endif
