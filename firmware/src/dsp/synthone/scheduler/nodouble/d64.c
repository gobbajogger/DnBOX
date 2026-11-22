/* Integer binary64 arithmetic. Round to nearest, ties to even.
 * The compiler never sees a double, so it emits no __*df* helper. */
#include "d64.h"

typedef struct { uint64_t hi, lo; } u128;

static u128 u128_shl(u128 a, int n)
{
    if (n <= 0) return a;
    if (n >= 128) return (u128){0, 0};
    if (n >= 64) return (u128){a.lo << (n - 64), 0};
    return (u128){(a.hi << n) | (a.lo >> (64 - n)), a.lo << n};
}

static u128 u128_shr(u128 a, int n)
{
    if (n <= 0) return a;
    if (n >= 128) return (u128){0, 0};
    if (n >= 64) return (u128){0, a.hi >> (n - 64)};
    return (u128){a.hi >> n, (a.lo >> n) | (a.hi << (64 - n))};
}

static int u128_bit(u128 a, int n)
{
    if (n < 0 || n >= 128) return 0;
    if (n >= 64) return (a.hi >> (n - 64)) & 1;
    return (a.lo >> n) & 1;
}

static int u128_sticky_below(u128 a, int n)
{
    /* any bit strictly below position n */
    if (n <= 0) return 0;
    if (n >= 128) return a.hi || a.lo;
    if (n >= 64) {
        uint64_t mask = (n == 128) ? ~0ull : ((1ull << (n - 64)) - 1ull);
        return (a.hi & mask) || a.lo;
    }
    uint64_t mask = (n == 64) ? ~0ull : ((1ull << n) - 1ull);
    return (a.lo & mask) != 0;
}

static int clz128(u128 a)
{
    if (a.hi) return __builtin_clzll(a.hi);
    if (a.lo) return 64 + __builtin_clzll(a.lo);
    return 128;
}

static uint64_t pack(int sign, int exp, uint64_t sig52)
{
    return ((uint64_t)sign << 63) | ((uint64_t)exp << 52) | (sig52 & 0x000fffffffffffffull);
}

static uint64_t pack_inf(int sign) { return pack(sign, 0x7ff, 0); }
static uint64_t pack_qnan(void) { return 0x7ff8000000000000ull; }

uint64_t d64_from_float(float x)
{
    uint32_t u;
    __builtin_memcpy(&u, &x, 4);
    int sign = u >> 31;
    int exp = (u >> 23) & 0xff;
    uint32_t frac = u & 0x7fffffu;
    if (exp == 0xff) {
        if (frac) return pack_qnan();
        return pack_inf(sign);
    }
    if (exp == 0) {
        if (!frac) return (uint64_t)sign << 63;
        /* subnormal float -> normal or subnormal double */
        /* frac is 23 bits. Shift its top bit to bit 22 of a 23-bit field. */
        int n = __builtin_clz(frac) - 8;
        frac <<= n;
        int unbiased = -126 - n;
        return pack(sign, unbiased + 1023, (uint64_t)(frac & 0x7fffffu) << 29);
    }
    return pack(sign, exp - 127 + 1023, (uint64_t)frac << 29);
}

float d64_to_float(uint64_t a)
{
    int sign = a >> 63;
    int exp = (a >> 52) & 0x7ff;
    uint64_t sig = a & 0x000fffffffffffffull;
    if (exp == 0x7ff) {
        uint32_t u = ((uint32_t)sign << 31) | (0xffu << 23) | (sig ? 0x400000u : 0);
        float f;
        __builtin_memcpy(&f, &u, 4);
        return f;
    }
    if (exp == 0 && sig == 0) {
        uint32_t u = (uint32_t)sign << 31;
        float f;
        __builtin_memcpy(&f, &u, 4);
        return f;
    }
    /* Build a 24-bit significand (hidden 1 + 23) and round. */
    int unbiased = exp - 1023;
    uint64_t full = sig | (1ull << 52);
    if (unbiased > 127) {
        uint32_t u = ((uint32_t)sign << 31) | (0xffu << 23);
        float f;
        __builtin_memcpy(&f, &u, 4);
        return f;
    }
    if (unbiased < -126 - 24) {
        uint32_t u = (uint32_t)sign << 31;
        float f;
        __builtin_memcpy(&f, &u, 4);
        return f;
    }
    if (unbiased < -126) {
        /* subnormal float: shift so the binary point matches 2^-126 */
        int sh = (-126 - unbiased) + (52 - 23);
        uint64_t keep = full >> sh;
        uint64_t rest = full & ((sh >= 64) ? ~0ull : ((1ull << sh) - 1ull));
        if (rest > (1ull << (sh - 1)) || (rest == (1ull << (sh - 1)) && (keep & 1))) keep++;
        uint32_t u = ((uint32_t)sign << 31) | (uint32_t)keep;
        float f;
        __builtin_memcpy(&f, &u, 4);
        return f;
    }
    /* normal: take top 23 fraction bits from the 52, hidden bit stays */
    uint64_t frac = sig >> 29;
    uint64_t rest = sig & 0x1fffffffull;
    if (rest > 0x10000000ull || (rest == 0x10000000ull && (frac & 1))) {
        frac++;
        if (frac == 0x800000ull) {
            frac = 0;
            unbiased++;
            if (unbiased > 127) {
                uint32_t u = ((uint32_t)sign << 31) | (0xffu << 23);
                float f;
                __builtin_memcpy(&f, &u, 4);
                return f;
            }
        }
    }
    uint32_t u = ((uint32_t)sign << 31) | ((uint32_t)(unbiased + 127) << 23) | ((uint32_t)frac & 0x7fffffu);
    float f;
    __builtin_memcpy(&f, &u, 4);
    return f;
}

