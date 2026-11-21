#pragma once
/* Float images of the original double formulas. Each result is the value
 * the C expression stores when it rounds a binary64 result to binary32.
 * The arithmetic is integer binary64, so the target link has no soft-double. */
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
float s1_exp2_f64(float x);
float s1_exp_f64(float x);
float s1_log2_f64(float x);
float s1_sin_f64(float x);
float s1_cos_f64(float x);
float s1_tan_f64(float x);
float s1_tanh_f64(float x);
float s1_pow_f64(float x, float y);
float s1_midi_cps_f64(float nn);
float s1_adsr_pole(float tau, float sr);
float s1_round_mul(float a, uint64_t b_bits);
int s1_f_gt_d(float a, uint64_t b_bits);
float s1_port_onedsr(float sr);
void s1_port_coeffs(float onedsr, float htime, float *c1, float *c2);
float s1_lfo_sine(float phase);
void s1_butbp_consts(float sr, float *pidsr, float *tpidsr);
void s1_butbp_coeffs(float pidsr, float tpidsr, float bw, float fr, float *a);
void s1_buthp_coeffs(float pidsr, float freq, float *a);
void s1_moog_coeffs(float freq, float sr, float *acr, float *tune);
float s1_moog_res4(float res, float acr);
float s1_moog_half(float a, float b);
float s1_moog_tanh_store(float x);
float s1_moog_direct(float delay, float tune, float input, float stag);
float s1_moog_stored(float delay, float tune, float stored, float other);
float s1_moog_tail(float delay, float tune, float stored, float input);
#ifdef __cplusplus
}
#endif
