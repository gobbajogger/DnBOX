#ifndef SJ6_CLASSIC_FAST_HISTORY_H
#define SJ6_CLASSIC_FAST_HISTORY_H
#include <stdint.h>
/* 4×8192 magnitude cells. Top reconstruction 2400000/32768 ≈73.0. */
static inline uint16_t ch_encode(float x)
{
    const float mag=(x<0.f?-x:x)*32767.f;
    if(!(mag>=0.f))return 0;
    const unsigned n=mag>=2400256.f?2400255u:(unsigned)mag;
    const unsigned code=n<40960?(n<8192?n:8192+((n-8192)>>2)):
                                (n<303104?16384+((n-40960)>>5):24576+((n-303104)>>8));
    return (uint16_t)((x<0.f?32768u:0u)|code);
}
static inline float ch_word(uint16_t word)
{
    const unsigned code=word&32767u;
    const unsigned lo=code&8191u;
    const unsigned n=code<16384?(code<8192?lo:8192+(lo<<2)):
                                  (code<24576?40960+(lo<<5):303104+(lo<<8));
    return word&32768u?-(float)n:(float)n;
}
#endif
