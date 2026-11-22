/* SPDX-License-Identifier: GPL-3.0-only */
#include "app/original_runtime.h"
#include "app/bass_identity.h"
#include "chicago_fx2_compact.h"
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
/* Sub edits with zero level are acoustically inert, both for a factory
 * INIT cell and for a legacy adjunct with authored master controls. */
static void sub_bypass(void)
{
    for (unsigned master = 0; master < 2; ++master) {
        uint8_t cell[32], extended[32], ext[S1_MASTER_EXT_SIZE] = {0}, upgraded[S1_MASTER_EXT_SIZE];
        assert(original_sound_init(cell, ORIGINAL_SOUND_SYNTHONE, 0));
        if (master) {
            assert(original_sound_set_extended(cell, ext, ORIGINAL_SYNTHONE_MASTER_VOLUME, 66));
            assert(original_sound_set_extended(cell, ext, ORIGINAL_SYNTHONE_MASTER_RATIO, 93));
        }
        memcpy(extended, cell, sizeof extended);
        memcpy(upgraded, ext, sizeof upgraded);
        assert(original_sound_s1_set_extended(extended, upgraded, S1_PATCH_SUB_OCTAVE, 127));
        assert(original_sound_s1_set_extended(extended, upgraded, S1_PATCH_SUB_SQUARE, 127));
        assert(original_runtime_init_extended(&part_a, cell, ext, 120, NULL, NULL) == ORIGINAL_RUNTIME_OK);
        assert(original_runtime_init_extended(&part_b, extended, upgraded, 120, NULL, NULL) == ORIGINAL_RUNTIME_OK);
        key(&part_a, 60, 105); key(&part_b, 60, 105);
        for (unsigned block = 0; block < 320; ++block) {
            if (block == 90) { key(&part_a, 67, 107); key(&part_b, 67, 107); }
            if (block == 200) { key(&part_a, 60, 0); key(&part_b, 60, 0); }
            float al[64], ar[64], bl[64], br[64];
            assert(original_runtime_render(&part_a, &workspace, al, ar, 64) == ORIGINAL_RUNTIME_OK);
            assert(original_runtime_render(&part_b, &workspace, bl, br, 64) == ORIGINAL_RUNTIME_OK);
            assert(!memcmp(al, bl, sizeof al) && !memcmp(ar, br, sizeof ar));
        }
    }
    puts("Synth One sub bypass: 40,960 stereo frames preserved across v1 and sub-only v2.");
}

/* Play identical notes on live production voices. Apply one native control to
 * A, retaining phase and both envelope histories. B remains the independent
 * control. Filter envelope parameters need nonzero depth to be audible. */
