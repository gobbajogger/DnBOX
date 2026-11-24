/* SPDX-License-Identifier: GPL-3.0-only */
/* Independent compact render comparison for INIT and every authenticated pack index. */
#include "app/s1c_part.h"
#include <math.h>
#include <stdio.h>

static s1_dry_part part;
static uint8_t pack[0xf000];
static _Alignas(8) uint8_t arena[S1C_ARENA_BYTES];

int main(int argc, char **argv)
{
    if (argc != 3) return 1;
    FILE *f = fopen(argv[1], "rb");
    if (!f) return 2;
    unsigned bytes = (unsigned)fread(pack, 1, sizeof pack, f);
    if (fgetc(f) != EOF || fclose(f) || s1_part_set_pack(pack, bytes) != 694) return 3;
    FILE *output = fopen(argv[2], "wb");
    if (!output) return 4;
    for (unsigned index = 0; index <= 694; ++index) {
        unsigned program = index == 694 ? S1_PART_INIT : index;
        s1_part_init(&part);
        s1_part_bind(&part, arena);
        if (!s1_part_program(&part, program) || !s1_part_note(&part, 57, 99)) return 5;
        for (unsigned step = 0; step < 8; ++step) {
            float l[128] = {0}, r[128] = {0};
            s1_part_render(&part, l, r, 128);
            for (unsigned i = 0; i < 128; ++i) {
                float stereo[] = {l[i], r[i]};
                if (!isfinite(l[i]) || !isfinite(r[i]) ||
                    fwrite(stereo, sizeof(float), 2, output) != 2) return 6;
            }
        }
    }
    if (fclose(output)) return 7;
    puts("PASS compact dry/wet render captured INIT and 694 factory programs, 1024 frames each");
    return 0;
}
