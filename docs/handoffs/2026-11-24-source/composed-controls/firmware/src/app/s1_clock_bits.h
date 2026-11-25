/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef DNBOX_S1_CLOCK_BITS_H
#define DNBOX_S1_CLOCK_BITS_H

#include <stdint.h>
#define S1_CF (UINT64_C(1) << 52)
#define S1_CF_MASK (S1_CF - 1)
#define S1_CSTOP UINT64_MAX
#define S1_CINTMAX ((uint32_t)INT32_MAX + 1u)

static inline unsigned s1_clock_nbits(uint64_t x)
{
    uint32_t hi = (uint32_t)(x >> 32);
    return hi ? 64u - (unsigned)__builtin_clz(hi)
              : 32u - (unsigned)__builtin_clz((uint32_t)x);
}

static inline uint32_t s1_clock_fbits(float f)
{
    union { float f; uint32_t u; } v;
    v.f = f;
    return v.u;
}

static inline uint32_t s1_clock_fsig(uint32_t w)
{
    return (w & 0x7fffffu) | 0x800000u;
}

/* Original: float32(bpm/60) widened exactly, then divided by exact 44100.0.
 * Generate the binary64 significand directly. This executes only on edit/adopt,
 * not per audio sample; remainder stays below 88200. */
static uint64_t s1_clock_delta_bits(float beats_per_second)
{
    uint32_t raw = s1_clock_fbits(beats_per_second), rest = s1_clock_fsig(raw);
    uint32_t init = rest / 44100u;
    rest -= init * 44100u;
    unsigned width = 32u - (unsigned)__builtin_clz(init);
    uint64_t q = init;
    for (unsigned i = width; i < 53u; ++i) {
        rest <<= 1;
        q <<= 1;
        if (rest >= 44100u) { rest -= 44100u; ++q; }
    }
    uint32_t half = rest << 1;
    if (half > 44100u || (half == 44100u && (q & 1u))) ++q;
    return ((uint64_t)((raw >> 23) + width + 871u) << 52) + q;
}

/* Add two positive, normal binary64 encodings with one nearest-even rounding.
 * The native clock domain contains only zero and such normals. No floating64
 * arithmetic, divide, or modulo executes in the per-sample path. */
static inline uint64_t s1_clock_add(uint64_t a, uint64_t b)
{
    if (!a) return b;
    if (b > a) { uint64_t x = a; a = b; b = x; }
    unsigned e = (unsigned)(a >> 52), d = e - (unsigned)(b >> 52);
    if (d > 53u) return a;
    uint64_t m = (a & S1_CF_MASK) | S1_CF;
    uint64_t n = (b & S1_CF_MASK) | S1_CF;
    uint64_t t = n >> d;
    uint64_t r = n - (t << d);
    m += t;
    if (m >> 53) {
        int odd = (int)(m & 1u);
        m >>= 1;
        if (odd && (r || (m & 1u))) ++m;
        ++e;
    } else if (d) {
        uint64_t h = UINT64_C(1) << (d - 1u);
        if (r > h || (r == h && (m & 1u))) ++m;
    }
    return ((uint64_t)(e - 1u) << 52) + m;
}

static inline uint32_t s1_clock_step(uint64_t phase, uint32_t factor, uint32_t *rounded)
{
    /* floor(phase / factor), saturated at the original signed index guard.
     * Native float32 factor is exactly widened by aligning its significand. */
    unsigned pe = (unsigned)(phase >> 52);
    unsigned fe = (factor >> 23) + 896u;
    if (!phase || pe < fe) { if (rounded) *rounded = 0; return 0; }
    unsigned d = pe - fe;
    if (d > 31u) { if (rounded) *rounded = S1_CINTMAX; return S1_CINTMAX; }
    uint64_t f = (uint64_t)s1_clock_fsig(factor) << 29;
    uint64_t rem = (phase & S1_CF_MASK) | S1_CF;
    uint32_t q = 0;
    do {
        q <<= 1;
        if (rem >= f) { rem -= f; ++q; }
        if (!d) break;
        --d;
        rem <<= 1;
    } while (1);
    /* Match double division followed by integer truncation when its nearest
     * rounding carries a nonintegral quotient exactly onto the next integer.
     * All q up to the cap are exactly representable in binary64. */
    if (q >= S1_CINTMAX) q = S1_CINTMAX;
    if (rounded) {
        *rounded = q;
        if (rem && q < S1_CINTMAX) {
            uint32_t next = q + 1u;
            unsigned l = 31u - (unsigned)__builtin_clz(next);
            unsigned power = (next & (next - 1u)) == 0;
            unsigned k = 53u - l + power;
            if (f - rem <= (f >> k)) *rounded = next;
        }
    }
    return q;
}

static inline uint64_t s1_clock_boundary(uint32_t step, uint32_t factor)
{
    if (!step || step > S1_CINTMAX) return S1_CSTOP;
    /* Smallest representable phase at or above the exact next product.
     * An inexact nearest-even product below the boundary would fire early. */
    uint64_t m = (uint64_t)s1_clock_fsig(factor) * step;
    unsigned bits = s1_clock_nbits(m);
    if (bits > 53u) {
        unsigned d = bits - 53u;
        m = (m + ((UINT64_C(1) << d) - 1u)) >> d;
    } else {
        m <<= 53u - bits;
    }
    return ((uint64_t)((factor >> 23) + bits + 871u) << 52) + m;
}
#endif