uint64_t d64_from_u32(uint32_t n)
{
    if (!n) return 0;
    int shift = __builtin_clz(n);
    uint32_t sig = n << shift;
    int unbiased = 31 - shift;
    return pack(0, unbiased + 1023, (uint64_t)(sig & 0x7fffffffu) << 21);
}

uint64_t d64_neg(uint64_t a) { return a ^ (1ull << 63); }

int d64_isfinite(uint64_t a) { return ((a >> 52) & 0x7ff) != 0x7ff; }

int d64_lt(uint64_t a, uint64_t b)
{
    int sa = a >> 63, sb = b >> 63;
    if (sa != sb) {
        if ((a << 1) == 0 && (b << 1) == 0) return 0;
        return sa > sb;
    }
    if (!sa) return a < b;
    return a > b;
}

static uint64_t round_sig(int sign, int unbiased, u128 mag, int msb)
{
    /* mag's highest set bit is at position msb. We want 53 bits starting there. */
    if (msb < 0) return (uint64_t)sign << 63;
    int exp = unbiased;
    int sh = msb - 52; /* shift so bit msb lands on bit 52 */
    uint64_t sig;
    int guard = 0, sticky = 0;
    if (sh >= 0) {
        u128 top = u128_shr(mag, sh);
        sig = top.lo;
        if (sh > 0) {
            guard = u128_bit(mag, sh - 1);
            sticky = u128_sticky_below(mag, sh - 1);
        }
    } else {
        sig = (mag.lo << -sh);
        guard = 0;
        sticky = 0;
    }
    if (guard && (sticky || (sig & 1))) {
        sig++;
        if (sig == (1ull << 53)) {
            sig >>= 1;
            exp++;
        }
    }
    if (exp >= 1024) return pack_inf(sign);
    if (exp <= -1023) {
        /* subnormal double */
        /* value ~ 2^(exp) * (sig/2^52). For subnormals, shift sig right. */
        int s = (-1022 - exp) + 0;
        if (s > 60) return (uint64_t)sign << 63;
        uint64_t keep = sig >> s;
        uint64_t lost_bit = (s > 0) ? ((sig >> (s - 1)) & 1) : 0;
        uint64_t lower = (s > 1) ? (sig & ((1ull << (s - 1)) - 1ull)) : 0;
        if (lost_bit && (lower || (keep & 1))) keep++;
        return pack(sign, 0, keep);
    }
    return pack(sign, exp + 1023, sig);
}

static uint64_t add_abs(int sign, int ea, uint64_t sa, int eb, uint64_t sb, int subtract)
{
    /* sa, sb include hidden 1 at bit 52 for normals. ea, eb unbiased, ea >= eb. */
    u128 a = u128_shl((u128){0, sa}, 60);
    int diff = ea - eb;
    u128 b;
    int sticky = 0;
    if (diff >= 120) {
        b = (u128){0, 0};
        sticky = sb != 0;
    } else if (diff > 60) {
        b = u128_shr((u128){0, sb}, diff - 60);
        sticky = u128_sticky_below(u128_shl((u128){0, sb}, 0), diff - 60);
        /* bits of sb shifted below 0 when placing at 60-diff */
        if (diff - 60 < 64 && ((sb & ((1ull << (diff - 60)) - 1ull)) != 0)) sticky = 1;
    } else {
        b = u128_shl((u128){0, sb}, 60 - diff);
    }
    if (sticky) b.lo |= 1;
    u128 r;
    if (!subtract) {
        r.lo = a.lo + b.lo;
        r.hi = a.hi + b.hi + (r.lo < a.lo);
    } else {
        r.lo = a.lo - b.lo;
        r.hi = a.hi - b.hi - (a.lo < b.lo);
    }
    if (!r.hi && !r.lo) return 0;
    int msb = 127 - clz128(r);
    /* a was placed with bit 52 at position 60+52 = 112. */
    int unbiased = ea + (msb - 112);
    return round_sig(sign, unbiased, r, msb);
}

