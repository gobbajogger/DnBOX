#include "juno6_lfo_phase.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>

static unsigned count;
static void check(uint32_t bits)
{
    float x;
    memcpy(&x,&bits,sizeof x);
    assert(sj6_lfo_finite(x) == !!isfinite(x));
    uint64_t want = (x >= 0.f && x < 0x1p64f) ? (uint64_t)x : 0;
    assert(sj6_lfo_unsigned_phase(x)==want);
    ++count;
}
int main(void)
{
    for(unsigned e=0;e<256;++e) {
        for(unsigned s=0;s<2;++s) {
            uint32_t base=(s<<31)|(e<<23);
            for(unsigned h=0;h<0x800000;h+=8191) check(base|h);
            check(base|0x7ffffe); check(base|0x7fffff);
        }
    }
    uint32_t x=1;
    for(unsigned i=0;i<2000000;++i) { x=1664525u*x+1013904223u; check(x); }
    printf("PASS binary32 phase truncation: %u words match finite classification and in-range C cast or defined zero out of domain\n",count);
}
