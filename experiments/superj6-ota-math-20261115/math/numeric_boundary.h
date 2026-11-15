#ifndef OTA_NUMERIC_BOUNDARY_H
#define OTA_NUMERIC_BOUNDARY_H
#include "fma32.h"
#ifdef __cplusplus
extern "C" {
#endif
/* All encodings: RNE/gradual/tininess-after, explicit flags,
 * canonical qNaN0x7fc00000. No ambient FENV reads or writes. */
fma32_result f64_to_f32_evaluate(uint64_t bits);
/* Preserve the double ABI; read representation, not a floating conversion. */
float f64_to_f32_rne(double value);
#ifdef __cplusplus
}
#endif
#endif
