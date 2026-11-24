/* SPDX-License-Identifier: GPL-3.0-only */
#include "app/s1c_part.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>

static s1_dry_part part;
static _Alignas(8) uint8_t arena[S1C_ARENA_BYTES];

static void render_one(void)
{
    float left = 0.f, right = 0.f;
    s1_part_render(&part, &left, &right, 1);
    assert(isfinite(left) && isfinite(right));
}

int main(void)
{
    s1_part_init(&part);
    s1_part_bind(&part, arena);
    assert(s1_part_note(&part, 64, 100));
    render_one();
    s1c_voice *voice = &part.e.v[part.e.play_idx];
    assert(voice->kc > 1 && voice->ka1 == 0.f);
    enum { mode = S1_FILTER_TYPE - S1_EXTRA_FIRST };
    uint8_t record[S1_EXTRA_BYTES] = {
        [mode/8] = 1u << (mode%8),
        [2+mode] = 1,
    };
    s1_part_apply_extra(&part, record);
    assert(s1_part_patch_value(&part, S1_FILTER_TYPE) == 1.f);
    render_one();
    assert(voice->ka1 > 0.f && "first held BAND PASS sample must use BAND PASS coefficients");
    float band = voice->ka1;
    record[2+mode] = 2;
    s1_part_apply_extra(&part, record);
    render_one();
    assert(voice->ka1 > 0.f && voice->ka1 != band);
    s1_part_apply_extra(&part, NULL);
    render_one();
    assert(voice->kA > 0.f && s1_part_patch_value(&part, S1_FILTER_TYPE) == 0.f);
    puts("PASS held mode changes refresh the active filter on the first compact DSP sample");
    return 0;
}
