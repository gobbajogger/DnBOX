/* SPDX-License-Identifier: GPL-3.0-only */
/* Exercise the foreground loader and real audio boundary on two Juno owners. */
#define SMC_UI_MAIN retained_ui_tests
#include "smc_ui_test.c"
#include "app/original_runtime.h"
#include <stddef.h>
#ifdef SJ6_AUTHORED_BANK
#include "../../experiments/sound-hardware-v1/sj6-factory/bank_reader.c"
#endif

static juno6_t *owner[2];
static unsigned opens;
void __real_juno6_init(juno6_t *, float);
void __wrap_juno6_init(juno6_t *s, float rate)
{
    __real_juno6_init(s, rate);
    if (opens < 2) owner[opens++] = s;
}

/* Expected IDs follow the row ABI independently of the decoder. */
static const unsigned id[] = {119,120,121,122,0,82,83,78,79,210,211,86};
static const uint32_t example[] = {
    0x80000000,0x3e800000,0x3f000000,0x3f400000,0x3fa8f5c3,
    0x3f800000,0x3f800000,0x80000000,0x3e800000,0x3f800000,0x80000000,0x3f000000
};
static sj6_factory_row synthetic(unsigned presence)
{
    sj6_factory_row row = {0};
    for (int i = 10; i >= 0; --i) {
        if (i >= 7 && !(presence & (1u << (i-7)))) continue;
        sj6_factory_add_word(&row, id[i], example[i]);
        sj6_factory_add_word(&row, id[i], 0x7fc00000); /* first match wins */
    }
    sj6_factory_add_word(&row, 219, 0x7fc00000);
    return row;
}
static void boundary(void) { int32_t stereo[2]; engine_render(stereo, 1); }
static void check_word(unsigned v, unsigned i, uint32_t expected)
{
    float raw;
    CHECK(dnbx_read_raw(&owner[v]->core, id[i], &raw) == DNBX_OK);
    CHECK(!memcmp(&raw, &expected, sizeof raw));
}
static void rejected(unsigned v, const sj6_factory_row *row)
{
    juno6_t before = *owner[v];
    CHECK(!engine_superj6_factory_load(v, row));
    CHECK(!memcmp(&before, owner[v], sizeof before));
}
static void check_negative(void)
{
    sj6_factory_row full = synthetic(15);
    sj6_factory_add_word(&full, 86, 0x3f000000u);
    rejected(0, NULL);
    CHECK(!engine_superj6_factory_load(2, &full));
    for (unsigned i = 0; i < SJ6_ROW_CHORUS_I; ++i) {
        sj6_factory_row bad = full;
        if (i < SJ6_ROW_AMOUNT) {
            bad.present &= ~(1u << i);
            rejected(0, &bad);
        }
        static const uint32_t invalid[] = {0x7fc00000,0x7f800000,0xbf800000,0x3fc00000};
        for (unsigned k = 0; k < 4; ++k) {
            memcpy(&bad.value[i], &invalid[k], sizeof(float));
            bad.present = full.present;
            rejected(0, &bad);
        }
    }
    CHECK(engine_superj6_edit(0, ENGINE_SJ6_MASTER_VOLUME, .25f));
    rejected(0, &full);
    boundary();
    CHECK(engine_superj6_factory_load(0, &full));
    rejected(0, &full);
    CHECK(!engine_superj6_edit(0, ENGINE_SJ6_MASTER_VOLUME, .5f));
    CHECK(!engine_superj6_pre_note_wave(0, 1, 0, NULL));
    /* A voice acquired after admission cancels both mailboxes together. */
    CHECK(juno6_note_on(owner[0], 64, 100));
    rejected(0, &full);
    boundary();
    check_word(0, SJ6_ROW_MASTER, 0x3e800000);
    CHECK(!owner[0]->typed && !owner[0]->pending_wave_mask && !owner[0]->pending_field_mask);
    juno6_note_off(owner[0], 64);
    rejected(0, &full);
    juno6_all_off(owner[0]);
    CHECK(engine_bass_select_preset(0, ORIGINAL_SOUND_JUNO, 3, &proj.sound) == 0);
    rejected(0, &full);
    engine_bass_cancel_preset(0);
}
static void check_range(void)
{
    const uint32_t stops[] = {0x80000000u, 0x3f000000u, 0x3f800000u};
    /* Original CoreSampler note increments at 44.1 kHz, from the pinned
     * sampler_reference.cpp driver. Rows are 16/8/4-foot register, columns
     * are -2/0/+2 semitones of subsequent performance bend (MIDI 60 input). */
    const uint32_t pitch[3][3] = {
        {0x3b2d3057,0x3b4265db,0x3b5a3449},
        {0x3bad3056,0x3bc265db,0x3bda3449},
        {0x3c2d3057,0x3c4265db,0x3c5a3449}
    };
    unsigned frames = 0, changed = 0, maximum_ulps = 0;
    for (unsigned v = 0; v < 2; ++v) {
        sj6_factory_row neutral = synthetic(15);
        CHECK(engine_superj6_factory_load(v, &neutral));
        boundary();
        juno6_t base = *owner[v];
        for (unsigned i = 0; i < 3; ++i) {
            sj6_factory_row row = neutral;
            sj6_factory_add_word(&row, 86, stops[i]);
            sj6_factory_add_word(&row, 86, 0x7fc00000u);
            CHECK(engine_superj6_factory_load(v, &row));
            boundary();
            check_word(v, SJ6_ROW_RANGE, stops[i]);
            for (unsigned bend = 0; bend < 3; ++bend) {
                juno6_t actual = *owner[v], expected = base, unshifted = base;
                juno6_all_off(&actual);
                juno6_all_off(&expected);
                juno6_all_off(&unshifted);
                CHECK(juno6_note_on(&actual, 60, 100));
                CHECK(juno6_note_on(&expected, 48 + 12*i, 100));
                CHECK(juno6_note_on(&unshifted, 60, 100));
                float al[128], ar[128], el[128], er[128], ul[128], ur[128];
                for (unsigned block = 0; block < 8; ++block) {
                    if (block == 3) {
                        CHECK(juno6_set(&actual, 16, .5f*bend));
                        CHECK(juno6_set(&expected, 16, .5f*bend));
                        CHECK(juno6_set(&unshifted, 16, .5f*bend));
                    }
                    for (unsigned layer = 0; layer < 2; ++layer) {
                        float increment = actual.core.voice[0].layer[layer].increment;
                        CHECK(increment == expected.core.voice[0].layer[layer].increment);
                        uint32_t word;
                        memcpy(&word, &increment, sizeof word);
                        uint32_t target = pitch[i][block < 3 ? 1 : bend];
                        unsigned diff = word > target ? word - target : target - word;
                        CHECK(diff <= 2);
                        if (diff > maximum_ulps) maximum_ulps = diff;
                    }
                    CHECK(juno6_render_stereo(&actual, al, ar, 128));
                    CHECK(juno6_render_stereo(&expected, el, er, 128));
                    CHECK(juno6_render_stereo(&unshifted, ul, ur, 128));
                    CHECK(!memcmp(al, el, sizeof al) && !memcmp(ar, er, sizeof ar));
                    if (i != 1) {
                        CHECK(memcmp(al, ul, sizeof al) || memcmp(ar, ur, sizeof ar));
                        ++changed;
                    }
                    frames += 128;
                }
            }
        }
        sj6_factory_row off_stop = neutral;
        sj6_factory_add_word(&off_stop, 86, 0x3e800000u);
        rejected(v, &off_stop);
        CHECK(engine_superj6_factory_load(v, &neutral));
        boundary();
        check_word(v, SJ6_ROW_RANGE, 0x3f000000u);
        CHECK(owner[v]->core.layer[0].params.tune_semitones == owner[v]->bend);
        CHECK(owner[v]->core.layer[1].params.tune_semitones == owner[v]->bend);
    }
    CHECK(frames == 18432 && changed == 96);
    printf("range: two owners, three stops, three bend values, %u matched stereo frames, %u changed blocks, max pitch error %u ULP\n",
           frames, changed, maximum_ulps);
}
/* Each line is one authored range word from the authenticated metrics table.
 * Other controls in these rows are fixtures; this exercises the register only. */
