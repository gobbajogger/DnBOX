/* SPDX-License-Identifier: GPL-3.0-only */
/* Positive soft-double: value = m * 2^e, m in [2^52, 2^53) or 0. Just enough (add, compare,
 * divide by an integer) to run the instrument's double-precision beat clock bit for bit on a
 * float-only FPU. */
#ifndef S1C_SDBL_H
#define S1C_SDBL_H
#include <stdint.h>
typedef struct { uint64_t m; int32_t e; } sdbl;
#endif
#if defined(SDBL_IMPL) && !defined(S1C_SDBL_IMPL_DONE)
#define S1C_SDBL_IMPL_DONE
#define SD_TOP (1ull << 53)
/* round x * 2^e (plus a sticky tail below bit 0) to 53 bits, nearest even */
static sdbl sd_round(uint64_t x, int32_t e, int sticky)
{
    sdbl r = {0, 0};
    int s = 0;
    if (!x) return r;
    while ((x >> s) >= SD_TOP) s++;
    while (x < (SD_TOP >> 1)) { x <<= 1; e--; }
    uint64_t m = x >> s;
    if (s) {
        int g = (int)(x >> (s - 1)) & 1;
        int rest = sticky || (x & ((1ull << (s - 1)) - 1)) != 0;
        if (g && (rest || (m & 1u))) { m++; if (m == SD_TOP) { m >>= 1; e++; } }
    }
    r.m = m; r.e = e + s;
    return r;
}
static sdbl sd_add(sdbl a, sdbl b)
{
    if (!b.m) return a;
    if (!a.m) return b;
    if (a.e < b.e) { sdbl t = a; a = b; b = t; }
    int32_t d = a.e - b.e;
    uint64_t bm = b.m << 10;
    int st = 0;
    if (d >= 63) { st = 1; bm = 0; }
    else if (d) { st = (bm & ((1ull << d) - 1)) != 0; bm >>= d; }
    return sd_round((a.m << 10) + bm, a.e - 10, st);
}
/* (float) f / n, correctly rounded like (double)f / (double)n */
static sdbl sd_fdiv(float f, uint32_t n)
{
    union { float f; uint32_t u; } b; b.f = f;
    uint32_t M = (b.u & 0x7FFFFFu) | 0x800000u, r;
    int32_t E = (int32_t)((b.u >> 23) & 0xFF) - 150;
    sdbl z = {0, 0};
    if (f <= 0.0f || !n) return z;
    uint64_t q = M / n; r = M % n;
    while (q < (1ull << 56)) { r <<= 1; q <<= 1; if (r >= n) { r -= n; q |= 1u; } E--; }
    return sd_round(q, E, r != 0);
}
static sdbl sd_from_u(uint32_t k, float f) /* k * f exactly (k < 2^29, f > 0) */
{
    union { float f; uint32_t u; } b; b.f = f;
    uint64_t M = (uint64_t)((b.u & 0x7FFFFFu) | 0x800000u) * k;
    return sd_round(M, (int32_t)((b.u >> 23) & 0xFF) - 150, 0);
}
static int sd_ge(sdbl a, sdbl b)
{
    if (!b.m) return 1;
    if (!a.m) return 0;
    return a.e != b.e ? a.e > b.e : a.m >= b.m;
}
#endif
