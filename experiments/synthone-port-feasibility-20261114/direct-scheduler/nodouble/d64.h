#pragma once
/* Binary64 add/sub/mul/div/sqrt-free compare, in integers.
 * Round to nearest, ties to even. No compiler soft-double calls.
 * A float promotes exactly. A binary64 rounds to binary32 the same way
 * the C cast (float) does. */
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
uint64_t d64_from_float(float x);
float d64_to_float(uint64_t a);
uint64_t d64_from_u32(uint32_t n);
uint64_t d64_neg(uint64_t a);
uint64_t d64_add(uint64_t a, uint64_t b);
uint64_t d64_sub(uint64_t a, uint64_t b);
uint64_t d64_mul(uint64_t a, uint64_t b);
uint64_t d64_div(uint64_t a, uint64_t b);
int d64_lt(uint64_t a, uint64_t b);
int d64_isfinite(uint64_t a);
/* trunc toward zero to a signed 64-bit integer. Saturates. */
int64_t d64_trunc_i64(uint64_t a);
/* Round to nearest integer, ties to even. Non-finite values return 0. */
int64_t d64_rint(uint64_t a);
uint64_t d64_from_i32(int32_t n);
/* Scale a normal value by 2^n. Overflow becomes infinity. */
uint64_t d64_ldexp(uint64_t a, int n);
/* Unbiased exponent and the significand as a value in [1, 2). -100000 if not a positive finite. */
int d64_mant(uint64_t a, uint64_t *mant);
/* Exact positive remainder. Matches libm fmod for finite a >= 0 and b > 0. */
uint64_t d64_fmod(uint64_t a, uint64_t b);
#ifdef __cplusplus
}
#endif
