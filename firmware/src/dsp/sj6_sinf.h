/* SPDX-License-Identifier: GPL-3.0-only */
/* Correctly rounded binary32 sine and cosine. No libm.
 * Payne-Hanek reduction with a 256-bit 2/pi, then a Q62 Taylor series.
 * Matches a correctly rounded result on the oscillator and filter ranges.
 * This platform's sinf is 1 ULP away on some of those inputs, so the voice
 * compares against the rounded long-double value, not against sinf. */
#ifndef SJ6_SINF_H
#define SJ6_SINF_H
#ifdef __cplusplus
extern "C" {
#endif
float sj6_sinf(float x);
float sj6_cosf(float x);
#ifdef __cplusplus
}
#endif
#endif
