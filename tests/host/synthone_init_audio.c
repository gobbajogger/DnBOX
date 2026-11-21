/* SPDX-License-Identifier: GPL-3.0-only */
#include "app/original_runtime.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static original_runtime_part part_a, part_b;
static original_runtime_workspace workspace;

static void key(original_runtime_part *part, unsigned note, unsigned velocity)
{
    assert(original_runtime_key(part, ORIGINAL_INPUT_KEYS, note, velocity) == ORIGINAL_RUNTIME_OK);
}

static float render(original_runtime_part *part, unsigned blocks)
{
    float peak = 0.0f, left[64], right[64];
    while (blocks--) {
        assert(original_runtime_render(part, &workspace, left, right, 64) == ORIGINAL_RUNTIME_OK);
        for (unsigned i = 0; i < 64; ++i) {
            assert(isfinite(left[i]) && left[i] == right[i]);
            if (fabsf(left[i]) > peak) peak = fabsf(left[i]);
        }
    }
    return peak;
}

static void retrigger_trace(const uint8_t cell[32])
{
    assert(original_runtime_init(&part_a, cell, 120, NULL, NULL) == ORIGINAL_RUNTIME_OK);
    assert(original_runtime_init(&part_b, cell, 120, NULL, NULL) == ORIGINAL_RUNTIME_OK);
    key(&part_a, 60, 100);
    key(&part_b, 60, 100);
    render(&part_a, 512);
    render(&part_b, 512);
    key(&part_a, 60, 100);
    float al[64], ar[64], bl[64], br[64];
    for (unsigned i = 0; i < 16; ++i) {
        assert(original_runtime_render(&part_a, &workspace, al, ar, 64) == ORIGINAL_RUNTIME_OK);
        assert(original_runtime_render(&part_b, &workspace, bl, br, 64) == ORIGINAL_RUNTIME_OK);
        assert(!memcmp(al, bl, sizeof al) && !memcmp(ar, br, sizeof ar));
    }
    /* Balance the two Note Ons before testing a retrigger during release. */
    key(&part_a, 60, 0);
    key(&part_a, 60, 0);
    render(&part_a, 16);
    s1_dry_voice before = part_a.dsp.synthone.voice[0];
    assert(before.started && !before.gate);
    key(&part_a, 60, 40);
    s1_dry_voice *after = &part_a.dsp.synthone.voice[0];
    assert(after->gate && after->phase == before.phase);
    assert(!memcmp(&after->amp, &before.amp, sizeof before.amp));
    assert(!memcmp(&after->filter, &before.filter, sizeof before.filter));
    assert(!memcmp(after->filter_history, before.filter_history, sizeof before.filter_history));
    float fraction = 40.f / 127.f;
    assert(after->velocity_squared == fraction * fraction);
}

static void allocation_trace(const uint8_t cell[32])
{
    const unsigned notes[] = {36, 43, 50, 58, 65, 72};
    for (unsigned trace = 0; trace < 3; ++trace) {
        assert(original_runtime_init(&part_a, cell, 120, NULL, NULL) == ORIGINAL_RUNTIME_OK);
        for (unsigned i = 0; i < S1_CORE_VOICES; ++i) {
            key(&part_a, notes[i], 100);
            render(&part_a, 40);
        }
        if (trace == 2) {
            key(&part_a, 50, 80);
            key(&part_a, 79, 100);
            assert(part_a.dsp.synthone.note[3] == 79 && part_a.dsp.synthone.note[0] == 36);
            continue;
        }
        unsigned fresh_slot = trace;
        unsigned released = fresh_slot ? 2 : 3;
        key(&part_a, notes[released], 0);
        render(&part_a, fresh_slot ? 400 : 4);
        assert(part_a.dsp.synthone.voice[released].started == !fresh_slot);
        key(&part_a, 79, 100);
        if (fresh_slot) {
            assert(part_a.dsp.synthone.note[2] == 79);
            key(&part_a, 82, 100);
            assert(part_a.dsp.synthone.note[3] == 82 && part_a.dsp.synthone.note[0] == 36);
        } else {
            assert(part_a.dsp.synthone.note[0] == 79 && part_a.dsp.synthone.note[3] == 58);
        }
    }
}

