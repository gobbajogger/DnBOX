/* SPDX-License-Identifier: GPL-3.0-only */
#include "app/original_runtime.h"
#include "chicago_fx2_compact.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#if PROJECT_CRUSH_LFO
enum { CONTROL_COUNT = 22, LAST_CONTROL = ORIGINAL_SYNTHONE_CRUSH_LFO };
#else
enum { CONTROL_COUNT = 21, LAST_CONTROL = ORIGINAL_SYNTHONE_MASTER_MAKEUP };
#endif

static original_runtime_part part;
static original_runtime_workspace workspace;
static original_chicago_fx2_compact insert;

static float physical(unsigned id, unsigned q)
{
    float unit = (float)q / 127.f;
    switch (id) {
    case ORIGINAL_SYNTHONE_CRUSH: return s1_part_crush_hz(unit);
    case ORIGINAL_SYNTHONE_PAN: return ((float)q - 1.f) / 126.f;
    case ORIGINAL_SYNTHONE_PAN_RATE: return s1_pan_hz(q);
#if PROJECT_CRUSH_LFO
    case ORIGINAL_SYNTHONE_CRUSH_LFO:
#endif
    case ORIGINAL_SYNTHONE_TREM:
    case ORIGINAL_SYNTHONE_LFO1_SHAPE:
    case ORIGINAL_SYNTHONE_LFO2_SHAPE: return (float)((q * 3u + 63u) / 127u);
    case ORIGINAL_SYNTHONE_LFO1_RATE:
    case ORIGINAL_SYNTHONE_LFO2_RATE: return s1_lfo_hz(q);
    }
    if (id >= ORIGINAL_SYNTHONE_MASTER_VOLUME) return s1_master_control(id - ORIGINAL_SYNTHONE_MASTER_VOLUME,q);
    if (id >= ORIGINAL_SYNTHONE_PHASER_MIX) return s1_phaser_control(id - ORIGINAL_SYNTHONE_PHASER_MIX,q);
    return unit;
}

static void key(unsigned note, unsigned velocity)
{
    assert(original_runtime_key(&part, ORIGINAL_INPUT_KEYS, note, velocity) == ORIGINAL_RUNTIME_OK);
}

int main(int argc, char **argv)
{
    assert(argc == 5);
    unsigned selected = (unsigned)strtoul(argv[2], NULL, 10);
    uint8_t cell[32], ext[7] = {0}, pot[ORIGINAL_SYNTHONE_NPARAMS] = {0};
    assert(selected < 694 && original_sound_init(cell, ORIGINAL_SOUND_SYNTHONE, 0));
    FILE *targets = fopen(argv[1], "rb");
    assert(targets && fseek(targets, (long)selected * CONTROL_COUNT * sizeof(float), SEEK_SET) == 0);
    for (unsigned id = 2; id <= LAST_CONTROL; ++id) {
        float target;
        assert(fread(&target, sizeof target, 1, targets) == 1 && isfinite(target));
        unsigned best = id == ORIGINAL_SYNTHONE_PAN ? 1u : 0u;
        for (unsigned q = best+1; q < 128; ++q)
            if (fabsf(target - physical(id,q)) < fabsf(target - physical(id,best))) best = q;
        pot[id] = (uint8_t)best;
        assert(original_sound_set_extended(cell, ext, id, best));
    }
    assert(fclose(targets) == 0);
    for (unsigned id = 2; id <= LAST_CONTROL; ++id) {
        unsigned saved = id >= ORIGINAL_SYNTHONE_MASTER_VOLUME && id <= ORIGINAL_SYNTHONE_MASTER_MAKEUP ?
            original_master_display(cell,ext,id) : original_sound_s1_display(cell,id);
        assert(physical(id,saved) == physical(id,pot[id]));
    }
    assert(original_chicago_fx2_compact_init(&insert));
    original_chicago_fx2_cold_owner owner = original_chicago_fx2_compact_owner(&insert);
    assert(original_runtime_init_extended(&part, cell, ext, 120, NULL, &owner) == ORIGINAL_RUNTIME_OK);
    FILE *controls = fopen(argv[4], "wb");
    assert(controls && fwrite(pot+2, 1, CONTROL_COUNT, controls) == CONTROL_COUNT && fclose(controls) == 0);
    FILE *pcm = fopen(argv[3], "wb");
    assert(pcm);
    for (unsigned at = 0; at < 132300;) {
        if (at == 0) key(54, 100);
        if (at == 22050) key(61, 93);
        if (at == 44100) { key(54, 0); key(61, 0); key(66, 116); }
        if (at == 66150) key(66, 0);
        unsigned n = 22050 - at % 22050;
        if (n > 128) n = 128;
        float left[128], right[128], stereo[256];
        assert(original_runtime_render(&part, &workspace, left, right, n) == ORIGINAL_RUNTIME_OK);
        for (unsigned i = 0; i < n; ++i) {
            stereo[2*i] = left[i]; stereo[2*i+1] = right[i];
        }
        assert(fwrite(stereo, sizeof(float), 2*n, pcm) == 2*n);
        at += n;
    }
    assert(fclose(pcm) == 0);
    return 0;
}
