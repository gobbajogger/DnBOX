/* SPDX-License-Identifier: GPL-3.0-only */
/* Write PCM plus complete working state for independent byte comparisons. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "juno6.h"

static uint32_t next(uint32_t *state)
{
    *state = *state * 1664525u + 1013904223u;
    return *state;
}

int main(int argc, char **argv)
{
    if (argc != 2) return 1;
    const int voices = atoi(argv[1]);
    if (voices < 1 || voices > 6) return 1;
    uint32_t seed = 1;
    float left[257], right[257];
    juno6_t synth;
    juno6_chorus_t chorus;
    for (unsigned rate = 22050; rate <= 96000; rate *= 2) {
        juno6_init(&synth, (float)rate);
        juno6_chorus_init(&chorus, (float)rate);
        for (int i = 0; i < voices; ++i) juno6_note_on(&synth, 32 + 7 * i, 99);
        for (int i = 0; i < 4000; ++i) {
            unsigned n = next(&seed) % 258;
            if (i % 73 == 0) {
                float value = (next(&seed) >> 16) / 65535.f;
                unsigned index = (next(&seed) >> 16) % 18;
                juno6_set(&synth, index, value);
            }
            unsigned note = 32 + 7 * (i % voices);
            if (i % 37 == 0) juno6_note_off(&synth, note);
            if (i % 53 == 0) juno6_note_on(&synth, note, 80 + i % 47);
            if (i % 103 == 0) juno6_chorus_set(&chorus, 1.25f, 5.2f, i % 206 == 0 ? .25f : 0.f);
            juno6_render(&synth, left, n);
            memcpy(right, left, n * sizeof *left);
            juno6_chorus_process(&chorus, left, right, n);
            if (fwrite(left, sizeof *left, n, stdout) != n ||
                fwrite(right, sizeof *right, n, stdout) != n ||
                fwrite(&synth, sizeof synth, 1, stdout) != 1 ||
                fwrite(&chorus, sizeof chorus, 1, stdout) != 1) return 2;
        }
    }
    return ferror(stdout) ? 2 : 0;
}
