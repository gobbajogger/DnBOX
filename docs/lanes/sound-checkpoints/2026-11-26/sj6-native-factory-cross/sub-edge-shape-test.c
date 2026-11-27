/* SPDX-License-Identifier: GPL-3.0-only */
/*
 * Host proof of the isolated analytic tent from the Sub edge experiment.
 * No source samples, calibration constants, firmware binding, or timing claim.
 * The caller resolves and validates a phase in [0,1), a center in [0,1) of
 * a half-cycle, and independently measured signed oscillator strength.
 */
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static inline float sub_edge_emit(float phase,float center,float strength)
{
    float hf=2.f*phase;
    if(hf>=1.f)hf-=1.f;
    float delta=fabsf(hf-center);
    if(delta>.5f)delta=1.f-delta;
    float z=1.f-delta*80.f;
    return strength*((z>0.f?z:0.f)-.0125f);
}

int main(void)
{
    unsigned samples=1u<<18;
    for(unsigned k=0;k<8;k++){
        float center=(float)k/8.f,strength=.25f;
        double avg=0;
        unsigned positive=0;
        for(unsigned i=0;i<samples;i++){
            float phase=(float)i/(float)(2u*samples);
            float z=sub_edge_emit(phase,center,strength);
            float h=sub_edge_emit(.5f+phase,center,strength);
            assert(!memcmp(&z,&h,sizeof z));
            avg+=z;positive+=z>0;
        }
        assert(fabs(avg/samples)<1e-8);
        assert(positive>samples/45u && positive<samples/35u);
        assert(sub_edge_emit(center*.5f,center,0.f)==0);
    }
    puts("PASS: exact half-cycle repeat, near-zero mean, narrow support; no waveform claim");
}
