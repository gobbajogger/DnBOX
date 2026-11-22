/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef DNBOX_S1_WIDEN_H
#define DNBOX_S1_WIDEN_H

#include <stdint.h>
#include <string.h>

enum { S1_WIDEN_FRAMES = 2205, S1_WIDEN_BYTES = 3 * S1_WIDEN_FRAMES };

/* A 50 ms right-channel history at 44.1 kHz. Call once per frame even at
 * dry mix. The owner supplies zeroed, byte-addressable storage. Preserving
 * the sign, exponent and 15 mantissa bits avoids clipping over-unity audio.
 * Two spans can occupy the selected owner's disjoint existing arenas.
 * Split at a multiple of three, so no sample straddles the two spans. */
static inline float s1_widen_tap(uint8_t *first, unsigned first_frames,
                                uint8_t *second, unsigned *position, float right)
{
    unsigned i = *position;
    uint8_t *p = i < first_frames ? first + 3u * i : second + 3u * (i - first_frames);
    uint32_t bits;
    memcpy(&bits, &right, sizeof bits);
    uint32_t prior = (uint32_t)p[0] << 8 | (uint32_t)p[1] << 16 | (uint32_t)p[2] << 24;
    p[0] = (uint8_t)(bits >> 8);
    p[1] = (uint8_t)(bits >> 16);
    p[2] = (uint8_t)(bits >> 24);
    *position = i + 1u == S1_WIDEN_FRAMES ? 0u : i + 1u;
    float delayed;
    memcpy(&delayed, &prior, sizeof delayed);
    return delayed;
}
#endif