int main(void)
{
    uint8_t cell[32], legacy[32];
    assert(original_sound_init(cell, ORIGINAL_SOUND_SYNTHONE, ORIGINAL_SYNTHONE_INIT_FACTORY));
    memcpy(legacy, cell, sizeof cell);
    legacy[6] = legacy[7] = 127;
    legacy[22] = 192;
    assert(original_sound_is_legacy_slot2_cell(legacy));
    assert(original_runtime_init(&part_a, legacy, 120, NULL, NULL) == ORIGINAL_RUNTIME_OK);
    assert(original_runtime_init(&part_b, cell, 120, NULL, NULL) == ORIGINAL_RUNTIME_OK);
    assert(!memcmp(legacy, cell, sizeof cell));
    assert(!memcmp(&part_a, &part_b, sizeof part_a));
    assert(render(&part_a, 2) == 0.0f);
    key(&part_a, 69, 100);
    assert(render(&part_a, 64) > 0.005f);
    assert(render(&part_b, 64) == 0.0f);

    key(&part_b, 69, 100);
    assert(original_runtime_param(&part_a, 0, 1.0f) == ORIGINAL_RUNTIME_OK);
    assert(fabsf(part_a.dsp.synthone.voice[0].phase_increment * 44100 - 493.8833f) < 0.02f);
    assert(fabsf(part_b.dsp.synthone.voice[0].phase_increment * 44100 - 440.0f) < 0.02f);
    assert(original_runtime_param(&part_a, 1, 1.0f) == ORIGINAL_RUNTIME_OK);
    key(&part_a, 69, 0);
    key(&part_b, 69, 0);
    assert(part_a.dsp.synthone.voice[0].gate && !part_a.dsp.synthone.down[0]);
    assert(!part_b.dsp.synthone.voice[0].gate);
    assert(render(&part_b, 375) == 0.0f);
    assert(!original_runtime_active(&part_b));
    assert(render(&part_a, 375) > 0.005f);
    assert(original_runtime_param(&part_a, 1, 0.0f) == ORIGINAL_RUNTIME_OK);
    assert(render(&part_a, 375) > 0.0f);
    assert(!original_runtime_active(&part_a) && render(&part_a, 2) == 0.0f);

    for (unsigned i = 0; i < S1_CORE_VOICES + 1; ++i) key(&part_a, 60 + i, 110);
    assert(part_a.dsp.synthone.note[0] == 60 + S1_CORE_VOICES);
    key(&part_a, 60, 0);
    assert(part_a.dsp.synthone.voice[0].gate);
    for (unsigned i = 0; i < S1_CORE_VOICES; ++i) assert(part_a.dsp.synthone.voice[i].started);
    assert(render(&part_a, 32) > 0.005f);
    assert(original_runtime_all_off(&part_a) == ORIGINAL_RUNTIME_OK);
    assert(render(&part_a, 64) == 0.0f && part_a.dsp.synthone.bend == 0.5f);
    key(&part_a, 60, 100);
    key(&part_a, 72, 100);
    render(&part_a, 4);
    key(&part_a, 72, 0);
    float level, modulation;
    unsigned state, down;
    assert(original_runtime_voice_view(&part_a, &level, &modulation, &state, &down));
    assert(level > 0.0f && down);
    for (unsigned note = 0; note < 128; ++note) {
        assert(original_runtime_all_off(&part_a) == ORIGINAL_RUNTIME_OK);
        key(&part_a, note, 100);
        float hz = (float)(440.0 * exp2(((double)note - 69.0) / 12.0));
        assert(part_a.dsp.synthone.voice[0].phase_increment == hz / 44100.0f);
    }
    retrigger_trace(cell);
    allocation_trace(cell);
    puts("Synth One INIT: stock and legacy cells, audible MIDI, two owners, sustain, pitch, release, voice steal and panic PASS");
}
