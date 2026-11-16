/* SPDX-License-Identifier: GPL-3.0-only */
/* Analytic SuperJ6 oscillator and white noise. No libm, no heap.
 * Same formulas as OriginalAnalyticOscillator / OriginalWhiteNoise.
 * Sine is the correctly rounded binary32 routine in sj6_sinf.c. */
#ifndef SJ6_ANALYTIC_H
#define SJ6_ANALYTIC_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct Sj6Osc {
    float phase;
    float increment;
    float down[8];
} Sj6Osc;

void sj6_osc_reset(Sj6Osc *o);
void sj6_osc_restart(Sj6Osc *o);
float sj6_osc_saw(Sj6Osc *o, float gain);
float sj6_osc_triangle(Sj6Osc *o, float gain);
float sj6_osc_sine(Sj6Osc *o, float gain);
float sj6_osc_pwm(Sj6Osc *o, float gain, float width);

typedef struct Sj6Noise {
    uint32_t seed;
} Sj6Noise;

void sj6_noise_reset(Sj6Noise *n);
float sj6_noise_sample(Sj6Noise *n);

#ifdef __cplusplus
}
#endif
#endif
