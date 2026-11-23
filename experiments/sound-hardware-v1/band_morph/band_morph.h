/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef DNBX_S1_BAND_MORPH_H
#define DNBX_S1_BAND_MORPH_H

/* Render one oscillator sample. The caller owns phase and modulation state.
 * Preconditions: finite phase in [0,1), cycles/sample in (0,0.5], morph
 * in [0,1], and pulse width in (0,1). The measured factory width is 1/8.
 * Band edges are for a 44,100 Hz engine. */
float s1_band_morph(float phase, float cycles_per_sample, float morph,
                    float pulse_width);

/* Unbandlimited factory waveform at physical pulse width 1/8. */
float s1_raw_morph(float phase, float morph);

#endif
