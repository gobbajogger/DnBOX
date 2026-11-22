/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include <stdint.h>
#include "fastmath.h"

typedef struct { uint32_t phase, step; } s1_sub_osc;
enum { S1_SUB_PERIOD = 1u << 24, S1_SUB_HALF = S1_SUB_PERIOD / 2u };

static inline void s1_sub_pitch(s1_sub_osc *osc, float base_hz, float pitch_ratio, unsigned extra_octave)
{
    float hz = base_hz * ((extra_octave ? 0.25f : 0.5f) * pitch_ratio);
    if (hz > 22050.f) hz = 22050.f;
    const float floating_step = hz * (16777216.f / 44100.f);
    const uint32_t step = (uint32_t)floating_step;
    const float fraction = floating_step - (float)step;
    osc->step = step + (fraction > 0.5f || (fraction == 0.5f && (step & 1u)));
}

static inline float s1_sub_sample(s1_sub_osc *osc, float velocity_squared, float level, unsigned square)
{
    const uint32_t phase = osc->phase;
    osc->phase = (phase + osc->step) & (S1_SUB_PERIOD - 1u);
    if (level == 0.f) return 0.f;
    if (square)
        return phase > 0u && phase < S1_SUB_HALF && velocity_squared > 0.f ? level : -level;
    /* Reduce turns without a float floor or a wavetable. */
    const int32_t centered = phase < S1_SUB_HALF ? (int32_t)phase :
                             (int32_t)phase - S1_SUB_PERIOD;
    const float sine = fm_sin_pi((float)centered * (FM_TWO_PI / (float)S1_SUB_PERIOD));
    return (sine * velocity_squared) * (3.f * level);
}
