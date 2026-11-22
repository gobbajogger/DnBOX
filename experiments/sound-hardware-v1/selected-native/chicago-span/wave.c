/* SPDX-License-Identifier: GPL-3.0-only */
#include <float.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef CHICAGO_SOURCE
#define CHICAGO_SOURCE "chicago_mp4.c"
#endif
#include CHICAGO_SOURCE

static uint32_t seed = 1;
static uint32_t next_u32(void) {
    seed = 1664525u * seed + 1013904223u;
    return seed;
}
static unsigned long total;
static void check_wave(uint32_t phase, float c) {
    for (unsigned saw = 0; saw <= 1; ++saw) {
        float ref = ck_wave(saw ? 1 : 2, ck_phase(phase), .5f, c, 0);
        float result = ck_master_wave(phase,c,saw);
        if (original_math_bits(ref) != original_math_bits(result)) {
            fprintf(stderr, "phase=%08x correction=%08x saw=%u expected=%08x actual=%08x\n",
                    phase, original_math_bits(c), saw, original_math_bits(ref), original_math_bits(result));
            exit(1);
        }
        ++total;
    }
}
int main(void) {
    static const uint32_t edges[] = {0,1,0x04000000,0x08000000,0x40000000,0x78000000,
        0x7fffffff,0x80000000,0x84000000,0x88000000,0xf8000000,0xfc000000,0xffffffff};
    static const float corrections[] = {.25f,0x1.000002p-2f,.375f,.5f,1.f,2.f,0x1p+16f,FLT_MAX};
    for(unsigned p=0;p<sizeof edges/sizeof *edges;++p)
        for(int d=-259;d<=259;++d)
            for(unsigned k=0;k<sizeof corrections/sizeof *corrections;++k)
                check_wave(edges[p]+(uint32_t)d,corrections[k]);
    for(unsigned i=0;i<1000000;++i) {
        uint32_t phase=next_u32();
        float correction=original_math_float(0x3e800000u+next_u32()%0x40ffffffu);
        check_wave(phase,correction);
    }
    printf("%lu exact saw/pulse values including phase and clamp limits\n", total);
}
