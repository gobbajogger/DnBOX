/* SPDX-License-Identifier: GPL-3.0-only */
#include "app/original_runtime.h"
#include "app/bass_identity.h"
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

/* A note-off and the next note-on can share one audio boundary. The
 * original takes one amp/filter envelope step at note-off, leaving phase
 * and filter delay untouched. The next rendered sample starts its attack. */
static void adjacent_note_edges(const uint8_t cell[32])
{
    assert(original_runtime_init(&part_a, cell, 120, NULL, NULL) == ORIGINAL_RUNTIME_OK);
    key(&part_a, 60, 100);
    render(&part_a, 300);
    s1_dry_voice *voice = &part_a.dsp.synthone.voice[0];
    s1_dry_voice held = *voice;
    assert(held.amp.stage == DNBX_S1_ENVELOPE_DECAY);
    key(&part_a, 60, 0);
    assert(voice->amp.stage == DNBX_S1_ENVELOPE_RELEASE);
    assert(voice->filter.stage == DNBX_S1_ENVELOPE_RELEASE);
    assert(voice->amp.level == held.amp.level * held.prepared.amp.release_retention);
    assert(voice->filter.level == held.filter.level * held.prepared.filter.release_retention);
    assert(voice->phase == held.phase);
    assert(!memcmp(voice->filter_history, held.filter_history, sizeof held.filter_history));
    key(&part_a, 60, 100);
    render(&part_a, 1);
    assert(voice->amp.stage == DNBX_S1_ENVELOPE_ATTACK);
    assert(voice->amp.level > held.amp.level);
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

#if X0X_SYNTHONE_PATCH
/* Play identical notes on live production voices. Apply one native control to
 * A, retaining phase and both envelope histories. B remains the independent
 * control. Filter envelope parameters need nonzero depth to be audible. */
static void patch_audio(void)
{
    const unsigned changed[] = {127,0,0,0,0,127,127,0,0,127,127,0,0};
    uint8_t cell[32];
    for (unsigned slot = 0; slot < S1_PATCH_COUNT; ++slot) {
        assert(original_sound_init(cell, ORIGINAL_SOUND_SYNTHONE, 0));
        assert(original_sound_s1_set(cell, 4, 90)); /* filterADSRMix */
        assert(original_sound_s1_set(cell, 7, 30)); /* filterSustainLevel */
        assert(original_sound_s1_set(cell, 1, 90)); /* morph1Volume */
        assert(original_runtime_init(&part_a, cell, 120, NULL, NULL) == ORIGINAL_RUNTIME_OK);
        assert(original_runtime_init(&part_b, cell, 120, NULL, NULL) == ORIGINAL_RUNTIME_OK);
        key(&part_a, 60, 105); key(&part_b, 60, 105);
        float al[64], ar[64], bl[64], br[64];
        assert(original_runtime_render(&part_a, &workspace, al, ar, 64) == ORIGINAL_RUNTIME_OK);
        assert(original_runtime_render(&part_b, &workspace, bl, br, 64) == ORIGINAL_RUNTIME_OK);
        assert(!memcmp(al, bl, sizeof al));
        s1_dry_voice before = part_a.dsp.synthone.voice[0];
        assert(original_sound_s1_set(cell, slot, changed[slot]));
        assert(original_sound_valid(cell) && !original_sound_is_legacy_slot2_cell(cell));
        s1_part_apply_cell(&part_a.dsp.synthone, cell);
        const s1_dry_voice *after = &part_a.dsp.synthone.voice[0];
        assert(after->phase == before.phase && !memcmp(&after->amp, &before.amp, sizeof before.amp));
        assert(!memcmp(&after->filter, &before.filter, sizeof before.filter));
        unsigned different = 0;
        for (unsigned block = 0; block < 320; ++block) {
            /* Like the source envelope, a new attack coefficient takes effect
             * at the next attack. Exercise both held and newly started voices. */
            if (block == 32) { key(&part_a, 67, 105); key(&part_b, 67, 105); }
            if (block == 180) {
                key(&part_a, 60, 0); key(&part_b, 60, 0);
                key(&part_a, 67, 0); key(&part_b, 67, 0);
            }
            assert(original_runtime_render(&part_a, &workspace, al, ar, 64) == ORIGINAL_RUNTIME_OK);
            assert(original_runtime_render(&part_b, &workspace, bl, br, 64) == ORIGINAL_RUNTIME_OK);
            for (unsigned f = 0; f < 64; ++f) {
                assert(isfinite(al[f]) && isfinite(bl[f]));
                if (al[f] != bl[f]) ++different;
            }
        }
        printf("factory ID %u changed %u of 20480 frames\n", s1_patch_at(slot)->factory_id, different);
        fflush(stdout);
        assert(different > 64);
        assert(original_runtime_all_off(&part_a) == ORIGINAL_RUNTIME_OK);
        assert(s1_part_patch_value(&part_a.dsp.synthone,slot) ==
            s1_patch_physical(s1_patch_at(slot),changed[slot]));
    }
}
#endif

static void crush_patch(void)
{
    uint8_t cell[32], old[32];
    const uint8_t old_tag[8] = {0x31,0x31,0x53,0x4f,ORIGINAL_SOUND_SCHEMA,0,2,BASS_IDENTITY_KNOWN};
    const bass_identity_entry_t registry = {ORIGINAL_SOUND_SYNTHONE_ID, ORIGINAL_SOUND_SCHEMA,
                                             ORIGINAL_SYNTHONE_NPARAMS, 1};
    bass_identity_t identity;
    assert(bass_identity_decode(&identity, old_tag, sizeof old_tag, &registry, 1) == BASS_IDENTITY_READY);
    assert(identity.count == ORIGINAL_SYNTHONE_NPARAMS);
    float amount;
    assert(original_sound_init(cell, ORIGINAL_SOUND_SYNTHONE, 0));
    assert(original_sound_value(cell, ORIGINAL_SYNTHONE_CRUSH, &amount) && amount == 0.f);
    memcpy(old, cell, sizeof old);
    /* Old eight-control cells can also carry an edit at position two. */
    old[2] = 127;
    old[22] = 4;
    assert(original_sound_is_legacy_slot2_cell(old));
    assert(original_sound_adopt_legacy_cell(old) && !memcmp(old, cell, sizeof old));
    assert(original_runtime_init(&part_b, cell, 120, NULL, NULL) == ORIGINAL_RUNTIME_OK);
    assert(original_sound_set(cell, ORIGINAL_SYNTHONE_CRUSH, 127));
    assert(original_sound_valid(cell) && !original_sound_is_legacy_slot2_cell(cell));
    assert(original_sound_value(cell, ORIGINAL_SYNTHONE_CRUSH, &amount) && amount == 1.f);
    assert(original_runtime_init(&part_a, cell, 120, NULL, NULL) == ORIGINAL_RUNTIME_OK);
    key(&part_a, 60, 105);
    key(&part_b, 60, 105);
    float l[64], r[64], dl[64], dr[64];
    assert(original_runtime_render(&part_a, &workspace, l, r, 64) == ORIGINAL_RUNTIME_OK);
    assert(original_runtime_render(&part_b, &workspace, dl, dr, 64) == ORIGINAL_RUNTIME_OK);
    assert(!memcmp(l, r, sizeof l) && memcmp(l, dl, sizeof l));
    assert(part_b.dsp.synthone.crush_interval == 1.f);
    assert(original_runtime_all_off(&part_a) == ORIGINAL_RUNTIME_OK);
    assert(part_a.dsp.synthone.crush_interval == 44100.f / 2048.f);
}
int main(void)
{
    crush_patch();
#if X0X_SYNTHONE_PATCH
    patch_audio();
#endif
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
    adjacent_note_edges(cell);
    puts("Synth One INIT: stock and legacy cells, audible MIDI, two owners, sustain, pitch, release, voice steal and panic PASS");
}
