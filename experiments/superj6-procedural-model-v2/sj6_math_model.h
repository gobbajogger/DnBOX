/* SPDX-License-Identifier: GPL-3.0-only */
/* Coefficient-free procedural recorded-zone model prototype.
 * No PCM or model payload is embedded. Caller owns model bytes and generated
 * cycle memory. This is approximate recorded-wave math, NOT the original AU. */
#ifndef SJ6_MATH_MODEL_H
#define SJ6_MATH_MODEL_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct Sj6MathModel {
 const uint8_t *bytes;
 uint32_t size, frames, loop_start, loop_end, off_harm, off_drift, off_transient;
 uint64_t phase_inc, phase_zero;
 uint16_t cycle_n, harmonics, segments, fade;
 uint8_t root, low, high;
 float dc;
} Sj6MathModel;
typedef struct Sj6MathState { uint16_t segment; } Sj6MathState;
/* Validates canonical wire offsets, CRC, finite/range/overflow constraints.
 * The inclusive loop endpoint may equal the frame count: that virtual
 * endpoint renders +0, matching the retained original interpolation guard.
 * Does not publish m on failure. n is the actual readable input span.
 * m must point to aligned writable model storage disjoint from the entire
 * input span. Input bytes remain borrowed and immutable for the view lifetime.
 * Span checks reject integer wrap, not unmapped pointers; caller supplies live
 * readable/writable storage. Do not forge/mutate an admitted view or race it. */
int sj6_math_open(Sj6MathModel *m, const void *bytes, uint32_t n);
/* Math-generated one-cycle RAM table. Never copies original PCM. Rejects
 * insufficient storage or out-of-int16 output, never clips or shortens.
 * Requires an unchanged admitted model and aligned writable scratch disjoint
 * from both its view and entire borrowed input. Only cycle_n samples are used.
 * Rejected scratch spans cause no mutation. Numerical failure may be partial.
 * On failure caller must discard the partially written scratch buffer. */
int sj6_math_build(const Sj6MathModel *m, int16_t *cycle, uint32_t capacity);
void sj6_math_reset(Sj6MathState *s);
/* Returns raw PCM-unit math, not normalized audio. Caller supplies original
 * source position and owns loop/note/envelope/pitch semantics. Integer frame
 * plus unsigned Q32 fraction supports block-independent arbitrary positions.
 * This API alone is not original sampler/loop/voice/FX integration. */
float sj6_math_at(const Sj6MathModel *m, Sj6MathState *s, const int16_t *cycle,
                 uint32_t frame, uint32_t fraction);
#ifdef __cplusplus
}
#endif
#endif
