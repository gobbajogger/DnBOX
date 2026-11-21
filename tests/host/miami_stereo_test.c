/* SPDX-License-Identifier: GPL-3.0-only */
/* The original Miami OUTPUT/Stereo macro controls the stereo spread of its
 * voices. Exercise the same 44.1 kHz part renderer and parameter path as MIDI. */
#include "app/original_runtime.h"
#include <math.h>
#include <stdio.h>

enum { STEREO = 15, FRAMES = 12288, WARMUP = 4096, BLOCK = 256 };
static original_runtime_part part;
static original_runtime_workspace workspace;

static int measure(unsigned factory, float width, double *ratio)
{
    uint8_t cell[ORIGINAL_SOUND_BYTES];
    float left[BLOCK], right[BLOCK];
    double mid = 0, side = 0, left_energy = 0, right_energy = 0;
    if (!original_sound_init(cell, ORIGINAL_SOUND_MIAMI, factory) ||
        original_runtime_init(&part, cell, 120.f, NULL, NULL) != ORIGINAL_RUNTIME_OK ||
        original_runtime_param(&part, STEREO, width) != ORIGINAL_RUNTIME_OK ||
        original_runtime_key(&part, ORIGINAL_INPUT_SEQUENCE, 48, 100) != ORIGINAL_RUNTIME_OK)
        return 0;

    for (unsigned start = 0; start < FRAMES; start += BLOCK) {
        if (original_runtime_render(&part, &workspace, left, right, BLOCK) != ORIGINAL_RUNTIME_OK)
            return 0;
        for (unsigned i = 0; i < BLOCK; ++i) {
            double l = left[i], r = right[i];
            if (!isfinite(l) || !isfinite(r)) return 0;
            if (start < WARMUP) continue;
            mid += (l + r) * (l + r);
            side += (l - r) * (l - r);
            left_energy += l * l;
            right_energy += r * r;
        }
    }
    if (mid == 0 || left_energy == 0 || right_energy == 0) return 0;
    *ratio = sqrt(side / mid);
    return 1;
}

int main(void)
{
    const unsigned factories[] = {0, 1, 29};
    for (unsigned i = 0; i < sizeof factories / sizeof factories[0]; ++i) {
        double low, middle, high;
        unsigned factory = factories[i];
        if (!measure(factory, 0.f, &low) || !measure(factory, .5f, &middle) ||
            !measure(factory, 1.f, &high)) {
            fprintf(stderr, "Miami %u failed to render finite audio in both channels\n", factory);
            return 1;
        }
        printf("Miami %u side/mid RMS at Stereo 0, 0.5, 1: %.8f %.8f %.8f\n",
               factory, low, middle, high);
        if (!(low < middle && middle < high)) {
            fprintf(stderr, "Miami %u Stereo failed to increase side/mid RMS\n", factory);
            return 1;
        }
    }
    puts("PASS original Miami stereo control");
    return 0;
}
