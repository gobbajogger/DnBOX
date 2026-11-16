/* One outlined copy of the firmware single-precision polynomials.
 * Accuracy is the bound already checked for firmware/src/dsp/fastmath.h:
 * exp2 relative 3e-7, exp relative 6e-7, sin/cos absolute 1e-6,
 * tan relative 1e-6, tanh absolute 3e-7, pow relative 1e-5. */
#include "float_math.h"
#include "fastmath.h"
float s1_exp2f(float x) { return fm_exp2f(x); }
float s1_expf(float x) { return fm_expf(x); }
float s1_sinf(float x) { return fm_sinf(x); }
float s1_cosf(float x) { return fm_cosf(x); }
float s1_tanf(float x) { return fm_tanf(x); }
float s1_tanhf(float x) { return fm_tanhf(x); }
float s1_powf(float x, float y) { return fm_powf(x, y); }
float s1_log2f(float x) { return fm_log2f(x); }