static void patch_audio(void)
{
    const unsigned changed[S1_PATCH_COUNT] = {127,0,0,0,0,127,127,0,0,127,127,0,0,127,127,127,127,127};
    uint8_t cell[32];
    for (unsigned slot = 0; slot < S1_PATCH_COUNT; ++slot) {
        assert(original_sound_init(cell, ORIGINAL_SOUND_SYNTHONE, 0));
        assert(original_sound_s1_set(cell, 4, 90)); /* filterADSRMix */
        assert(original_sound_s1_set(cell, 7, 30)); /* filterSustainLevel */
        assert(original_sound_s1_set(cell, 1, 90)); /* morph1Volume */
        uint8_t ext[S1_MASTER_EXT_SIZE] = {0};
        if (slot == S1_PATCH_PITCH_LFO) {
            assert(original_sound_set(cell, ORIGINAL_SYNTHONE_LFO1_LEVEL, 127));
            assert(original_sound_set(cell, ORIGINAL_SYNTHONE_LFO2_LEVEL, 63));
        }
        if (slot == S1_PATCH_SUB_OCTAVE || slot == S1_PATCH_SUB_SQUARE)
            assert(original_sound_s1_set_extended(cell, ext, S1_PATCH_SUB_LEVEL, 63));
        assert(original_runtime_init_extended(&part_a, cell, ext, 120, NULL, NULL) == ORIGINAL_RUNTIME_OK);
        assert(original_runtime_init_extended(&part_b, cell, ext, 120, NULL, NULL) == ORIGINAL_RUNTIME_OK);
        key(&part_a, 60, 105); key(&part_b, 60, 105);
        float al[64], ar[64], bl[64], br[64];
        assert(original_runtime_render(&part_a, &workspace, al, ar, 64) == ORIGINAL_RUNTIME_OK);
        assert(original_runtime_render(&part_b, &workspace, bl, br, 64) == ORIGINAL_RUNTIME_OK);
        assert(!memcmp(al, bl, sizeof al));
        s1_dry_voice before = part_a.dsp.synthone.voice[0];
        assert(original_sound_s1_set_extended(cell, ext, slot, changed[slot]));
        assert(original_sound_valid(cell) && !original_sound_is_legacy_slot2_cell(cell));
        s1_part_apply_cell(&part_a.dsp.synthone, cell, ext);
        const s1_dry_voice *after = &part_a.dsp.synthone.voice[0];
        assert(after->phase == before.phase && after->sub.phase == before.sub.phase);
        assert(!memcmp(&after->amp, &before.amp, sizeof before.amp));
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
    uint8_t old_tag[8] = {0x31,0x31,0x53,0x4f,ORIGINAL_SOUND_SCHEMA,0,2,BASS_IDENTITY_KNOWN};
    const bass_identity_entry_t registry = {ORIGINAL_SOUND_SYNTHONE_ID, ORIGINAL_SOUND_SCHEMA,
                                             ORIGINAL_SYNTHONE_NPARAMS, 1};
    bass_identity_t identity;
    assert(bass_identity_decode(&identity, old_tag, sizeof old_tag, &registry, 1) == BASS_IDENTITY_READY);
    assert(identity.count == ORIGINAL_SYNTHONE_NPARAMS);
    old_tag[6] = 23;
    assert(bass_identity_decode(&identity,old_tag,sizeof old_tag,&registry,1)==BASS_IDENTITY_READY);
    assert(identity.count == ORIGINAL_SYNTHONE_NPARAMS);
    old_tag[6] = 17;
    assert(bass_identity_decode(&identity,old_tag,sizeof old_tag,&registry,1)==BASS_IDENTITY_READY);
    assert(identity.count == ORIGINAL_SYNTHONE_NPARAMS);
    old_tag[6] = 13;
    assert(bass_identity_decode(&identity, old_tag, sizeof old_tag, &registry, 1) == BASS_IDENTITY_READY);
    assert(identity.count == ORIGINAL_SYNTHONE_NPARAMS);
    old_tag[6] = 6; /* Existing autopan projects have no extended LFO header. */
    assert(bass_identity_decode(&identity, old_tag, sizeof old_tag, &registry, 1) == BASS_IDENTITY_READY);
    assert(identity.count == ORIGINAL_SYNTHONE_NPARAMS);
    old_tag[6] = 4; /* Projects with stock CRUSH and stereo width. */
    assert(bass_identity_decode(&identity, old_tag, sizeof old_tag, &registry, 1) == BASS_IDENTITY_READY);
    assert(identity.count == ORIGINAL_SYNTHONE_NPARAMS);
    old_tag[6] = 3; /* Projects written with the stock CRUSH control. */
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
    /* All 128 legacy hold lengths, including the source log2/exp2 rounding. */
    uint32_t interval_hash=UINT32_C(2166136261);
    for(unsigned q=0;q<128;++q) {
        assert(original_runtime_param(&part_b,ORIGINAL_SYNTHONE_CRUSH,(float)q/127.f)==ORIGINAL_RUNTIME_OK);
        uint32_t word; memcpy(&word,&part_b.dsp.synthone.crush_interval,sizeof word);
        interval_hash=(interval_hash^word)*UINT32_C(16777619);
    }
    assert(interval_hash==UINT32_C(1551743393));
    assert(original_runtime_param(&part_b,ORIGINAL_SYNTHONE_CRUSH,0.f)==ORIGINAL_RUNTIME_OK);
    assert(part_b.dsp.synthone.crush_interval == 1.f);
    assert(original_runtime_all_off(&part_a) == ORIGINAL_RUNTIME_OK);
    assert(part_a.dsp.synthone.crush_interval == 44100.f / 2048.f);
}
enum { TAP_FRAMES = 2205 };
typedef struct {
    uint32_t history[TAP_FRAMES];
    unsigned pos, changed;
    float wet;
} width_reference;

static void check_width(original_runtime_part *part, width_reference *ref, float target, unsigned n)
{
    float l[256], r[256];
    assert(original_runtime_prepare(part, 120.f) == ORIGINAL_RUNTIME_OK);
    assert(original_runtime_render(part, &workspace, l, r, n) == ORIGINAL_RUNTIME_OK);
    const float c2 = (float)pow(.5, (float)(1. / 44100.) / .1f);
    for (unsigned i = 0; i < n; ++i) {
        uint32_t tap = ref->history[ref->pos] & UINT32_C(0xffffff00);
        float delayed;
        memcpy(&delayed, &tap, sizeof delayed);
        memcpy(&ref->history[ref->pos], &l[i], sizeof l[i]);
        ref->pos = (ref->pos + 1) % TAP_FRAMES;
        ref->wet = (1.f - c2) * target + c2 * ref->wet;
        float expected = ref->wet * delayed + (1.f - ref->wet) * l[i];
        assert(!memcmp(&r[i], &expected, sizeof expected));
        if (l[i] != r[i]) ++ref->changed;
    }
}

static void width_patch(void)
{
    static original_chicago_fx2_compact fx[2];
    static original_chicago_tuning tuning;
    static width_reference refs[2];
    original_runtime_part *parts[] = {&part_a, &part_b};
    original_chicago_fx2_cold_owner owners[2];
    uint8_t saved[2][32], chicago[32];
    float targets[2] = {0};
    assert(original_chicago_tuning_bind(&tuning, 0, 0, NULL, 0x6d70));
    for (unsigned v = 0; v < 2; ++v) {
        assert(original_chicago_fx2_compact_init(&fx[v]));
        owners[v] = original_chicago_fx2_compact_owner(&fx[v]);
        assert(original_sound_init(saved[v], ORIGINAL_SOUND_SYNTHONE, 0));
        assert(original_sound_set(saved[v], ORIGINAL_SYNTHONE_CRUSH, v ? 0 : 127));
        assert(original_runtime_init(parts[v], saved[v], 120, &tuning, &owners[v]) == ORIGINAL_RUNTIME_OK);
        key(parts[v], 47 + 15*v, 100);
    }
    /* Warm both rings at zero mix, then run distinct edits across 128 positions.
     * The post-crush left channel is the independent source for the right tap. */
    for (unsigned block = 0; block < 153; ++block) {
        for (unsigned v = 0; v < 2; ++v) {
            if (block >= 24) {
                unsigned position = v ? (block - 24) % 128 : 127 - ((block - 24) % 128);
                assert(original_sound_set(saved[v], ORIGINAL_SYNTHONE_WIDTH, position));
                assert(original_sound_value(saved[v], ORIGINAL_SYNTHONE_WIDTH, &targets[v]));
                assert(original_runtime_param(parts[v], ORIGINAL_SYNTHONE_WIDTH, targets[v]) == ORIGINAL_RUNTIME_OK);
            }
            check_width(parts[v], &refs[v], targets[v], block % 2 ? 113 : 256);
        }
    }
    assert(refs[0].changed > 10000 && refs[1].changed > 10000);
    /* Selection retires A's insert. B continues against its uninterrupted oracle. */
    assert(original_runtime_retire(&part_a) == ORIGINAL_RUNTIME_OK);
    assert(original_sound_init(chicago, ORIGINAL_SOUND_CHICAGO, 0));
    assert(original_runtime_init(&part_a, chicago, 120, &tuning, &owners[0]) == ORIGINAL_RUNTIME_OK);
    key(&part_a, 51, 100);
    float scratch_l[256], scratch_r[256];
    assert(original_runtime_render(&part_a, &workspace, scratch_l, scratch_r, 256) == ORIGINAL_RUNTIME_OK);
    assert(!original_chicago_fx2_compact_free_history(&owners[0]));
    check_width(&part_b, &refs[1], targets[1], 256);
    assert(original_runtime_retire(&part_a) == ORIGINAL_RUNTIME_OK);
    assert(original_runtime_init(&part_a, saved[0], 120, &tuning, &owners[0]) == ORIGINAL_RUNTIME_OK);
    memset(&refs[0], 0, sizeof refs[0]);
    key(&part_a, 73, 100);
    for (unsigned i = 0; i < 18; ++i) {
        check_width(&part_a, &refs[0], targets[0], 256);
        check_width(&part_b, &refs[1], targets[1], 256);
    }
    assert(part_a.dsp.synthone.crush_interval == 44100.f / 2048.f);
    assert(refs[0].changed > 2048);
    assert(original_runtime_all_off(&part_a) == ORIGINAL_RUNTIME_OK);
    assert(!original_runtime_active(&part_a) && part_a.dsp.synthone.width.target == targets[0]);
    assert(original_runtime_retire(&part_a) == ORIGINAL_RUNTIME_OK);
    assert(original_runtime_retire(&part_b) == ORIGINAL_RUNTIME_OK);
    puts("Stereo width: both owners, 128 edits, packed source tap, smoothing, switch, restore and panic passed.");
}


#if X0X_SYNTHONE_PATCH
static void pitch_lfo_step(void)
{
    uint8_t baseline[32], edited[32];
    assert(original_sound_init(baseline, ORIGINAL_SOUND_SYNTHONE, 0));
    memcpy(edited, baseline, sizeof edited);
    assert(original_sound_s1_set_extended(edited, NULL, S1_PATCH_PITCH_LFO, 42));
    assert(original_sound_valid(edited));
    assert(original_sound_s1_sub_position(edited, NULL, S1_PATCH_PITCH_LFO) == 42);
    assert(original_sound_s1_sub_position(baseline, NULL, S1_PATCH_PITCH_LFO) == 0);
    assert(original_runtime_init(&part_a, baseline, 120, NULL, NULL) == ORIGINAL_RUNTIME_OK);
    assert(original_runtime_init(&part_b, edited, 120, NULL, NULL) == ORIGINAL_RUNTIME_OK);
    float al[64], ar[64], bl[64], br[64];
    for (unsigned block = 0; block < 128; ++block) {
        if (block == 0 || block == 32 || block == 96) {
            key(&part_a, 60, 100); key(&part_b, 60, 100);
        }
        if (block == 16 || block == 64) {
            key(&part_a, 60, 0); key(&part_b, 60, 0);
        }
        assert(original_runtime_render(&part_a, &workspace, al, ar, 64) == ORIGINAL_RUNTIME_OK);
        assert(original_runtime_render(&part_b, &workspace, bl, br, 64) == ORIGINAL_RUNTIME_OK);
        assert(!memcmp(al, bl, sizeof al) && !memcmp(ar, br, sizeof ar));
    }
    assert(original_sound_set(edited, ORIGINAL_SYNTHONE_LFO1_LEVEL, 127));
    assert(original_runtime_init(&part_a, edited, 120, NULL, NULL) == ORIGINAL_RUNTIME_OK);
    const unsigned notes[] = {60, 63, 66, 60};
    for (unsigned i = 0; i < sizeof notes / sizeof notes[0]; ++i) {
        const unsigned pitch = notes[i];
        if (i == 3) key(&part_a, pitch, 0);
        key(&part_a, pitch, 100);
        s1_dry_voice *voice = &part_a.dsp.synthone.voice[i == 3 ? 0 : i];
        assert(voice->started);
        const s1_trem_lfo lfo = part_a.dsp.synthone.lfo[0];
        const float wave = fm_sin_pi((lfo.phase > .5f ? lfo.phase - 1.f : lfo.phase) * FM_TWO_PI);
        const float smoothed = 0x1.8338p-9f*wave + 0x1.fe7cc8p-1f*lfo.smooth;
        const float ratio = 1.f + .0594630944f * (.5f * (1.f + smoothed));
        const float increment = voice->phase_increment;
        const uint32_t step = voice->sub.step;
        float expected = voice->phase + increment * ratio;
        if (expected >= 1.f) expected -= 1.f;
        assert(original_runtime_render(&part_a, &workspace, al, ar, 1) == ORIGINAL_RUNTIME_OK);
        assert(voice->phase == expected && voice->phase_increment == increment && voice->sub.step == step);
    }
    puts("Synth One pitch LFO: 8,192 inactive stereo frames bit-identical, stepped and overlapping notes retain base steps.");
}

static void alternate_filter_live_cutoff(void)
{
    float peak[2] = {0};
    for (unsigned kind = 1; kind < 3; ++kind) {
        for (unsigned resonance = 0; resonance <= 4; ++resonance) {
            s1_dry_voice voice = {0};
            s1_dry_controls controls;
            s1_dry_frozen_controls(&controls);
            controls.filter_type = (float)kind;
            controls.resonance = resonance * .245f;
            controls.filter_depth = 1.2f;
            assert(s1_dry_configure(&voice, &controls) == S1_DRY_OK);
            assert(s1_dry_start(&voice, 194.209f, 127, 0.f) == S1_DRY_OK);
            for (unsigned stage = 0; stage <= 24; ++stage) {
                controls.cutoff_hz = stage % 4 < 2 ? 22050.f : 64.f;
                controls.filter_depth = stage % 2 ? 1.2f : 0.f;
                assert(s1_dry_configure(&voice, &controls) == S1_DRY_OK);
                for (unsigned frame = 0; frame < 22050u; ++frame) {
                    const float sample = fabsf(s1_dry_sample(&voice));
                    assert(sample < 2.f);
                    if (sample > peak[kind - 1]) peak[kind - 1] = sample;
                }
            }
        }
    }
    printf("Synth One alternate-filter live cutoff: 5,512,500 finite samples, bandpass peak %.6f, highpass peak %.6f.\n",
           peak[0], peak[1]);
}
#endif

int main(void)
{
    width_patch();
    uint8_t master_cell[32],extra[S1_MASTER_EXT_SIZE]={0};
    assert(original_sound_init(master_cell,ORIGINAL_SOUND_SYNTHONE,0));
    assert(original_sound_set_extended(master_cell,extra,ORIGINAL_SYNTHONE_MASTER_RATIO,73));
    assert(original_runtime_init(&part_a,master_cell,120,NULL,NULL)==ORIGINAL_RUNTIME_INVALID);
    assert(original_runtime_init_extended(&part_a,master_cell,extra,120,NULL,NULL)==ORIGINAL_RUNTIME_OK);
    assert(part_a.dsp.synthone.master.active==63 && part_a.dsp.synthone.master.knobs[1]==73);
    extra[3]=128;
    assert(original_runtime_init_extended(&part_a,master_cell,extra,120,NULL,NULL)==ORIGINAL_RUNTIME_INVALID);

    crush_patch();
#if X0X_SYNTHONE_PATCH
    sub_bypass();
    patch_audio();
    alternate_filter_live_cutoff();
    pitch_lfo_step();
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
