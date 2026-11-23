/* Source-free experimental coefficient preparation, no binary64 or libm.
 * Contract: binary32 inputs, rate 8000..192000, seconds 0..2, attack 0/1.
 * Zero duration -> zero is an explicit proposal extension.
 * Requires IEEE binary32 and uint64_t. Not a complete correct-rounding proof.
 */
#ifndef S1_FIXED_COEFFICIENT_REVIEW_H
#define S1_FIXED_COEFFICIENT_REVIEW_H
#include <stdint.h>
#include <string.h>

static inline uint64_t s1_q62_mul(uint64_t a, uint64_t b)
{
    uint64_t p0 = (uint64_t)(uint32_t)a * (uint32_t)b;
    uint64_t p1 = (uint64_t)(uint32_t)a * (b >> 32);
    uint64_t p2 = (a >> 32) * (uint32_t)b;
    uint64_t p3 = (a >> 32) * (b >> 32);
    uint64_t mid = (p0 >> 32) + (uint32_t)p1 + (uint32_t)p2;
    uint64_t lo = (mid << 32) | (uint32_t)p0;
    uint64_t hi = p3 + (p1 >> 32) + (p2 >> 32) + (mid >> 32);
    uint64_t q = (hi << 2) | (lo >> 62);
    uint64_t tail = lo & ((UINT64_C(1) << 62) - 1);
    uint64_t half = UINT64_C(1) << 61;
    return q + (tail > half || (tail == half && (q & 1)));
}

/* Compute round_even(numerator * 2^shift / denominator), capped at 2^62.
 * Denominator <= 3*(2^24-1)^2 < 2^50, so shifted remainder fits uint64_t.
 * This is coefficient preparation only, outside all sample loops.
 */
static inline uint64_t s1_ratio_shift(unsigned numerator, unsigned shift,
                                      uint64_t denominator)
{
    uint64_t rem = numerator, q = 0;
    for (unsigned i = 0; i < shift; ++i) {
        if (q >= (UINT64_C(1) << 62))
            return UINT64_MAX;
        rem <<= 1;
        q <<= 1;
        if (rem >= denominator) {
            rem -= denominator;
            q |= 1;
        }
    }
    if (q >= (UINT64_C(1) << 62))
        return UINT64_MAX;
    return q + ((rem << 1) > denominator ||
                ((rem << 1) == denominator && (q & 1)));
}

static inline uint64_t s1_round_shift(uint64_t x, unsigned shift)
{
    uint64_t q, tail, half;
    if (shift > 63)
        return 0;
    if (!shift)
        return x;
    q = x >> shift;
    tail = x & ((UINT64_C(1) << shift) - 1);
    half = UINT64_C(1) << (shift - 1);
    return q + (tail > half || (tail == half && (q & 1)));
}

static inline float s1_retention_fixed(float seconds, float sample_rate,
                                      int attack)
{
    uint32_t ts, rs, te, re, tm, rm, bits;
    uint64_t denominator, y, term, m;
    int power, exponent = 0;
    float result;
    memcpy(&ts, &seconds, sizeof ts);
    memcpy(&rs, &sample_rate, sizeof rs);
    if ((ts & UINT32_C(0x7fffffff)) == 0)
        return 0.0f;
    te = (ts >> 23) & 255;
    re = (rs >> 23) & 255;
    tm = (ts & UINT32_C(0x7fffff)) | (te ? UINT32_C(0x800000) : 0);
    rm = (rs & UINT32_C(0x7fffff)) | UINT32_C(0x800000);
    power = (te ? (int)te - 150 : -149) + (int)re - 150;
    denominator = (uint64_t)tm * rm * (attack ? 3u : 1u);
    /* x = 1/(rate*seconds*scale); y=x/128 in Q62.
     * Attack's exact 3/5 scale differs slightly from binary64 literal .6;
     * that distinction is included in the comparison/proof limitation.
     */
    y = s1_ratio_shift(attack ? 5u : 1u, (unsigned)(55 - power), denominator);
    if (y >= (UINT64_C(1) << 62))
        return 0.0f; /* x>=128: correctly below half a binary32 subnormal. */
    m = term = UINT64_C(1) << 62;
    for (unsigned n = 1; n <= 20; ++n) {
        term = s1_q62_mul(term, y) / n;
        if (n & 1)
            m -= term;
        else
            m += term;
    }
    if (m < (UINT64_C(1) << 61)) {
        m <<= 1;
        --exponent;
    }
    for (unsigned i = 0; i < 7; ++i) {
        m = s1_q62_mul(m, m);
        exponent *= 2;
        if (m < (UINT64_C(1) << 61)) {
            m <<= 1;
            --exponent;
        }
    }
    if (exponent >= -125) {
        uint64_t significand = s1_round_shift(m, 38);
        if (significand == (UINT64_C(1) << 24)) {
            significand >>= 1;
            ++exponent;
        }
        bits = (uint32_t)(exponent + 126) << 23;
        bits |= (uint32_t)significand & UINT32_C(0x7fffff);
    } else {
        bits = (uint32_t)s1_round_shift(m, (unsigned)(-exponent - 87));
    }
    memcpy(&result, &bits, sizeof result);
    return result;
}
#endif
