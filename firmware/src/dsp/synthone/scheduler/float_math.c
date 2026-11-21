/* The names stay so existing call sites keep compiling. Each one returns the
 * float stored by the original double libm call. */
#include "float_math.h"
#include "exact.h"
float s1_exp2f(float x) { return s1_exp2_f64(x); }
float s1_expf(float x) { return s1_exp_f64(x); }
float s1_sinf(float x) { return s1_sin_f64(x); }
float s1_cosf(float x) { return s1_cos_f64(x); }
float s1_tanf(float x) { return s1_tan_f64(x); }
float s1_tanhf(float x) { return s1_tanh_f64(x); }
float s1_powf(float x, float y) { return s1_pow_f64(x, y); }
float s1_log2f(float x) { return s1_log2_f64(x); }
