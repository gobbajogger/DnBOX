/* SPDX-License-Identifier: GPL-3.0-only */
#include "sj6_sinf.h"
#include <stdint.h>
#include <string.h>

static void mul64(uint64_t a, uint64_t b, uint64_t *hi, uint64_t *lo)
{
    uint64_t a0 = a & 0xffffffffULL, a1 = a >> 32;
    uint64_t b0 = b & 0xffffffffULL, b1 = b >> 32;
    uint64_t p0 = a0 * b0, p1 = a1 * b0, p2 = a0 * b1, p3 = a1 * b1;
    uint64_t mid = (p0 >> 32) + (p1 & 0xffffffffULL) + (p2 & 0xffffffffULL);
    *lo = (mid << 32) | (p0 & 0xffffffffULL);
    *hi = p3 + (p1 >> 32) + (p2 >> 32) + (mid >> 32);
}

/* 2/pi * 2^256, little-endian. */
static const uint64_t k_two_over_pi[4] = {
    0xfe5163abdebbc561ULL, 0xdb6295993c439041ULL,
    0xfc2757d1f534ddc0ULL, 0xa2f9836e4e441529ULL
};
/* (pi/2) * 2^62 */
static const uint64_t k_pi2_q62 = 0x6487ed5110b4611aULL;

static uint64_t extract64(const uint64_t *p, int nlimb, int bit)
{
    int li, off;
    uint64_t v;
    if (bit < 0) return 0;
    li = bit >> 6;
    off = bit & 63;
    if (li >= nlimb) return 0;
    v = p[li] >> off;
    if (off && li + 1 < nlimb) v |= p[li + 1] << (64 - off);
    return v;
}

static uint64_t qmul(uint64_t a, uint64_t b)
{
    uint64_t hi, lo, r;
    mul64(a, b, &hi, &lo);
    r = (hi << 2) | (lo >> 62);
    if ((lo >> 61) & 1ULL) r++;
    return r;
}

static float q62_to_f(uint64_t mag, int sign)
{
    uint32_t u, mant;
    int lead, exp, e, sh;
    if (!mag) {
        u = sign ? 0x80000000u : 0;
        goto out;
    }
    lead = 63 - __builtin_clzll(mag);
    exp = lead - 62;
    if (lead > 23) {
        uint64_t rem, half;
        sh = lead - 23;
        mant = (uint32_t)((mag >> sh) & 0x7fffffULL);
        rem = mag & ((1ULL << sh) - 1);
        half = 1ULL << (sh - 1);
        if (rem > half || (rem == half && (mant & 1u))) mant++;
    } else {
        mant = (uint32_t)((mag << (23 - lead)) & 0x7fffffULL);
    }
    if (mant == 0x800000u) { mant = 0; exp++; }
    e = exp + 127;
    u = (sign ? 0x80000000u : 0u) | ((uint32_t)e << 23) | mant;
out:
    {
        float f;
        memcpy(&f, &u, 4);
        return f;
    }
}

static float sincos_reduced(float x, int cosine)
{
    uint32_t bits;
    int sign, exp, i, shift, folded, quad, use_cos, out_sign, k, sterm;
    uint32_t mant;
    uint64_t limb[6], carry, q, frac, hi, lo, r, r2, term, sum;
    memcpy(&bits, &x, 4);
    sign = bits >> 31;
    bits &= 0x7fffffffu;
    exp = (bits >> 23) & 255;
    if (exp == 255) {
        uint32_t nan = 0x7fc00000u;
        float n;
        memcpy(&n, &nan, 4);
        return n;
    }
    /* |x| < 2^-12: sin(x) == x and cos(x) == 1 in binary32. */
    if (exp < 115) {
        if (cosine) return 1.f;
        return x;
    }
    mant = (bits & 0x7fffffu) | 0x800000u;
    carry = 0;
    for (i = 0; i < 4; i++) {
        uint64_t s, c2;
        mul64(mant, k_two_over_pi[i], &hi, &lo);
        s = lo + carry;
        c2 = s < lo;
        limb[i] = s;
        carry = hi + c2;
    }
    limb[4] = carry;
    limb[5] = 0;
    shift = 406 - exp;
    q = extract64(limb, 6, shift);
    frac = extract64(limb, 6, shift - 64);
    folded = 0;
    if (frac >= (1ULL << 63)) {
        q += 1;
        frac = ~frac + 1ULL;
        folded = 1;
    }
    mul64(frac, k_pi2_q62, &hi, &lo);
    r = hi;
    if (lo >= (1ULL << 63)) r++;
    r2 = qmul(r, r);
    quad = (int)(q & 3);
    if (cosine) {
        /* cos is even, so the sign of x is dropped. */
        use_cos = quad == 0 || quad == 2;
        if (use_cos) out_sign = quad == 2;
        else out_sign = folded ? (quad == 3) : (quad == 1);
    } else {
        use_cos = quad == 1 || quad == 3;
        if (use_cos) out_sign = (quad == 3) ^ sign;
        else out_sign = ((quad >= 2) ^ folded) ^ sign;
    }
    if (use_cos) {
        term = sum = 1ULL << 62;
        sterm = 1;
        for (k = 1; k <= 9; k++) {
            int den = (2 * k - 1) * (2 * k);
            term = qmul(term, r2);
            term = (term + (uint64_t)den / 2) / (uint64_t)den;
            sterm = -sterm;
            if (!term) break;
            if (sterm < 0) sum -= term;
            else sum += term;
        }
    } else {
        term = sum = r;
        sterm = 1;
        for (k = 1; k <= 9; k++) {
            int den = (2 * k) * (2 * k + 1);
            term = qmul(term, r2);
            term = (term + (uint64_t)den / 2) / (uint64_t)den;
            sterm = -sterm;
            if (!term) break;
            if (sterm < 0) sum -= term;
            else sum += term;
        }
    }
    return q62_to_f(sum, out_sign);
}

float sj6_sinf(float x) { return sincos_reduced(x, 0); }
float sj6_cosf(float x) { return sincos_reduced(x, 1); }
