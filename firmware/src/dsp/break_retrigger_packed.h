/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef X0X_BREAK_RETRIGGER_PACKED_H
#define X0X_BREAK_RETRIGGER_PACKED_H
#include <stdint.h>

/* Bits 0..21 store the shortest repeat probability in Q24. Each subsequent
 * step doubles the interval. A two-bit correction recovers the original
 * binary32 rounding of 1 - (1 - p)^2 at each of the five longer lengths. */
static const uint32_t brk_retrigger_packed[100] = {
#include "break_retrigger_pairs.inc"
};

static inline float brk_retrigger_probability(unsigned length, unsigned pot)
{
    if (pot >= 100) return 1.0f;
    uint32_t word = brk_retrigger_packed[pot];
    uint32_t q = word & UINT32_C(0x3fffff);
    uint32_t corrections = word >> 22;
    for (unsigned step = 0; step < length; ++step) {
        uint32_t squared = (uint32_t)(((uint64_t)q * q) >> 24);
        q = 2u * q - squared + (corrections & 3u) - 2u;
        corrections >>= 2;
    }
    return (float)q * 0x1p-24f;
}
#endif
