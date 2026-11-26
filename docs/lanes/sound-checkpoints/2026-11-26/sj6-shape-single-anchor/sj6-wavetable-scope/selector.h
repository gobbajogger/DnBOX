#include <stdint.h>
/* Temporary algebraic replacement for exact witnessed recurrent row halving.
 * Finite nonnegative phase increment; no effect on oscillator/data read. */
typedef struct { unsigned row; float blend; } sj6_band;
static inline unsigned sj6_as_uint(float x){union{float f;uint32_t u;} m={.f=x};return m.u;}
static inline float sj6_as_float(unsigned x){union{float f;uint32_t u;} m={.u=x};return m.f;}
static inline sj6_band sj6_wavetable_band(float step)
{
    float x=64.f*step;
    unsigned u=sj6_as_uint(x),v=sj6_as_uint(.49f),b=0;
    if(u>=v){
        b=((u-v)>>23)+1u;
        if(b>=6u)return (sj6_band){6,0.f};
        x=sj6_as_float(u-(b<<23));
    }
    float m=x>.4f?(x-.4f)/.09f:0.f;
    if(m>1.f)m=1.f;
    return (sj6_band){b,m};
}
