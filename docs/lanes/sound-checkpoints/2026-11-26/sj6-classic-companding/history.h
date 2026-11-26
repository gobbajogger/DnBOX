#include <stdint.h>
/* Same-size signed history; low Q15 words reconstruct exactly.
 * Width is Q15 units: 4096 cells in each of 8 linear segments. */
static const unsigned ch_base[8]={0,4096,12288,28672,61440,126976,258048,520192};
static const unsigned char ch_shift[8]={0,1,2,3,4,5,6,9};
static inline uint16_t ch_encode(float x)
{
    const unsigned sign=x<0.f?32768u:0u;
    const float mag=(x<0.f?-x:x)*32767.f;
    if(!(mag>=0.f))return 0;
    unsigned n=mag>=2617344.f?2617343u:(unsigned)mag;
    unsigned b=n<61440u?(n<12288u?(n<4096u?0u:1u):(n<28672u?2u:3u)):
                          (n<258048u?(n<126976u?4u:5u):(n<520192u?6u:7u));
    return (uint16_t)(sign|(b<<12)|((n-ch_base[b])>>ch_shift[b]));
}
static inline float ch_word(uint16_t word)
{
    const unsigned b=(word>>12)&7;
    const unsigned n=ch_base[b]+((word&4095u)<<ch_shift[b]);
    return word&32768u?-(float)n:(float)n;
}
