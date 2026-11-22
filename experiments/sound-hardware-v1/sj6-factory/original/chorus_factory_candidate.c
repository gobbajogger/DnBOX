/* SPDX-License-Identifier: GPL-3.0-only */
#include <stdio.h>
#include <stdlib.h>
#include "juno6.h"

enum { MAX_FRAMES = 131072 };

int main(int argc, char **argv)
{
    static float samples[2 * MAX_FRAMES];
    static juno6_chorus_t chorus;
    if (argc != 5) return 2;
    unsigned mode = (unsigned)strtoul(argv[1], NULL, 10);
    float rate = strtof(argv[2], NULL);
    unsigned frames = (unsigned)strtoul(argv[3], NULL, 10);
    float mix = strtof(argv[4], NULL);
    if (mode > 3 || frames > MAX_FRAMES || rate < 8000.f || mix < 0 || mix > 1)
        return 2;
    if (fread(samples, sizeof *samples, 2 * frames, stdin) != 2 * frames)
        return 2;
    juno6_chorus_init(&chorus, rate);
    if (mode) {
        juno6_chorus_select_factory(&chorus, mode, mix);
        for (unsigned first = 0; first < frames; first += 128u) {
            unsigned count = frames - first < 128u ? frames - first : 128u;
            juno6_chorus_process(&chorus, samples + first, samples + frames + first, count);
        }
    }
    return fwrite(samples, sizeof *samples, 2 * frames, stdout) == 2 * frames ? 0 : 1;
}
