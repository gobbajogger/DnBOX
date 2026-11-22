/* SPDX-License-Identifier: GPL-3.0-only */
#include "app/original_runtime.h"
#include "chicago_fx2_compact.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if PROJECT_CRUSH_LFO
enum { CONTROL_COUNT = 22, LAST_CONTROL = ORIGINAL_SYNTHONE_CRUSH_LFO };
#else
enum { CONTROL_COUNT = 21, LAST_CONTROL = ORIGINAL_SYNTHONE_MASTER_MAKEUP };
#endif

#if X0X_SYNTHONE_PATCH
#ifdef S1_PROJ_SUB
enum { VOICE_COUNT = S1_PATCH_COUNT };
#else
enum { VOICE_COUNT = S1_PATCH_CELL_COUNT };
#endif
#else
enum { VOICE_COUNT = 0 };
#endif

unsigned s1_diag_mode, s1_diag_legato;
float s1_diag_glide, s1_diag_wave, s1_diag_tune;
float s1_diag_lfo[132300][2];
unsigned s1_diag_frame, s1_diag_local, s1_diag_cutoff_route, s1_diag_depth_route;
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
#if X0X_SYNTHONE_PATCH
    if (argc == 2 && !strcmp(argv[1], "--voice-ids")) {
        for (unsigned i = 0; i < VOICE_COUNT; ++i)
            printf("%u\n", s1_patch_at(i)->factory_id);
        return 0;
    }
#endif
    assert(argc == 13);
    s1_diag_mode = (unsigned)strtoul(argv[5], NULL, 10);
    s1_diag_legato = (unsigned)strtoul(argv[6], NULL, 10);
    s1_diag_glide = strtof(argv[7], NULL);
    s1_diag_wave = strtof(argv[8], NULL);
    s1_diag_tune = strtof(argv[9], NULL);
    s1_diag_cutoff_route = (unsigned)strtoul(argv[10], NULL, 10);
    s1_diag_depth_route = (unsigned)strtoul(argv[11], NULL, 10);
    FILE *trace = fopen(argv[12], "rb");
    assert(trace && fread(s1_diag_lfo, sizeof s1_diag_lfo, 1, trace) == 1 && !fclose(trace));
    unsigned selected = (unsigned)strtoul(argv[2], NULL, 10);
    uint8_t cell[32], ext[S1_MASTER_EXT_SIZE] = {0}, pot[ORIGINAL_SYNTHONE_NPARAMS] = {0};
    assert(selected < 694 && original_sound_init(cell, ORIGINAL_SOUND_SYNTHONE, 0));
    FILE *targets = fopen(argv[1], "rb");
    assert(targets && fseek(targets, (long)selected * (CONTROL_COUNT + VOICE_COUNT) * sizeof(float),
                            SEEK_SET) == 0);
    for (unsigned id = 2; id <= LAST_CONTROL; ++id) {
        float target;
        assert(fread(&target, sizeof target, 1, targets) == 1 && isfinite(target));
        unsigned best = id == ORIGINAL_SYNTHONE_PAN ? 1u : 0u;
        for (unsigned q = best+1; q < 128; ++q)
            if (fabsf(target - physical(id,q)) < fabsf(target - physical(id,best))) best = q;
        pot[id] = (uint8_t)best;
        assert(original_sound_set_extended(cell, ext, id, best));
    }
#if X0X_SYNTHONE_PATCH
    uint8_t voice_positions[VOICE_COUNT];
    for (unsigned slot = 0; slot < VOICE_COUNT; ++slot) {
        float target;
        assert(fread(&target, sizeof target, 1, targets) == 1 && isfinite(target));
        const s1_patch_descriptor *d = s1_patch_at(slot);
        unsigned best = 0;
        for (unsigned q = 1; q <= S1_PATCH_STEPS; ++q)
            if (fabsf(target - s1_patch_physical(d, q)) < fabsf(target - s1_patch_physical(d, best)))
                best = q;
        voice_positions[slot] = (uint8_t)best;
        assert(original_sound_s1_set_extended(cell, ext, slot, best));
    }
#endif
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
    assert(controls && fwrite(pot+2, 1, CONTROL_COUNT, controls) == CONTROL_COUNT);
#if X0X_SYNTHONE_PATCH
    for (unsigned slot = 0; slot < VOICE_COUNT; ++slot)
        assert(s1_part_patch_value(&part.dsp.synthone, slot) ==
               s1_patch_physical(s1_patch_at(slot), voice_positions[slot]));
    assert(fwrite(voice_positions, 1, VOICE_COUNT, controls) == VOICE_COUNT);
#endif
    assert(fclose(controls) == 0);
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
        s1_diag_frame = at;
        s1_diag_local = 0;
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
