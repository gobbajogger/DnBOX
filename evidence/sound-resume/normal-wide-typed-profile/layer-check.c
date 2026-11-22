/* SPDX-License-Identifier: GPL-3.0-only */
#include "dnbx_core.h"
#include <stdio.h>

static int write_block(const float *left, const float *right, unsigned frames)
{
    return fwrite(left, sizeof *left, frames, stdout) != frames ||
           fwrite(right, sizeof *right, frames, stdout) != frames;
}

int main(void)
{
    dnbx_part part;
    if (dnbx_init_superj6(&part, 44100.f) != DNBX_OK) return 1;
    for (unsigned step = 0; step < 1800; ++step) {
        if (step % 23 == 0) {
            for (unsigned layer = 0; layer < DNBX_LAYERS; ++layer) {
                dnbx_layer_params params;
                if (dnbx_get_layer(&part, layer, &params) != DNBX_OK) return 1;
                params.oscillator_mode = DNBX_OSC_SUPERJ6;
                params.waveform = step < 90 || step % 161 == 0 ? 1.f / 3.f : 1.f;
                params.pulse_width = step % 2 ? .5f : .21f;
                params.lfo_hz = .31f;
                params.lfo_pwm_depth = .06f;
                params.amp.release_s = .03f;
                if (step % 5 == 0 && layer == 1) params.pulse_width = .3f;
                if (dnbx_set_layer(&part, layer, &params) != DNBX_OK) return 1;
            }
        }
        if (step % 11 == 1 && dnbx_note_on(&part, 48 + step % 19, 104) < 0) return 1;
        if (step >= 4 && step % 11 == 4 && dnbx_note_off(&part, 48 + (step - 3) % 19) < 0) return 1;
        if (step % 141 == 140 && dnbx_all_off(&part) != DNBX_OK) return 1;
        float left[73], right[73];
        const unsigned frames = step % 73 + 1;
        if (dnbx_render(&part, left, right, frames, NULL) != DNBX_OK ||
            write_block(left, right, frames) || fwrite(&part, sizeof part, 1, stdout) != 1) return 1;
    }
    return 0;
}
