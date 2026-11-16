#pragma once
/* Single-precision helpers for the firmware-target closure.
 * The hot polynomials live in float_math.c, one copy, so the image does not
 * grow a libm and a set of soft-double routines. Index floor is inline and
 * matches libm floor for every finite value inside the signed 32-bit range. */
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
float s1_exp2f(float x);
float s1_expf(float x);
float s1_sinf(float x);
float s1_cosf(float x);
float s1_tanf(float x);
float s1_tanhf(float x);
float s1_powf(float x, float y);
float s1_log2f(float x);
#ifdef __cplusplus
}
#endif
/* Round to nearest integer, ties to even. Matches lrintf for every finite
 * value whose magnitude is at most 2^24, which covers cps * sicvt
 * (SP_FT_MAXLEN is 2^24 and frequency stays at or below the sample rate). */
static inline int32_t s1_lrintf(float x) {
    uint32_t u;
    __builtin_memcpy(&u, &x, 4);
    unsigned sign = u >> 31;
    unsigned exp = (u >> 23) & 255u;
    uint32_t frac = u & 0x7fffffu;
    if (exp < 126u) return 0;
    if (exp == 126u) {
        if (!frac) return 0;
        return sign ? -1 : 1;
    }
    if (exp >= 255u) return 0;
    unsigned e = exp - 127u;
    uint32_t sig = frac | 0x800000u;
    uint32_t mag;
    if (e >= 23u) mag = sig << (e - 23u);
    else {
        unsigned shift = 23u - e;
        mag = sig >> shift;
        uint32_t rem = sig & ((1u << shift) - 1u);
        uint32_t half = 1u << (shift - 1u);
        if (rem > half || (rem == half && (mag & 1u))) mag++;
    }
    return sign ? -(int32_t)mag : (int32_t)mag;
}
/* Exponent field all-ones is infinity or NaN. Same predicate as isfinite. */
static inline int s1_signbitf(float x) {
    uint32_t u;
    __builtin_memcpy(&u, &x, 4);
    return (int)(u >> 31);
}
static inline int s1_finitef(float x) {
    uint32_t u;
    __builtin_memcpy(&u, &x, 4);
    return (u & 0x7f800000u) != 0x7f800000u;
}
static inline float s1_fabsf(float x) {
    union { float f; uint32_t u; } b;
    b.f = x;
    b.u &= 0x7fffffffu;
    return b.f;
}
static inline int s1_ifloor(float x) {
    if (!(x >= -2147483648.f && x < 2147483648.f))
        return x < 0.f ? (-2147483647 - 1) : 2147483647;
    int i = (int)x;
    return ((float)i > x) ? i - 1 : i;
}
#define S1_PI 3.14159265358979f
#define S1_TWO_PI 6.28318530717959f
