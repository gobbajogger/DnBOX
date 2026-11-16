#pragma once
/* Integer IEEE-754 binary64, copied from the host reference so the firmware
 * link does not pull <cmath> or soft-double helpers. Same bit patterns as
 * experiments/superj6-original-reference-20261115/.../f64_bits.hpp.
 * Freestanding: stdint.h and string.h only. No exceptions, no heap.
 */
// Integer IEEE-754 binary64 operations. No floating type is used, so a
// single-precision target does not call soft-double helpers.
#include <stdint.h>
#include <string.h>
namespace sj6_math {
inline int f64_clz(uint64_t x) noexcept { return x ? __builtin_clzll(x) : 64; }
inline bool f64_nan(uint64_t u) noexcept { return ((u >> 52) & 0x7ff) == 0x7ff && (u & 0x000fffffffffffffULL); }
inline bool f64_inf(uint64_t u) noexcept { return ((u >> 52) & 0x7ff) == 0x7ff && !(u & 0x000fffffffffffffULL); }
inline bool f64_finite(uint64_t u) noexcept { return ((u >> 52) & 0x7ff) != 0x7ff; }
inline bool f64_zero(uint64_t u) noexcept { return (u << 1) == 0; }
inline int f64_cmp(uint64_t a, uint64_t b) noexcept {
 if (f64_nan(a) || f64_nan(b)) return 2;
 if (f64_zero(a) && f64_zero(b)) return 0;
 const bool sa = a >> 63, sb = b >> 63;
 if (sa != sb) return sa ? -1 : 1;
 if (!sa) return (a > b) - (a < b);
 return (a < b) - (a > b);
}
inline bool f64_lt(uint64_t a, uint64_t b) noexcept { return f64_cmp(a, b) < 0; }
inline bool f64_gt(uint64_t a, uint64_t b) noexcept { return f64_cmp(a, b) > 0; }
inline bool f64_ge(uint64_t a, uint64_t b) noexcept { int c = f64_cmp(a, b); return c == 0 || c > 0; }
inline bool f64_le(uint64_t a, uint64_t b) noexcept { int c = f64_cmp(a, b); return c == 0 || c < 0; }

struct F64Dec { int sign, exp; uint64_t sig; bool nan, inf, zero; };
inline F64Dec f64_dec(uint64_t u) noexcept {
 F64Dec d{};
 d.sign = int(u >> 63);
 const int e = int((u >> 52) & 0x7ff);
 const uint64_t s = u & 0x000fffffffffffffULL;
 if (e == 0x7ff) { d.nan = s != 0; d.inf = s == 0; d.exp = e; d.sig = s; return d; }
 if (e == 0) {
  if (!s) { d.zero = true; d.exp = -1074; return d; }
  const int sh = f64_clz(s) - 11;
  d.sig = s << sh;
  d.exp = -1022 - sh;
  return d;
 }
 d.exp = e - 1023;
 d.sig = s | (1ULL << 52);
 return d;
}
// sig is 53 bits with the leading 1 at bit 52. exp is unbiased. Round already applied.
inline uint64_t f64_pack(int sign, int exp, uint64_t sig) noexcept {
 if (sig == 0) return uint64_t(sign) << 63;
 if (exp >= 1024) return (uint64_t(sign) << 63) | (0x7ffULL << 52);
 if (exp <= -1023) {
  // Shift the leading 1 from bit 52 down to a subnormal.
  const int sh = -1022 - exp;
  if (sh >= 54) return uint64_t(sign) << 63;
  uint64_t lost = sh ? (sig & ((1ULL << (sh - 1)) - 1)) : 0;
  uint64_t bit = sh ? ((sig >> (sh - 1)) & 1) : 0;
  uint64_t mant = sig >> sh;
  if (bit && (lost || (mant & 1))) ++mant;
  if (mant == 0) return uint64_t(sign) << 63;
  if (mant & (1ULL << 52)) {
   // Rounded up into the smallest normal.
   return (uint64_t(sign) << 63) | (1ULL << 52);
  }
  return (uint64_t(sign) << 63) | mant;
 }
 return (uint64_t(sign) << 63) | (uint64_t(exp + 1023) << 52) | (sig & 0x000fffffffffffffULL);
}
// Round a significand whose leading 1 is at bit 62. Bits below bit 10 are extra.
inline uint64_t f64_round(int sign, int exp, uint64_t mag, bool sticky) noexcept {
 if (mag & (1ULL << 63)) { mag >>= 1; exp += 1; }
 const bool round = (mag >> 9) & 1;
 const bool stick = sticky || ((mag & 0x1ff) != 0);
 uint64_t mant = mag >> 10; // bit 52 is the leading 1
 if (round && (stick || (mant & 1))) {
  ++mant;
  if (mant == (1ULL << 53)) { mant >>= 1; exp += 1; }
 }
 return f64_pack(sign, exp, mant);
}
inline uint64_t f64_add(uint64_t ua, uint64_t ub) noexcept {
 if (f64_nan(ua) || f64_nan(ub)) return 0x7ff8000000000000ULL;
 if (f64_inf(ua) && f64_inf(ub)) return (ua >> 63) == (ub >> 63) ? ua : 0x7ff8000000000000ULL;
 if (f64_inf(ua)) return ua;
 if (f64_inf(ub)) return ub;
 F64Dec a = f64_dec(ua), b = f64_dec(ub);
 if (a.zero && b.zero) return (a.sign && b.sign) ? 0x8000000000000000ULL : 0;
 if (a.zero) return ub;
 if (b.zero) return ua;
 if (a.exp < b.exp || (a.exp == b.exp && a.sig < b.sig)) { F64Dec t = a; a = b; b = t; }
 const int shift = a.exp - b.exp;
 uint64_t sa = a.sig << 10;
 uint64_t sb = b.sig << 10;
 bool sticky = false;
 if (shift >= 64) { sticky = sb != 0; sb = 0; }
 else if (shift > 0) { sticky = (sb & ((1ULL << shift) - 1)) != 0; sb >>= shift; }
 if (a.sign == b.sign) {
  uint64_t sum = sa + sb;
  return f64_round(a.sign, a.exp, sum, sticky);
 }
 uint64_t diff = sa - sb;
 if (sticky && ((diff >> 9) & 1) && (diff & 0x1ff) == 0) diff &= ~(1ULL << 9);
 if (diff == 0) return 0;
 const int lz = f64_clz(diff);
 const int cur = 63 - lz; // position of leading 1
 const int want = 62;
 int exp = a.exp;
 if (cur < want) { diff <<= (want - cur); exp -= (want - cur); }
 else if (cur > want) { diff >>= (cur - want); exp += (cur - want); }
 return f64_round(a.sign, exp, diff, false);
}
inline uint64_t f64_sub(uint64_t a, uint64_t b) noexcept { return f64_add(a, b ^ 0x8000000000000000ULL); }
inline uint64_t f64_neg(uint64_t a) noexcept { return a ^ 0x8000000000000000ULL; }
inline void f64_mul64(uint64_t a, uint64_t b, uint64_t& hi, uint64_t& lo) noexcept {
 const uint64_t a0 = a & 0xffffffffULL, a1 = a >> 32, b0 = b & 0xffffffffULL, b1 = b >> 32;
 const uint64_t p0 = a0 * b0, p1 = a1 * b0, p2 = a0 * b1, p3 = a1 * b1;
 const uint64_t mid = (p0 >> 32) + (p1 & 0xffffffffULL) + (p2 & 0xffffffffULL);
 lo = (p0 & 0xffffffffULL) | (mid << 32);
 hi = p3 + (p1 >> 32) + (p2 >> 32) + (mid >> 32);
}
inline uint64_t f64_mul(uint64_t ua, uint64_t ub) noexcept {
 if (f64_nan(ua) || f64_nan(ub)) return 0x7ff8000000000000ULL;
 const int sign = int((ua >> 63) ^ (ub >> 63));
 if (f64_inf(ua) || f64_inf(ub)) {
  if (f64_zero(ua) || f64_zero(ub)) return 0x7ff8000000000000ULL;
  return (uint64_t(sign) << 63) | (0x7ffULL << 52);
 }
 F64Dec a = f64_dec(ua), b = f64_dec(ub);
 if (a.zero || b.zero) return uint64_t(sign) << 63;
 uint64_t hi, lo;
 f64_mul64(a.sig, b.sig, hi, lo);
 // 53-bit * 53-bit: leading bit is at position 104 or 103. hi holds bits [127:64].
 int exp = a.exp + b.exp;
 int top = 127 - f64_clz(hi);
 // Product bits: hi is the high half. The leading 1 should move to bit 62 of a 64-bit word,
 // with 10 extra low bits available for rounding. Bit position in the 128-bit product is top.
 // We want that bit at position 62 in `mag`, so shift the 128-bit value right by (top - 62).
 const int sh = top - 62;
 uint64_t mag;
 bool sticky = false;
 if (sh >= 0) {
  if (sh >= 64) { mag = hi >> (sh - 64); sticky = lo != 0 || (sh > 64 && (hi & ((1ULL << (sh - 64)) - 1))); }
  else { mag = (hi << (64 - sh)) | (lo >> sh); sticky = sh && (lo & ((1ULL << sh) - 1)); }
 } else {
  mag = hi << (-sh);
 }
 // exp adjustment: leading 1 of the mathematical product was at bit `top` of a number
 // whose binary point is 104 bits below the top of a 2^104-scale product of two 2^52 values.
 // sig = m * 2^52, product = m1*m2 * 2^104, value = product_int * 2^(exp_sum - 104).
 // After placing leading 1 at bit 62, mag's bit 62 corresponds to product bit `top`.
 // The unbiased exponent of bit 62 equals exp_sum - 104 + top.
 // f64_round expects the leading 1 at bit 62 to have exponent `exp` (the unbiased exp of that bit's place as bit 52 after >>10, i.e. exp is the exponent of the leading bit).
 exp = a.exp + b.exp - 104 + top;
 return f64_round(sign, exp, mag, sticky);
}
inline uint64_t f64_div(uint64_t ua, uint64_t ub) noexcept {
 if (f64_nan(ua) || f64_nan(ub)) return 0x7ff8000000000000ULL;
 const int sign = int((ua >> 63) ^ (ub >> 63));
 if (f64_inf(ua)) return f64_inf(ub) ? 0x7ff8000000000000ULL : ((uint64_t(sign) << 63) | (0x7ffULL << 52));
 if (f64_zero(ub)) return f64_zero(ua) ? 0x7ff8000000000000ULL : ((uint64_t(sign) << 63) | (0x7ffULL << 52));
 if (f64_inf(ub)) return uint64_t(sign) << 63;
 if (f64_zero(ua)) return uint64_t(sign) << 63;
 F64Dec a = f64_dec(ua), b = f64_dec(ub);
 uint64_t rem = 0, qi = 0;
 for (int i = 52; i >= 0; --i) {
  rem = (rem << 1) | ((a.sig >> i) & 1);
  qi <<= 1;
  if (rem >= b.sig) { rem -= b.sig; qi |= 1; }
 }
 uint64_t q = qi;
 for (int i = 0; i < 55; ++i) {
  rem <<= 1;
  q <<= 1;
  if (rem >= b.sig) { rem -= b.sig; q |= 1; }
 }
 const bool sticky = rem != 0;
 int exp = a.exp - b.exp;
 uint64_t mant;
 bool round, stick;
 if (q & (1ULL << 55)) {
  mant = (q >> 3) & ((1ULL << 53) - 1);
  round = (q >> 2) & 1;
  stick = sticky || (q & 3);
 } else {
  mant = (q >> 2) & ((1ULL << 53) - 1);
  round = (q >> 1) & 1;
  stick = sticky || (q & 1);
  exp -= 1;
 }
 if (round && (stick || (mant & 1))) {
  ++mant;
  if (mant == (1ULL << 53)) { mant >>= 1; exp += 1; }
 }
 return f64_pack(sign, exp, mant);
}
inline uint64_t f64_ceil(uint64_t u) noexcept {
 if (!f64_finite(u) || f64_zero(u)) return u;
 F64Dec d = f64_dec(u);
 if (d.exp >= 52) return u;
 if (d.exp < 0) return d.sign ? 0x8000000000000000ULL : 0x3ff0000000000000ULL;
 const int frac = 52 - d.exp;
 const uint64_t mask = (1ULL << frac) - 1;
 uint64_t sig = d.sig & ~mask;
 if ((d.sig & mask) && !d.sign) {
  sig += 1ULL << frac;
  if (sig == (1ULL << 53)) { sig >>= 1; d.exp += 1; }
 }
 return f64_pack(d.sign, d.exp, sig);
}
inline float f64_to_f32(uint64_t u) noexcept {
 const uint32_t sign = uint32_t(u >> 63) << 31;
 const int exp = int((u >> 52) & 0x7ff);
 const uint64_t frac = u & 0x000fffffffffffffULL;
 uint32_t out = sign;
 if (exp == 0x7ff) out = sign | 0x7f800000u | (frac ? 0x400000u : 0u);
 else if (exp != 0 || frac) {
  const bool sub = exp == 0;
  int e = sub ? -1022 : exp - 1023;
  uint64_t sig = sub ? frac : ((1ULL << 52) | frac);
  if (sub) { const int sh = f64_clz(sig) - 11; sig <<= sh; e = -1022 - sh; }
  if (e > 127) out = sign | 0x7f800000u;
  else if (e < -126) {
   const int sh = -(e + 97);
   if (sh < 64) {
    uint64_t mant = sig >> sh;
    const uint64_t rem = sh ? (sig & ((1ULL << sh) - 1)) : 0;
    const uint64_t half = sh ? (1ULL << (sh - 1)) : 0;
    if (rem > half || (rem == half && (mant & 1))) ++mant;
    out = mant >= 0x800000 ? (sign | (1u << 23)) : (sign | uint32_t(mant));
   }
  } else {
   uint64_t mant = (sig >> 29) & 0x7fffffULL;
   const uint64_t round = (sig >> 28) & 1ULL;
   const uint64_t sticky = sig & 0x0fffffffULL;
   if (round && (sticky || (mant & 1))) {
    ++mant;
    if (mant == 0x800000ULL) { mant = 0; e += 1; }
   }
   out = e > 127 ? (sign | 0x7f800000u) : (sign | (uint32_t(e + 127) << 23) | uint32_t(mant));
  }
 }
 float f; memcpy(&f, &out, 4); return f;
}
inline uint64_t f64_from_f32(float x) noexcept {
 uint32_t u; memcpy(&u, &x, 4);
 const uint64_t sign = uint64_t(u >> 31) << 63;
 const int exp = int((u >> 23) & 0xff);
 const uint32_t frac = u & 0x7fffff;
 if (exp == 255) return sign | (0x7ffULL << 52) | (uint64_t(frac) << 29);
 if (exp == 0) {
  if (!frac) return sign;
  const int sh = __builtin_clz(frac) - 8;
  const uint32_t norm = frac << sh;
  const int e = -126 - sh + 1023;
  return sign | (uint64_t(e) << 52) | (uint64_t(norm & 0x7fffff) << 29);
 }
 return sign | (uint64_t(exp - 127 + 1023) << 52) | (uint64_t(frac) << 29);
}
inline constexpr uint64_t f64_c(uint64_t bits) noexcept { return bits; }

// Integer whose magnitude fits in 63 bits. Used for the sample number and for series terms.
inline uint64_t f64_from_i64(int64_t n) noexcept {
 if (n == 0) return 0;
 const int sign = n < 0;
 const uint64_t mag = sign ? uint64_t(-n) : uint64_t(n);
 const int exp = 63 - f64_clz(mag);
 uint64_t sig = mag;
 int e = exp;
 if (exp <= 52) sig <<= (52 - exp);
 else {
  const int sh = exp - 52;
  const uint64_t rem = sig & ((1ULL << sh) - 1);
  const uint64_t half = 1ULL << (sh - 1);
  sig >>= sh;
  if (rem > half || (rem == half && (sig & 1))) ++sig;
  if (sig == (1ULL << 53)) { sig >>= 1; e += 1; }
 }
 return f64_pack(sign, e, sig);
}
// Toward zero. Matches the double-to-int truncation the original index uses.
inline uint64_t f64_trunc(uint64_t u) noexcept {
 if (!f64_finite(u) || f64_zero(u)) return u;
 F64Dec d = f64_dec(u);
 if (d.exp >= 52) return u;
 if (d.exp < 0) return d.sign ? 0x8000000000000000ULL : 0;
 const int frac = 52 - d.exp;
 return f64_pack(d.sign, d.exp, d.sig & ~((1ULL << frac) - 1));
}
// Round to the nearest integer. Ties go to the even integer.
inline uint64_t f64_nearest(uint64_t u) noexcept {
 if (!f64_finite(u) || f64_zero(u)) return u;
 F64Dec d = f64_dec(u);
 if (d.exp >= 52) return u;
 if (d.exp < -1) return d.sign ? 0x8000000000000000ULL : 0;
 if (d.exp == -1) {
  if (d.sig == (1ULL << 52)) return d.sign ? 0x8000000000000000ULL : 0;
  return d.sign ? 0xbff0000000000000ULL : 0x3ff0000000000000ULL;
 }
 const int frac = 52 - d.exp;
 const uint64_t rem = d.sig & ((1ULL << frac) - 1);
 const uint64_t half = 1ULL << (frac - 1);
 uint64_t sig = d.sig & ~((1ULL << frac) - 1);
 int exp = d.exp;
 if (rem > half || (rem == half && ((sig >> frac) & 1))) {
  sig += 1ULL << frac;
  if (sig == (1ULL << 53)) { sig >>= 1; exp += 1; }
 }
 return f64_pack(d.sign, exp, sig);
}
inline int64_t f64_to_i64(uint64_t u) noexcept {
 if (!f64_finite(u) || f64_zero(u)) return 0;
 F64Dec d = f64_dec(u);
 if (d.exp < 0) return 0;
 if (d.exp >= 63) return d.sign ? int64_t(1ULL << 63) : int64_t(0x7fffffffffffffffLL);
 const uint64_t mag = d.exp >= 52 ? (d.sig << (d.exp - 52)) : (d.sig >> (52 - d.exp));
 return d.sign ? -int64_t(mag) : int64_t(mag);
}
inline uint64_t f64_scalbn(uint64_t u, int n) noexcept {
 if (n == 0 || !f64_finite(u) || f64_zero(u)) return u;
 if (f64_inf(u)) return u;
 F64Dec d = f64_dec(u);
 return f64_pack(d.sign, d.exp + n, d.sig);
}

// IEEE fused multiply-add. The product is kept exact and rounded once with the addend.
uint64_t f64_fma(uint64_t ua, uint64_t ub, uint64_t uc) noexcept;
uint64_t f64_exp(uint64_t u) noexcept;
uint64_t f64_log(uint64_t u) noexcept;
uint64_t f64_pow(uint64_t base, uint64_t exp) noexcept;
uint64_t f64_sin(uint64_t u) noexcept;
#ifdef F64_BITS_IMPLEMENTATION
uint64_t f64_fma(uint64_t ua, uint64_t ub, uint64_t uc) noexcept {
 if (f64_nan(ua) || f64_nan(ub) || f64_nan(uc)) return 0x7ff8000000000000ULL;
 if (f64_inf(ua) || f64_inf(ub) || f64_inf(uc) || f64_zero(ua) || f64_zero(ub))
  return f64_add(f64_mul(ua, ub), uc);
 F64Dec a = f64_dec(ua), b = f64_dec(ub), c = f64_dec(uc);
 if (c.zero) return f64_mul(ua, ub);
 const int ps = a.sign ^ b.sign;
 uint64_t phi, plo;
 f64_mul64(a.sig, b.sig, phi, plo);
 const int top = phi ? (127 - f64_clz(phi)) : (63 - f64_clz(plo));
 const int e_lead = a.exp + b.exp - 104 + top;
 // Window bit 160 holds the product leading 1. Bits below 0 are sticky.
 const int sh = 160 - top;
 uint64_t w0 = 0, w1 = 0, w2 = 0, w3 = 0;
 if (sh >= 128) {
  w2 = plo << (sh - 128);
  w3 = (phi << (sh - 128)) | (plo >> (192 - sh));
 } else if (sh >= 64) {
  w1 = plo << (sh - 64);
  w2 = (phi << (sh - 64)) | (plo >> (128 - sh));
  w3 = phi >> (128 - sh);
 } else {
  w0 = plo << sh;
  w1 = (phi << sh) | (plo >> (64 - sh));
  w2 = phi >> (64 - sh);
 }
 auto add_mag = [&](int bit, uint64_t mag) {
  if (bit < 0 || bit >= 256) return;
  const int limb = bit >> 6;
  const int off = bit & 63;
  uint64_t* p[4] = {&w0, &w1, &w2, &w3};
  const uint64_t lo = mag << off;
  const uint64_t hi = off ? (mag >> (64 - off)) : 0;
  uint64_t carry = 0;
  auto add_slot = [&](int i, uint64_t v) {
   if (i < 0 || i > 3) return;
   const uint64_t cur = *p[i];
   const uint64_t sum = cur + v;
   uint64_t next = sum < cur ? 1 : 0;
   const uint64_t sum2 = sum + carry;
   if (sum2 < sum) next = 1;
   *p[i] = sum2;
   carry = next;
  };
  add_slot(limb, lo);
  add_slot(limb + 1, hi);
  for (int i = limb + 2; carry && i < 4; ++i) add_slot(i, 0);
 };
 auto sub_slot = [](uint64_t& slot, uint64_t v, uint64_t& borrow) {
  const uint64_t cur = slot;
  uint64_t diff = cur - v;
  uint64_t b = cur < v;
  if (borrow) { if (diff == 0) b = 1; --diff; }
  slot = diff;
  borrow = b;
 };
 auto sub_mag = [&](int bit, uint64_t mag) -> bool {
  const int limb = bit >> 6;
  const int off = bit & 63;
  uint64_t* p[4] = {&w0, &w1, &w2, &w3};
  const uint64_t lo = mag << off;
  const uint64_t hi = off ? (mag >> (64 - off)) : 0;
  uint64_t borrow = 0;
  if (limb >= 0 && limb < 4) sub_slot(*p[limb], lo, borrow);
  if (limb + 1 >= 0 && limb + 1 < 4) sub_slot(*p[limb + 1], hi, borrow);
  else if (hi) borrow = 1;
  for (int i = limb + 2; borrow && i < 4; ++i) sub_slot(*p[i], 0, borrow);
  return borrow != 0;
 };
 // Place c so its leading bit has the same weight as the product leading bit.
 const int cbit = 160 + (c.exp - e_lead);
 bool sticky = false;
 bool flipped = false;
 if (cbit < 0) sticky = true;
 else if (cbit - 52 < 0) {
  const int keep = cbit + 1;
  sticky = (c.sig & ((1ULL << (53 - keep)) - 1)) != 0;
  const uint64_t kept = c.sig >> (53 - keep);
  if (c.sign == ps) add_mag(0, kept);
  else flipped = sub_mag(0, kept);
 } else if (cbit < 256) {
  if (c.sign == ps) add_mag(cbit - 52, c.sig);
  else flipped = sub_mag(cbit - 52, c.sig);
 } else {
  // Addend dominates. Fall back; this range is outside sample indices and filter state.
  return f64_add(f64_mul(ua, ub), uc);
 }
 if (flipped) {
  uint64_t carry = 1;
  uint64_t* p[4] = {&w0, &w1, &w2, &w3};
  for (int i = 0; i < 4; ++i) {
   const uint64_t inv = ~(*p[i]);
   const uint64_t sum = inv + carry;
   carry = sum < inv;
   *p[i] = sum;
  }
 }
 int sign = ps ^ (flipped ? 1 : 0);
 if ((w3 | w2 | w1 | w0) == 0) return sticky ? 0 : 0;
 // A subtraction may have borrowed past the leading 1. Find the new leading bit.
 int lead = w3 ? (192 + 63 - f64_clz(w3)) : w2 ? (128 + 63 - f64_clz(w2)) : w1 ? (64 + 63 - f64_clz(w1)) : (63 - f64_clz(w0));
 const int exp = e_lead - (160 - lead);
 // Move `lead` to bit 62 of a 64-bit word and keep a sticky tail.
 const int rsh = lead - 62;
 uint64_t mag = 0;
 if (rsh >= 0) {
  const int src = rsh;
  auto bit = [&](int b) -> uint64_t {
   if (b < 0 || b >= 256) return 0;
   const uint64_t limb = b < 64 ? w0 : b < 128 ? w1 : b < 192 ? w2 : w3;
   return (limb >> (b & 63)) & 1ULL;
  };
  for (int i = 0; i < 64; ++i) mag |= bit(src + i) << i;
  if (!sticky) {
   for (int b = 0; b < src; ++b) if (bit(b)) { sticky = true; break; }
  }
 } else {
  mag = w0 << (-rsh);
 }
 return f64_round(sign, exp, mag, sticky);
}

uint64_t f64_exp(uint64_t u) noexcept {
 constexpr uint64_t kLn2 = 0x3fe62e42fefa39efULL;
 constexpr uint64_t kInvLn2 = 0x3ff71547652b82feULL;
 constexpr uint64_t kOne = 0x3ff0000000000000ULL;
 if (f64_nan(u)) return u;
 if (f64_inf(u)) return (u >> 63) ? 0 : u;
 if (f64_gt(u, 0x40862e42fefa39efULL)) return 0x7ff0000000000000ULL;
 if (f64_lt(u, 0xc0874910d52d3051ULL)) return 0;
 if (f64_zero(u)) return kOne;
 const uint64_t nbits = f64_nearest(f64_mul(u, kInvLn2));
 const int n = int(f64_to_i64(nbits));
 const uint64_t r = f64_sub(u, f64_mul(nbits, kLn2));
 uint64_t term = kOne, sum = kOne;
 for (int k = 1; k <= 18; ++k) {
  term = f64_div(f64_mul(term, r), f64_from_i64(k));
  sum = f64_add(sum, term);
 }
 return f64_scalbn(sum, n);
}
uint64_t f64_log(uint64_t u) noexcept {
 constexpr uint64_t kLn2 = 0x3fe62e42fefa39efULL;
 constexpr uint64_t kTwo = 0x4000000000000000ULL;
 if (f64_nan(u) || (u >> 63)) return 0x7ff8000000000000ULL;
 if (f64_inf(u)) return u;
 if (f64_zero(u)) return 0xfff0000000000000ULL;
 F64Dec d = f64_dec(u);
 // m in [1, 2): significand with exponent 0.
 const uint64_t m = f64_pack(0, 0, d.sig);
 const uint64_t num = f64_sub(m, 0x3ff0000000000000ULL);
 const uint64_t den = f64_add(m, 0x3ff0000000000000ULL);
 const uint64_t z = f64_div(num, den);
 const uint64_t z2 = f64_mul(z, z);
 uint64_t term = z, sum = z;
 for (int k = 1; k <= 22; ++k) {
  term = f64_mul(term, z2);
  sum = f64_add(sum, f64_div(term, f64_from_i64(2 * k + 1)));
 }
 const uint64_t series = f64_mul(sum, kTwo);
 const uint64_t scaled = f64_mul(f64_from_i64(d.exp), kLn2);
 return f64_add(series, scaled);
}
uint64_t f64_pow(uint64_t base, uint64_t exp) noexcept {
 if (f64_nan(base) || f64_nan(exp)) return 0x7ff8000000000000ULL;
 if (f64_zero(exp)) return 0x3ff0000000000000ULL;
 if (base == 0x3ff0000000000000ULL) return base;
 if (base == 0x4000000000000000ULL) return f64_exp(f64_mul(exp, 0x3fe62e42fefa39efULL));
 if ((base >> 63) && !f64_zero(base)) return 0x7ff8000000000000ULL;
 return f64_exp(f64_mul(exp, f64_log(base)));
}
uint64_t f64_sin(uint64_t u) noexcept {
 constexpr uint64_t kInvPi2 = 0x3fe45f306dc9c883ULL;
 constexpr uint64_t kPi2Hi = 0x3ff921fb54400000ULL;
 constexpr uint64_t kPi2Lo = 0x3dd0b4611a626331ULL;
 if (!f64_finite(u)) return 0x7ff8000000000000ULL;
 if (f64_zero(u)) return u;
 uint64_t x = u & 0x7fffffffffffffffULL;
 const int sign0 = int(u >> 63);
 const uint64_t qbits = f64_nearest(f64_mul(x, kInvPi2));
 int q = int(f64_to_i64(qbits));
 uint64_t r = f64_sub(x, f64_mul(qbits, kPi2Hi));
 r = f64_sub(r, f64_mul(qbits, kPi2Lo));
 // Nearest quadrant can leave a negative residual. Fold it into the previous quadrant.
 if ((r >> 63) && !f64_zero(r)) { r = f64_add(0x3ff921fb54442d18ULL, r); q -= 1; }
 const int quad = q & 3;
 const bool cos_path = quad == 1 || quad == 3;
 const int sign = sign0 ^ (quad >= 2);
 uint64_t r2 = f64_mul(r, r);
 if (cos_path) {
  uint64_t term = 0x3ff0000000000000ULL, sum = term;
  for (int k = 1; k <= 12; ++k) {
   term = f64_div(f64_mul(term, r2), f64_from_i64((2 * k - 1) * (2 * k)));
   term ^= 0x8000000000000000ULL;
   sum = f64_add(sum, term);
  }
  return sign ? f64_neg(sum) : sum;
 }
 uint64_t term = r, sum = r;
 for (int k = 1; k <= 12; ++k) {
  term = f64_div(f64_mul(term, r2), f64_from_i64((2 * k) * (2 * k + 1)));
  term ^= 0x8000000000000000ULL;
  sum = f64_add(sum, term);
 }
 return sign ? f64_neg(sum) : sum;
}
#endif
}
