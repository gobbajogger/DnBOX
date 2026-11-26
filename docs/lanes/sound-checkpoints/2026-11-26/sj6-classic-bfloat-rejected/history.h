#include <string.h>
#include <stdint.h>
static inline uint16_t sj6_hist_encode(float x)
{
    uint32_t u;memcpy(&u,&x,4);
    /* Valid finite DSP; magnitude over 2^127 or nonfinite is not in
       the admitted typed source. Never read a nonfinite as audio. */
    return (uint16_t)((u+0x7fffu+((u>>16)&1u))>>16);
}
static inline float sj6_hist_decode(uint16_t x)
{
    uint32_t u=(uint32_t)x<<16;float f;memcpy(&f,&u,4);return f;
}
