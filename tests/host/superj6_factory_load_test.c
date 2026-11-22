/* SPDX-License-Identifier: GPL-3.0-only */
/* Exercise the foreground loader and real audio boundary on two Juno owners. */
#define SMC_UI_MAIN retained_ui_tests
#include "smc_ui_test.c"
#include "app/original_runtime.h"
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

/* API slots 0..10. Expected IDs are independent of the decoder. */
static const unsigned id[] = {119,120,121,122,0,82,83,78,79,210,211};
static const uint32_t example[] = {
    0x80000000,0x3e800000,0x3f000000,0x3f400000,0x3fa8f5c3,
    0x3f800000,0x3f800000,0x80000000,0x3e800000,0x3f800000,0x80000000
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
    rejected(0, NULL);
    CHECK(!engine_superj6_factory_load(2, &full));
    for (unsigned i = 0; i < SJ6_ROW_COUNT; ++i) {
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
#ifdef SJ6_AUTHORED_BANK
static void check_bank(void)
{
    unsigned programs = 0, authored = 0, defaulted = 0;
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
            for (unsigned i = 0; i < SJ6_ROW_COUNT; ++i) {
                bits = 0;
                if (row.present & (1u << i)) {
                    memcpy(&bits, &row.value[i], sizeof bits);
                    ++authored;
                } else ++defaulted;
                check_word(v, i, bits);
            }
        }
    }
    printf("programs=%u authored_words=%u defaulted_pwm_words=%u\n", programs, authored, defaulted);
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
            for (unsigned i = 0; i < SJ6_ROW_COUNT; ++i) {
                unsigned present = (row.present >> i) & 1u;
                check_word(v, i, present ? example[i] : 0u);
                authored += present;
            }
            CHECK(!memcmp(old, peer->core.raw, sizeof old));
            CHECK(!memcmp(old_presence, peer->core.raw_present, sizeof old_presence));
        }
    }
    CHECK(authored == 288);
    printf("synthetic applications=32 authored_words=288 defaulted_pwm_words=64; rejection and group ownership PASS\n");
#ifdef SJ6_AUTHORED_BANK
    check_bank();
#endif
    return 0;
}
