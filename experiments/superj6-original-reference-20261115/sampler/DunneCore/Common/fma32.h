#ifndef SOUND_FMA32_H
#define SOUND_FMA32_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* RNE, gradual underflow, tininess after rounding, canonical qNaN0x7fc00000.
 * No ambient rounding-mode/FENV reads or writes. Per-call IEEE-style flags.
 * This explicit NaN policy does not assert original ARM payload compatibility. */
typedef struct fma32_result { uint32_t bits; uint32_t flags; } fma32_result;
enum { FMA32_INEXACT=1, FMA32_UNDERFLOW=2, FMA32_OVERFLOW=4,
       FMA32_INFINITE=8, FMA32_INVALID=16 };
fma32_result fma32_evaluate(uint32_t a, uint32_t b, uint32_t c);
/* IEEE-binary32 float ABI adapter; does not change ambient exception flags. */
float fma32_fused(float a, float b, float c);
#ifdef __cplusplus
}
#endif
#endif
