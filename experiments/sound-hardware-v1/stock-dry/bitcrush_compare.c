/* SPDX-License-Identifier: GPL-3.0-only */
#include "s1_dry_part.h"
#include "original_sound.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Compiled directly from the original kernel by bitcrush_reference.py. */
void crush_reference_reset(void);
float crush_reference_sample(float input, float hz);

int main(void)
{
    unsigned failures = 0, cases = 0;
    for (unsigned q = 0; q <= 127; ++q) {
        s1_dry_part wet, dry;
        uint8_t cell[32] = {[0] = 64, [26] = 'O', [27] = 'S', [28] = ORIGINAL_SOUND_SCHEMA,
                            [29] = ORIGINAL_SOUND_SYNTHONE};
        s1_part_init(&wet);
        s1_part_init(&dry);
        crush_reference_reset();
        if (!original_sound_valid(cell)) return 2;
        unsigned slot = ORIGINAL_SYNTHONE_CRUSH;
        float amount;
        if (!original_sound_set(cell, slot, q)) return 2;
        if (!original_sound_control_value(ORIGINAL_SOUND_SYNTHONE, slot, cell[slot], &amount) || !s1_part_param(&wet, slot, amount)) return 2;
        s1_part_note(&wet, 52, 107);
        s1_part_note(&dry, 52, 107);
        float hz = s1_part_crush_hz(amount);
        unsigned different = 0, held = 0;
        for (unsigned block = 0; block < 256; ++block) {
            unsigned n = block % 2 ? 53 : 64;
            float l[64], r[64], dl[64], dr[64];
            if (block == 128) {
                if (!original_sound_set(cell, slot, 127 - q)) return 2;
                if (!original_sound_control_value(ORIGINAL_SOUND_SYNTHONE, slot, cell[slot], &amount) || !s1_part_param(&wet, slot, amount)) return 2;
                hz = s1_part_crush_hz(amount);
            }
            s1_part_render(&wet, l, r, n);
            s1_part_render(&dry, dl, dr, n);
            for (unsigned i = 0; i < n; ++i) {
                float orig = crush_reference_sample(dl[i], hz);
                if (l[i] != orig || r[i] != orig) ++different;
                if (orig != dl[i]) ++held;
            }
        }
        if (different || !held) {
            printf("position %u: %u differing frames, %u held frames\n", q, different, held);
            ++failures;
        }
        ++cases;
    }
    printf("%u control sweeps, 14976 frames per sweep, %u failures\n", cases, failures);
    return failures != 0;
}
