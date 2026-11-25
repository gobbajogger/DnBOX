/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include <stdint.h>
#include "s1_pan_port_table.inc"

static inline float s1_pan_free_select(unsigned detent) {
    return s1_pan_detent_hz[detent<=127 ? detent : 127];
}
static inline unsigned s1_pan_free_position(float hz) {
    unsigned lo=0, hi=127;
    while(lo<hi) {
        unsigned m=(lo+hi)>>1;
        if(hz<s1_pan_boundary_hz[m]) hi=m;
        else lo=m+1;
    }
    return lo;
}
/* Original oscillator advances a positive 24-bit phase step with
 * ties-to-even, under the default IEEE rounding direction. */
static inline uint32_t s1_pan_q24_increment(float rate,float phase_scale) {
    const float x=rate*phase_scale;
    const uint32_t n=(uint32_t)x;
    const float fraction=x-(float)n;
    return n+(fraction>0.5f || (fraction==0.5f && (n&1u)));
}
