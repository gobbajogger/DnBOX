/* SPDX-License-Identifier: GPL-3.0-only */
#include "app/s1c_part.h"
#include <assert.h>
#include <stdio.h>

static s1_dry_part part;
static uint8_t pack[0xf000];

int main(int argc, char **argv)
{
    assert(argc == 2);
    FILE *file = fopen(argv[1], "rb");
    assert(file);
    unsigned length = (unsigned)fread(pack, 1, sizeof pack, file);
    assert(fgetc(file) == EOF && !fclose(file));
    assert(s1_part_set_pack(pack, length) == 694);
    enum { mode = S1_FILTER_TYPE - S1_EXTRA_FIRST, track = S1_ADSR_TRACK - S1_EXTRA_FIRST };
    uint8_t record[S1_EXTRA_BYTES] = {0};
    record[mode/8] |= 1u << (mode%8);
    record[track/8] |= 1u << (track%8);
    unsigned modes[3] = {0}, tracking = 0;
    for (unsigned index = 0; index <= 694; ++index) {
        unsigned program = index == 694 ? S1_PART_INIT : index;
        assert(s1_part_program(&part, program));
        float original_mode = s1_part_patch_value(&part, S1_FILTER_TYPE);
        float original_tracking = s1_part_patch_value(&part, S1_ADSR_TRACK);
        assert(original_mode == 0.f || original_mode == 1.f || original_mode == 2.f);
        assert(original_tracking >= 0.f && original_tracking <= 1.f);
        ++modes[(int)original_mode];
        tracking += original_tracking != 0.f;
        record[mode+2] = original_mode == 2.f ? 0 : 2;
        record[track+2] = original_tracking > .5f ? 0 : 127;
        s1_part_apply_extra(&part, record);
        assert(s1_part_patch_value(&part, S1_FILTER_TYPE) == record[mode+2]);
        assert(s1_part_patch_value(&part, S1_ADSR_TRACK) == (record[track+2] ? 1.f : 0.f));
        s1_part_apply_extra(&part, NULL);
        assert(s1_part_patch_value(&part, S1_FILTER_TYPE) == original_mode);
        assert(s1_part_patch_value(&part, S1_ADSR_TRACK) == original_tracking);
    }
    printf("PASS INIT and 694 factory mode/track values restored exactly. Modes low/band/high %u/%u/%u, tracking nonzero %u.\n",
           modes[0], modes[1], modes[2], tracking);
    return 0;
}