static void check_register_file(const char *path)
{
    FILE *f = fopen(path, "r");
    CHECK(f != NULL);
    unsigned bits, rows = 0;
    while (fscanf(f, "%x", &bits) == 1) {
        sj6_factory_row row = synthetic(15);
        sj6_factory_add_word(&row, 86, bits);
        for (unsigned v = 0; v < 2; ++v) {
            CHECK(engine_superj6_factory_load(v, &row));
            boundary();
            check_word(v, SJ6_ROW_RANGE, bits);
            int semitones = bits == 0 ? -12 : bits == 0x3f800000u ? 12 : 0;
            for (unsigned layer = 0; layer < 2; ++layer)
                CHECK(owner[v]->core.layer[layer].params.tune_semitones == owner[v]->bend + semitones);
        }
        ++rows;
    }
    CHECK(feof(f) && rows == 640);
    fclose(f);
    printf("%u archived register words applied to both owners and both layers\n", rows);
}
static void check_layer_one_attack(void)
{
    const enum engine_superj6_field attack_id = ENGINE_SJ6_AMP1_ATTACK;
    static const float values[] = {0.f, .25f, .5f, .75f, 1.f};
    static const float seconds[] = {.002f, .1601998f, 1.5928741f, 6.1398985f, 16.f};
    unsigned rendered = 0;
    for (unsigned v = 0; v < 2; ++v) {
        juno6_t *s = owner[v];
        CHECK(!juno6_active(s));
        CHECK(!engine_superj6_edit(v, attack_id, -1.f));
        CHECK(!engine_superj6_edit(v, attack_id, 1.1f));
        CHECK(engine_superj6_edit(v, attack_id, .25f));
        CHECK(juno6_note_on(s, 60, 100));
        CHECK(!engine_superj6_edit(v, attack_id, .5f));
        boundary();
        float raw;
        CHECK(dnbx_read_raw(&s->core, 36, &raw) == DNBX_E_ABSENT);
        juno6_all_off(s);
        for (unsigned k = 0; k < sizeof values / sizeof *values; ++k) {
            juno6_t baseline = *s, expected = *s;
            const dnbx_layer_params layer_two = s->core.layer[1].params;
            const float peer_attack = owner[v ^ 1u]->core.layer[0].params.amp.attack_s;
            CHECK(engine_superj6_edit(v, attack_id, values[k]));
            CHECK(s->core.layer[0].params.amp.attack_s == baseline.core.layer[0].params.amp.attack_s);
            boundary();
            CHECK(dnbx_read_raw(&s->core, 36, &raw) == DNBX_OK && raw == values[k]);
            dnbx_layer_params target;
            CHECK(dnbx_get_layer(&expected.core, 0, &target) == DNBX_OK);
            CHECK(dnbx_attack_from_unit(values[k], &target.amp.attack_s) == DNBX_OK);
            float error = s->core.layer[0].params.amp.attack_s - seconds[k];
            CHECK(error >= -.00001f && error <= .00001f);
            CHECK(!memcmp(&s->core.layer[1].params, &layer_two, sizeof layer_two));
            CHECK(owner[v ^ 1u]->core.layer[0].params.amp.attack_s == peer_attack);
            CHECK(dnbx_set_layer(&expected.core, 0, &target) == DNBX_OK);
            juno6_t actual = *s;
            CHECK(juno6_note_on(&actual, 60, 100) && juno6_note_on(&expected, 60, 100));
            float al[256], ar[256], el[256], er[256];
            CHECK(juno6_render_stereo(&actual, al, ar, 256));
            CHECK(juno6_render_stereo(&expected, el, er, 256));
            CHECK(!memcmp(al, el, sizeof al) && !memcmp(ar, er, sizeof ar));
            rendered += 256;
        }
    }
    printf("Layer 1 attack: 10 endpoints, %u stereo frames bit-exact against direct physical settings\n", rendered);
}
static void check_factory_chorus_bypass(void)
{
    static original_runtime_part active;
    static original_runtime_workspace workspace;
    unsigned exact_frames = 0;
    for (unsigned v = 0; v < 2; ++v) {
        /* No complete factory row is persisted in the legacy project cell. */
        unsigned char saved_sound[sizeof proj.sound];
        memcpy(saved_sound, &proj.sound, sizeof saved_sound);
        for (unsigned mode = 0; mode < 4; ++mode) {
            sj6_factory_row row = synthetic(15);
            if (mode >= 1) sj6_factory_add_word(&row, 40, 0x80000000u);
            if (mode >= 2) sj6_factory_add_word(&row, 41, mode == 3 ? 0x3f800000u : 0u);
            unsigned peer_bypass = owner[v ^ 1u]->factory_chorus_bypass;
            CHECK(engine_superj6_factory_load(v, &row));
            CHECK(owner[v]->factory_chorus_bypass == (mode == 3));
            boundary();
            CHECK(owner[v ^ 1u]->factory_chorus_bypass == peer_bypass);
            const original_runtime_part *part = (const void *)((const unsigned char *)owner[v] -
                offsetof(original_runtime_part, dsp.juno.synth));
            active = *part;
            CHECK(active.dsp.juno.chorus.mix > 0.f);
            CHECK(juno6_note_on(&active.dsp.juno.synth, 60, 100));
            juno6_t dry = active.dsp.juno.synth;
            juno6_chorus_t chorus = active.dsp.juno.chorus;
            float al[256], ar[256], dl[256], dr[256];
            CHECK(juno6_render_stereo(&dry, dl, dr, 256));
            CHECK(original_runtime_render(&active, &workspace, al, ar, 256) == ORIGINAL_RUNTIME_OK);
            float gain = dry.master_gain;
            float energy = 0.f;
            for (unsigned i = 0; i < 256; ++i) {
                dl[i] *= gain;
                dr[i] *= gain;
                energy += dl[i] * dl[i] + dr[i] * dr[i];
            }
            CHECK(energy > 0.f);
            int exact = !memcmp(al, dl, sizeof al) && !memcmp(ar, dr, sizeof ar);
            CHECK(exact == (mode == 2));
            CHECK((!memcmp(&chorus, &active.dsp.juno.chorus, sizeof chorus)) == (mode == 2));
            if (exact) exact_frames += 256;
            CHECK(!memcmp(saved_sound, &proj.sound, sizeof saved_sound));
        }
    }
    sj6_factory_row off = synthetic(15);
    sj6_factory_add_word(&off, 40, 0u);
    sj6_factory_add_word(&off, 40, 0x3f800000u); /* first occurrence wins */
    sj6_factory_add_word(&off, 41, 0u);
    for (unsigned v = 0; v < 2; ++v) {
        sj6_factory_row bad = off;
        bad.value[SJ6_ROW_CHORUS_II] = 1.5f;
        rejected(v, &bad);
        CHECK(engine_superj6_factory_load(v, &off));
        boundary();
        CHECK(owner[v]->factory_chorus_bypass);
    }
    sound_t saved_sound = proj.sound;
    /* The SMC fixture force-selects its BT role without the persisted clock bit. */
    proj.set.bt |= PROJ_BT_CLK_IN;
    CHECK(project_save() == 0);
    CHECK(project_load() == 0);
    engine_apply_sound(&proj.sound);
    boundary();
    CHECK(!memcmp(&saved_sound, &proj.sound, sizeof saved_sound));
    for (unsigned v = 0; v < 2; ++v) {
        CHECK(engine_bass_bank(v) == ORIGINAL_SOUND_JUNO);
        CHECK(!owner[v]->factory_chorus_bypass && !owner[v]->typed);
    }
    printf("Factory chorus: signed-zero switches bypass bit-exactly over %u audible stereo frames\n",
           exact_frames);
}
#ifdef SJ6_AUTHORED_BANK
static void check_bank(void)
{
    unsigned programs = 0, authored = 0, defaulted = 0, register_words = 0;
    for (unsigned b = 0; b < BANK_COUNT; ++b) {
        for (unsigned p = 0; p < bank_start[b+1] - (unsigned)bank_start[b]; ++p) {
            sj6_record_reader reader;
            CHECK(sj6_select_position(b, p, &reader));
            sj6_factory_row row = {0};
            unsigned field;
            uint32_t bits;
            while (sj6_next_field(&reader, &field, &bits))
                sj6_factory_add_word(&row, field, bits);
            unsigned v = programs++ % 2;
            CHECK(engine_superj6_factory_load(v, &row));
            boundary();
            for (unsigned i = 0; i < SJ6_ROW_CHORUS_I; ++i) {
                bits = i == SJ6_ROW_RANGE ? 0x3f000000u : 0;
                if (row.present & (1u << i)) {
                    memcpy(&bits, &row.value[i], sizeof bits);
                    if (i == SJ6_ROW_RANGE) ++register_words;
                    else ++authored;
                } else if (i != SJ6_ROW_RANGE) ++defaulted;
                check_word(v, i, bits);
            }
        }
    }
    printf("programs=%u authored_words=%u defaulted_pwm_words=%u authored_register_words=%u\n",
           programs, authored, defaulted, register_words);
    CHECK(programs == 640 && authored == 4580 && defaulted == 2460);
}
#endif
int main(void)
{
    setup(V_PART, PART_303A);
    sj6_factory_row full = synthetic(15);
    CHECK(engine_bass_bank(0) != ORIGINAL_SOUND_JUNO);
    CHECK(!engine_superj6_factory_load(0, &full));
    for (unsigned v = 0; v < 2; ++v) {
        int32_t stereo[512];
        CHECK(engine_bass_select_preset(v, ORIGINAL_SOUND_JUNO, v, &proj.sound) == 0);
        for (unsigned n = 0; n < 1024 && engine_bass_preset_pending(v); ++n) engine_render(stereo, 256);
        CHECK(!engine_bass_preset_pending(v));
    }
    CHECK(opens == 2 && owner[0] != owner[1]);
    for (unsigned v = 0; v < 2; ++v) {
        float raw;
        CHECK(dnbx_read_raw(&owner[v]->core, 86, &raw) == DNBX_E_ABSENT);
    }
    check_negative();
    /* Both selected stock presets have an audible PWM LFO. An individual
     * waveform edit preserves it, and a full factory load supersedes it. */
    for (unsigned v = 0; v < 2; ++v) {
        float depth = owner[v]->core.layer[0].params.lfo_pwm_depth;
        CHECK(depth > 0.f);
        engine_superj6_pwm pwm = {.amount = .5f, .enable = 1.f};
        CHECK(engine_superj6_pre_note_wave(v, 0, 1.f, &pwm));
        boundary();
        CHECK(owner[v]->core.layer[0].params.lfo_pwm_depth == depth);
        CHECK(engine_superj6_factory_load(v, &full));
        boundary();
        for (unsigned layer = 0; layer < 2; ++layer)
            CHECK(owner[v]->core.layer[layer].params.lfo_pwm_depth == 0.f);
    }
    unsigned authored = 0;
    for (unsigned presence = 0; presence < 16; ++presence) {
        sj6_factory_row row = synthetic(presence);
        CHECK(row.present == (127u | (presence << 7)));
        for (unsigned v = 0; v < 2; ++v) {
            juno6_t *peer = owner[v^1u];
            float old[218];
            uint32_t old_presence[7];
            memcpy(old, peer->core.raw, sizeof old);
            memcpy(old_presence, peer->core.raw_present, sizeof old_presence);
            CHECK(engine_superj6_factory_load(v, &row));
            boundary();
            for (unsigned i = 0; i < SJ6_ROW_CHORUS_I; ++i) {
                unsigned present = (row.present >> i) & 1u;
                uint32_t fallback = i == SJ6_ROW_RANGE ? 0x3f000000u : 0u;
                check_word(v, i, present ? example[i] : fallback);
                authored += present;
            }
            CHECK(!memcmp(old, peer->core.raw, sizeof old));
            CHECK(!memcmp(old_presence, peer->core.raw_present, sizeof old_presence));
        }
    }
    CHECK(authored == 288);
    check_range();
    check_layer_one_attack();
    check_factory_chorus_bypass();
    const char *register_file = getenv("SJ6_REGISTER_WORDS");
    if (register_file) check_register_file(register_file);
    printf("synthetic applications=32 authored_words=288 defaulted_pwm_words=64; rejection and group ownership PASS\n");
#ifdef SJ6_AUTHORED_BANK
    check_bank();
#endif
    return 0;
}
