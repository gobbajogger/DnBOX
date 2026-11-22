/* SPDX-License-Identifier: GPL-3.0-only */
/* Original-reference stimulus, rendered through the firmware dry voice. */
#include "s1_dry_voice.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    if (argc != 7) return 2;
    const unsigned velocity = (unsigned)strtoul(argv[2], NULL, 10);
    s1_dry_controls controls;
    s1_dry_frozen_controls(&controls);
    controls.cutoff_hz = strtof(argv[3], NULL);
    controls.resonance = strtof(argv[4], NULL);
    controls.filter_depth = strtof(argv[5], NULL);
    if (!strcmp(argv[6], "dynamic"))
        controls.filter = (s1_dry_adsr){0.2f, 0.3f, 0.4f, 0.5f};
    else if (strcmp(argv[6], "static")) return 2;

    const float pitches[] = {65.4063949584961f, 261.6255798339844f, 1046.5023193359375f};
    FILE *out = fopen(argv[1], "wbx");
    if (!out) return 1;
    for (unsigned i = 0; i < 3; ++i) {
        s1_dry_voice voice;
        s1_dry_reset(&voice);
        if (s1_dry_configure(&voice, &controls) != S1_DRY_OK) return 1;
        float samples[64];
        for (unsigned block = 0; block < 2757; ++block) {
            if (s1_dry_render(&voice, samples, 64) != S1_DRY_OK) return 1;
            for (unsigned n = 0; n < 64; ++n) if (samples[n] != 0.0f) return 1;
        }
        if (s1_dry_start(&voice, pitches[i], velocity, 0.0f) != S1_DRY_OK) return 1;
        for (unsigned block = 0; block < 1378; ++block) {
            if (block == 1033 && s1_dry_note_off(&voice) != S1_DRY_OK) return 1;
            if (s1_dry_render(&voice, samples, 64) != S1_DRY_OK) return 1;
            for (unsigned n = 0; n < 64; ++n) if (!isfinite(samples[n])) return 1;
            if (fwrite(samples, sizeof(float), 64, out) != 64) return 1;
        }
    }
    return fclose(out) != 0;
}
