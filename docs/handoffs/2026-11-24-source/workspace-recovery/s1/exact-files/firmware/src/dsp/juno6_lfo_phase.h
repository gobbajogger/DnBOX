/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef JUNO6_LFO_PHASE_H
#define JUNO6_LFO_PHASE_H
#include <stdint.h>
#include <string.h>

/* The target also has no libc __fpclassifyf implementation. */
static inline int sj6_lfo_finite(float value)
{
    uint32_t raw;
    memcpy(&raw, &value, sizeof raw);
    return (raw & UINT32_C(0x7f800000)) != UINT32_C(0x7f800000);
}

/* Truncate nonnegative binary32 below 2^64 exactly. Jieli's target runtime
 * lacks float-to-uint64 conversion; build the two halves with 32-bit shifts.
 * This is after the existing float expression has already rounded. */
static inline uint64_t sj6_lfo_unsigned_phase(float advance)
{
    uint32_t raw;
    memcpy(&raw, &advance, sizeof raw);
    int exponent = (int)((raw >> 23) & 255u) - 127;
    if (raw >> 31 || exponent < 0 || exponent >= 64) return 0;
    uint32_t sig = (raw & UINT32_C(0x007fffff)) | UINT32_C(0x00800000);
    if (exponent < 23) return sig >> (23 - exponent);
    unsigned shift = (unsigned)exponent - 23u;
    if (shift >= 32u) return (uint64_t)(sig << (shift - 32u)) << 32;
    uint32_t lo = sig << shift;
    uint32_t hi = shift ? sig >> (32u - shift) : 0;
    return ((uint64_t)hi << 32) | lo;
}
#endif