static void unpack(uint64_t a, int *sign, int *exp, uint64_t *sig, int *special)
{
    *sign = a >> 63;
    int e = (a >> 52) & 0x7ff;
    uint64_t s = a & 0x000fffffffffffffull;
    *special = 0;
    if (e == 0x7ff) { *special = s ? 2 : 1; *exp = e; *sig = s; return; }
    if (e == 0) {
        if (!s) { *special = 3; *exp = -1022; *sig = 0; return; }
        int n = __builtin_clzll(s) - 11;
        s <<= n;
        *exp = 1 - 1023 - n;
        *sig = s;
        return;
    }
    *exp = e - 1023;
    *sig = s | (1ull << 52);
}

uint64_t d64_add(uint64_t a, uint64_t b)
{
    if ((b << 1) > (a << 1)) { uint64_t t = a; a = b; b = t; }
    int sa, sb, ea, eb, spa, spb;
    uint64_t ma, mb;
    unpack(a, &sa, &ea, &ma, &spa);
    unpack(b, &sb, &eb, &mb, &spb);
    if (spa == 2 || spb == 2) return pack_qnan();
    if (spa == 1) {
        if (spb == 1 && sa != sb) return pack_qnan();
        return pack_inf(sa);
    }
    if (spb == 1) return pack_inf(sb);
    if (spa == 3 && spb == 3) return (sa && sb) ? (1ull << 63) : 0;
    if (spa == 3) return b;
    if (spb == 3) return a;
    return add_abs(sa, ea, ma, eb, mb, sa != sb);
}

uint64_t d64_sub(uint64_t a, uint64_t b) { return d64_add(a, d64_neg(b)); }

static u128 mul53(uint64_t a, uint64_t b)
{
    uint64_t a0 = a & 0xffffffffull, a1 = a >> 32;
    uint64_t b0 = b & 0xffffffffull, b1 = b >> 32;
    uint64_t p0 = a0 * b0;
    uint64_t p1 = a0 * b1;
    uint64_t p2 = a1 * b0;
    uint64_t p3 = a1 * b1;
    uint64_t mid = (p0 >> 32) + (p1 & 0xffffffffull) + (p2 & 0xffffffffull);
    u128 r;
    r.lo = (p0 & 0xffffffffull) | (mid << 32);
    r.hi = p3 + (p1 >> 32) + (p2 >> 32) + (mid >> 32);
    return r;
}

uint64_t d64_mul(uint64_t a, uint64_t b)
{
    int sa, sb, ea, eb, spa, spb;
    uint64_t ma, mb;
    unpack(a, &sa, &ea, &ma, &spa);
    unpack(b, &sb, &eb, &mb, &spb);
    int sign = sa ^ sb;
    if (spa == 2 || spb == 2) return pack_qnan();
    if (spa == 3 || spb == 3) {
        if (spa == 1 || spb == 1) return pack_qnan();
        return (uint64_t)sign << 63;
    }
    if (spa == 1 || spb == 1) return pack_inf(sign);
    u128 p = mul53(ma, mb);
    /* ma, mb are 53-bit. Product bit position: bit 52*2 = 104 is the leading pair. */
    int msb = 127 - clz128(p);
    int unbiased = ea + eb + (msb - 104);
    return round_sig(sign, unbiased, p, msb);
}

uint64_t d64_div(uint64_t a, uint64_t b)
{
    int sa, sb, ea, eb, spa, spb;
    uint64_t ma, mb;
    unpack(a, &sa, &ea, &ma, &spa);
    unpack(b, &sb, &eb, &mb, &spb);
    int sign = sa ^ sb;
    if (spa == 2 || spb == 2) return pack_qnan();
    if (spb == 3) return (spa == 3 || spa == 1) ? pack_qnan() : pack_inf(sign);
    if (spa == 3) return (uint64_t)sign << 63;
    if (spa == 1) return pack_inf(sign);
    if (spb == 1) return (uint64_t)sign << 63;
    /* ma/mb is in [0.5, 2). The first quotient bit has weight 2^0 and ends at bit 55. */
    u128 rem = (u128){0, ma};
    uint64_t quot = 0;
    for (int i = 0; i < 56; i++) {
        quot <<= 1;
        if (rem.hi || rem.lo >= mb) {
            if (rem.lo < mb) rem.hi--;
            rem.lo -= mb;
            quot |= 1;
        }
        rem = u128_shl(rem, 1);
    }
    if (rem.hi || rem.lo) quot |= 1;
    int msb = 63 - __builtin_clzll(quot);
    int unbiased = ea - eb + (msb - 55);
    return round_sig(sign, unbiased, (u128){0, quot}, msb);
}

