/* SPDX-License-Identifier: GPL-3.0-only */
#include "band_morph.h"

void sample_cycle(float hz, float morph, float width, unsigned count, float *samples)
{
    for (unsigned frame = 0; frame < count; ++frame) {
        float phase = (float)frame / (float)count;
        samples[frame] = s1_band_morph(phase, hz / 44100.f, morph, width);
    }
}

void sample_raw_cycle(float morph, unsigned count, float *samples)
{
    for (unsigned i = 0; i < count; ++i)
        samples[i] = s1_raw_morph((float)i/(float)count, morph);
}

void sample_steps(float hz, float morph, float width, unsigned frames,
                  float *samples, float *phases)
{
    float increment = hz / 44100.f, phase = 0.f;
    for (unsigned frame = 0; frame < frames; ++frame) {
        phases[frame] = phase;
        samples[frame] = s1_band_morph(phase, increment, morph, width);
        phase += increment;
        if (phase >= 1.f) phase -= 1.f;
    }
}

void sample_parts(unsigned edit, unsigned frames, float *samples)
{
    const float hz[4] = {261.6256f, 440.f, 329.6276f, 523.2511f};
    float morph[4] = {1.f/6.f, .5f, 4.f/6.f, 1.f};
    float width[4] = {.125f, .125f, .125f, .125f};
    float phase[4] = {0.f};
    for (unsigned frame = 0; frame < frames; ++frame) {
        if (frame == frames/2) {
            if (edit == 1) morph[0] = 1.f;
            if (edit == 2) morph[1] = 1.f;
            if (edit == 3) width[1] = .25f;
        }
        for (unsigned lane = 0; lane < 4; ++lane) {
            float increment = hz[lane] / 44100.f;
            if (edit == 4 && lane >= 2) {
                samples[4*frame+lane] = 0.f;
                continue;
            }
            samples[4*frame+lane] = s1_band_morph(phase[lane], increment, morph[lane], width[lane]);
            phase[lane] += increment;
            if (phase[lane] >= 1.f) phase[lane] -= 1.f;
        }
    }
}
