/* Local timing fixture. No production call site. Apply only to emulator image. */
#include <stdint.h>
#include "leaf.c"
#ifndef HP_PROBE_TICKS
#error provide a deterministic/emulated clock source
#endif
#define SJ6_HP_PROBE_REPS 32
#define SJ6_HP_PROBE_FRAMES 32
struct sj6_hp_probe_result { uint32_t magic, digest, ticks, ticks_min, count; };
volatile struct sj6_hp_probe_result sj6_hp_probe_result;
static uint32_t sj6_hp_probe_word(float x)
{
    union {float f;uint32_t u;} n; n.f=x; return n.u;
}
__attribute__((noinline))
static void sj6_hp_probe(void)
{
    sj6_zero_fx_stereo s={0};
    float l[SJ6_HP_PROBE_FRAMES],r[SJ6_HP_PROBE_FRAMES];
    uint32_t x=0x48276db3u, digest=2166136261u;
    uint32_t ticks=0,min=UINT32_MAX;
    for(unsigned j=0;j<SJ6_HP_PROBE_REPS;j++) {
        for(unsigned i=0;i<SJ6_HP_PROBE_FRAMES;i++) {
            x=x*1664525u+1013904223u;
            l[i]=(float)((int32_t)(x>>16)-32768)*0x1p-16f;
            x=x*1664525u+1013904223u;
            r[i]=(float)((int32_t)(x>>16)-32768)*0x1p-16f;
        }
        const uint32_t a=HP_PROBE_TICKS();
        sj6_hp_crusher_44100_span(&s,l,r,SJ6_HP_PROBE_FRAMES);
        const uint32_t n=HP_PROBE_TICKS()-a;
        ticks+=n;if(n<min)min=n;
        for(unsigned i=0;i<SJ6_HP_PROBE_FRAMES;i++) {
            digest=(digest^sj6_hp_probe_word(l[i]))*16777619u;
            digest=(digest^sj6_hp_probe_word(r[i]))*16777619u;
        }
    }
    sj6_hp_probe_result.digest=digest;
    sj6_hp_probe_result.ticks=ticks;
    sj6_hp_probe_result.ticks_min=min;
    sj6_hp_probe_result.count=SJ6_HP_PROBE_REPS*SJ6_HP_PROBE_FRAMES;
    sj6_hp_probe_result.magic=0x48505a30u;
}
