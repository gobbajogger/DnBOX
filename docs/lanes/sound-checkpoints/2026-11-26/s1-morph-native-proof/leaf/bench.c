#include <stdint.h>
#include "fm1_time.h"
#include "phase-round.h"
extern const float direct_wave[3][4][4096];
extern float exact_morph_sample(uint32_t *,float,float,int);
enum { N=16384, CASES=3 };
struct row { uint32_t direct_ticks, compact_ticks, direct_phase, compact_phase; float direct_sum, compact_sum; };
struct result { uint32_t magic, n, cases; struct row data[CASES]; };
static volatile struct result *const out=(volatile struct result *)0x01c10000u;
extern float direct_sample(uint32_t *,float,float,int);
void bench_start(void){
    static const float freq[CASES]={440.f,880.f,12000.f};
    fm1_time_init();
    for(unsigned k=0;k<CASES;k++){
        uint32_t phase=0x793aef, st=fm1_ticks(); float v=0;
        for(unsigned i=0;i<N;i++)v+=direct_sample(&phase,freq[k],(float)(i&1023u)*(1.f/1024.f),1);
        out->data[k].direct_ticks=fm1_ticks()-st;
        out->data[k].direct_phase=phase;
        out->data[k].direct_sum=v;
        phase=0x793aef;v=0;st=fm1_ticks();
        for(unsigned i=0;i<N;i++)v+=exact_morph_sample(&phase,freq[k],(float)(i&1023u)*(1.f/1024.f),1);
        out->data[k].compact_ticks=fm1_ticks()-st;
        out->data[k].compact_phase=phase;
        out->data[k].compact_sum=v;
    }
    out->n=N;out->cases=CASES;out->magic=0x4d4f5250u;
}