int64_t d64_trunc_i64(uint64_t a)
{
    int sign, exp, special;
    uint64_t sig;
    unpack(a, &sign, &exp, &sig, &special);
    if (special == 2) return 0;
    if (special == 1) return sign ? (int64_t)((uint64_t)1 << 63) : (int64_t)(((uint64_t)1 << 63) - 1);
    if (special == 3 || exp < 0) return 0;
    if (exp >= 63) return sign ? (int64_t)((uint64_t)1 << 63) : (int64_t)(((uint64_t)1 << 63) - 1);
    uint64_t mag = sig >> (52 - exp);
    if (sign) return mag ? -(int64_t)mag : 0;
    return (int64_t)mag;
}

uint64_t d64_from_i32(int32_t n)
{
    if (n < 0) {
        if (n == (int32_t)0x80000000) return 0xc1e0000000000000ull;
        return d64_neg(d64_from_u32((uint32_t)(-n)));
    }
    return d64_from_u32((uint32_t)n);
}

uint64_t d64_ldexp(uint64_t a, int n)
{
    int sign = (int)(a >> 63);
    int exp = (int)((a >> 52) & 0x7ff);
    if (exp == 0x7ff) return a;
    if (((a << 1) == 0)) return a;
    if (exp == 0) return a;
    int e = exp + n;
    if (e >= 0x7ff) return pack_inf(sign);
    if (e <= 0) return (uint64_t)sign << 63;
    return (a & ~(0x7ffull << 52)) | ((uint64_t)e << 52);
}

int64_t d64_rint(uint64_t a)
{
    int sign, exp, special;
    uint64_t sig;
    unpack(a, &sign, &exp, &sig, &special);
    if (special) return 0;
    if (exp < -1) return 0;
    if (exp == -1) {
        if ((sig & 0x000fffffffffffffull) == 0) return 0;
        return sign ? -1 : 1;
    }
    if (exp >= 62) return d64_trunc_i64(a);
    int shift = 52 - exp;
    uint64_t mag, rem, half;
    if (shift <= 0) mag = sig << (-shift);
    else {
        mag = sig >> shift;
        rem = sig & ((1ull << shift) - 1ull);
        half = 1ull << (shift - 1);
        if (rem > half || (rem == half && (mag & 1ull))) mag++;
    }
    if (sign) return mag ? -(int64_t)mag : 0;
    return (int64_t)mag;
}

int d64_mant(uint64_t a, uint64_t *mant)
{
    int sign, exp, special;
    uint64_t sig;
    unpack(a, &sign, &exp, &sig, &special);
    if (special || sign) return -100000;
    *mant = pack(0, 1023, sig);
    return exp;
}

uint64_t d64_fmod(uint64_t a, uint64_t b)
{
    /* Exact remainder for finite positive values. The clock uses this so a
     * step lands on the same side of a multiplier boundary as libm fmod. */
    if (!d64_isfinite(a) || !d64_isfinite(b) || (b << 1) == 0) return pack_qnan();
    if ((a << 1) == 0) return 0;
    if (!d64_lt(b, a)) return a == b ? 0 : a;
    int sa, sb, ea, eb, spa, spb;
    uint64_t ma, mb;
    unpack(a, &sa, &ea, &ma, &spa);
    unpack(b, &sb, &eb, &mb, &spb);
    int diff = ea - eb;
    if (diff < 0) return a;
    u128 rem = (u128){0, 0};
    int nbits = 53 + diff;
    for (int i = 0; i < nbits; i++) {
        int src = 52 + diff - i;
        unsigned bit = 0;
        if (src >= diff) bit = (unsigned)((ma >> (src - diff)) & 1ull);
        rem = u128_shl(rem, 1);
        rem.lo |= bit;
        if (rem.hi || rem.lo >= mb) {
            if (rem.lo < mb) rem.hi--;
            rem.lo -= mb;
        }
    }
    if (!rem.lo && !rem.hi) return 0;
    if (rem.hi) return pack_qnan();
    int msb = 63 - __builtin_clzll(rem.lo);
    int unbiased = eb + msb - 52;
    if (unbiased >= 1024) return pack_inf(0);
    if (unbiased <= -1023) return 0;
    return pack(0, unbiased + 1023, rem.lo << (52 - msb));
}
