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
