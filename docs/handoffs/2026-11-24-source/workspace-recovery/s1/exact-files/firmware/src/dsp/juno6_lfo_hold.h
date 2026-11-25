/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include <stdint.h>

/* Build 9 held sample law. The compact renderer owns an independent seeded
 * provider per engine. No table is filled or stored; the unused table's draws
 * are preserved while the compact held route ticks. Bound provider rejection
 * in realtime; failure does not publish a partial held/elapsed result. */
typedef uint64_t (*sj6_entropy_word)(void *);
static uint64_t sj6_held_high(uint64_t a, uint64_t b)
{
    const uint64_t m = UINT32_MAX;
    uint64_t x = (a & m) * (b & m), t = (a >> 32) * (b & m) + (x >> 32);
    return (a >> 32) * (b >> 32) + (t >> 32) + (((t & m) + (a & m) * (b >> 32)) >> 32);
}
static int sj6_held_bound(sj6_entropy_word next, void *ctx,
                         uint64_t bound, uint64_t threshold, uint64_t *out)
{
    for (unsigned attempt = 0; attempt < 4; ++attempt) {
        const uint64_t sample = next(ctx);
        if (sample * bound >= threshold) {
            *out = sj6_held_high(sample, bound);
            return 1;
        }
    }
    return 0;
}
static int sj6_held_amplitude_discard(sj6_entropy_word next, void *ctx)
{
    for (unsigned attempt = 0; attempt < 4; ++attempt)
        if ((uint32_t)((uint64_t)(uint32_t)next(ctx) * 16777217u) >= 16776961u)
            return 1;
    return 0;
}
static int sj6_held_table_discard(sj6_entropy_word next, void *ctx)
{
    uint64_t duration;
    if (!sj6_held_amplitude_discard(next, ctx) ||
        !sj6_held_bound(next, ctx, 207u, 52u, &duration)) return 0;
    for (unsigned done = 0; done < 256u; done += 50u + (unsigned)duration)
        if (!sj6_held_amplitude_discard(next, ctx) ||
            !sj6_held_bound(next, ctx, 207u, 52u, &duration)) return 0;
    return 1;
}
static float sj6_held_center(uint64_t value)
{
    const uint64_t middle = UINT64_C(1) << 52;
    const unsigned negative = value < middle;
    uint64_t magnitude = negative ? middle - value : value - middle;
    if (!magnitude) return 0.f;
    /* Integer conversion would otherwise require a target-specific uint64 ->
     * float helper. Round the 24-bit mantissa, then scale by a binary power. */
    unsigned shift = magnitude >> 32 ? 40u - __builtin_clz((uint32_t)(magnitude >> 32)) :
                     magnitude >= (1u << 24) ? 8u - __builtin_clz((uint32_t)magnitude) : 0u;
    uint64_t top = magnitude >> shift;
    if (shift) {
        uint64_t half = UINT64_C(1) << (shift - 1);
        uint64_t low = magnitude & ((half << 1) - 1u);
        top += low > half || (low == half && (top & 1u));
    }
    /* top <= 2^24, so its binary32 conversion is exact. */
    union {uint32_t u; float f;} scale = {.u = (uint32_t)(75 + shift) << 23};
    float f = (uint32_t)top * scale.f;
    return negative ? -f : f;
}
static int sj6_held_tick(float *elapsed, float *held, float dt, float period,
                         sj6_entropy_word next, void *ctx)
{
    if (!sj6_held_table_discard(next, ctx)) return 0;
    float e = *elapsed + dt;
    if (period > 0.f && e >= period) {
        uint64_t value;
        if (!sj6_held_bound(next, ctx, UINT64_C(9007199254740993),
                           UINT64_C(9007199254738945), &value)) return 0;
        *held = sj6_held_center(value);
        e = 0.f;
    }
    *elapsed = e;
    return 1;
}
