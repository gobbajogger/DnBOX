/* SPDX-License-Identifier: GPL-3.0-only */
/* Verify the shipping WIDE selector and saved-project path against real DSP state. */
#define main simulator_main
#include "../../host/x0x_host.c"
#undef main
#include "app/original_runtime.h"
#include "app/bass_note_sound.h"
#include "dsp/juno6_controls.h"

#if !X0X_SUPERJ6_STOCK_PRESET || !X0X_SUPERJ6_FACTORY
#error WIDE regression requires both stock-preset and factory build options
#endif

static unsigned checks;
#define CHECK(condition) do { \
    ++checks; \
    if (!(condition)) { \
        fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #condition); \
        exit(1); \
    } \
} while (0)

/* Linker wraps record runtime addresses while leaving both initializers intact. */
static juno6_t *last_synth;
static struct { juno6_t *synth; juno6_chorus_t *chorus; } owners[NBASS];
static unsigned observed;
void __real_juno6_init(juno6_t *, float);
void __real_juno6_chorus_init(juno6_chorus_t *, float);
void __wrap_juno6_init(juno6_t *synth, float rate)
{
    __real_juno6_init(synth, rate);
    last_synth = synth;
}

void __wrap_juno6_chorus_init(juno6_chorus_t *chorus, float rate)
{
    __real_juno6_chorus_init(chorus, rate);
    CHECK(last_synth != NULL);
    unsigned owner = 0;
    while (owner < observed && owners[owner].synth != last_synth) ++owner;
    CHECK(owner < NBASS);
    if (owner == observed) ++observed;
    owners[owner].synth = last_synth;
    owners[owner].chorus = chorus;
    last_synth = NULL;
}

static int render(void)
{
    int32_t audio[512];
    engine_render(audio, 256);
    for (unsigned i = 0; i < 512; ++i) if (audio[i]) return 1;
    return 0;
}

static void select_preset(unsigned owner, unsigned preset)
{
    CHECK(engine_bass_select_preset(owner, ORIGINAL_SOUND_JUNO, preset, &proj.sound) == 0);
    CHECK(engine_bass_preset_pending(owner));
    for (unsigned i = 0; i < 2048 && engine_bass_preset_pending(owner); ++i) render();
    CHECK(!engine_bass_preset_pending(owner));
    CHECK(engine_bass_bank(owner) == ORIGINAL_SOUND_JUNO && engine_bass_status(owner) == ORIGINAL_RUNTIME_OK);
    CHECK(engine_bass_preset_index(owner) == (int)preset && proj.sound.v[T_303][owner][30] == preset);
}

static void check_catalog(void)
{
    static const char *const names[] = {
        "SUPERJ6 POLY", "SUPERJ6 PAD", "SUPERJ6 PLUCK", "SUPERJ6 BRASS", "SUPERJ6 WIDE"
    };
    CHECK(ORIGINAL_JUNO_WIDE_PRESET == 4 && original_sound_factories(ORIGINAL_SOUND_JUNO) == 5);
    for (unsigned id = 0; id < 5; ++id) {
        unsigned kind, one_based;
        uint8_t cell[ORIGINAL_SOUND_BYTES];
        CHECK(!strcmp(original_sound_name(ORIGINAL_SOUND_JUNO, id), names[id]));
        CHECK(original_sound_init(cell, ORIGINAL_SOUND_JUNO, id) && cell[30] == id);
        CHECK(bass_pattern_preset_encode(ORIGINAL_SOUND_JUNO, id + 1) == 0xE1u + id);
        CHECK(bass_pattern_preset_decode(0xE1u + id, &kind, &one_based));
        CHECK(kind == ORIGINAL_SOUND_JUNO && one_based == id + 1);
    }
    unsigned kind, one_based;
    CHECK(!bass_pattern_preset_decode(0xE6u, &kind, &one_based));
    CHECK(!bass_pattern_preset_encode(ORIGINAL_SOUND_JUNO, 6));
}

static void check_bridge(unsigned owner, float waveform)
{
    juno6_t *synth = owners[owner].synth;
    juno6_chorus_t *chorus = owners[owner].chorus;
    CHECK(synth->stock_preset && synth->typed);
    CHECK(chorus->factory.mode == 2 && chorus->factory.follow_panel && chorus->factory.history);
    CHECK(chorus->factory.mix == chorus->mix);
    for (unsigned layer = 0; layer < DNBX_LAYERS; ++layer) {
        dnbx_layer_params params;
        CHECK(dnbx_get_layer(&synth->core, layer, &params) == DNBX_OK);
        CHECK(params.oscillator_mode == DNBX_OSC_SUPERJ6 && params.waveform == waveform);
    }
}

static void check_pwm(unsigned owner, unsigned depth, unsigned width)
{
    for (unsigned layer = 0; layer < DNBX_LAYERS; ++layer) {
        dnbx_layer_params params;
        CHECK(dnbx_get_layer(&owners[owner].synth->core, layer, &params) == DNBX_OK);
        CHECK(fabsf(params.lfo_pwm_depth - juno6_control_physical(11, depth / 127.f)) < 1e-6f);
        CHECK(fabsf(params.pulse_width - juno6_control_physical(12, width / 127.f)) < 1e-6f);
    }
}

static void first_note(unsigned owner)
{
    juno6_t *synth = owners[owner].synth;
    juno6_chorus_t *chorus = owners[owner].chorus;
    CHECK(!juno6_active(synth) && !dnbx_active_voices(&synth->core));
    uint32_t phase = chorus->factory.phase;
    CHECK(!SEQ_PART_OFF(&seq, TRK_BASS0 + owner));
    engine_bass_on_velocity(owner, 60 + owner * 7, 0, 0, 110);
    unsigned audible = 0;
    for (unsigned i = 0; i < 8 && !dnbx_active_voices(&synth->core); ++i) audible |= render();
    CHECK(dnbx_active_voices(&synth->core) && juno6_active(synth));
    CHECK(chorus->factory.mode == 2 && chorus->factory.phase != phase);
    for (unsigned i = 0; i < 8; ++i) audible |= render();
    CHECK(audible);
    engine_request_notes_off();
    for (unsigned i = 0; i < 8; ++i) render();
    CHECK(!juno6_active(synth));
}

/* Use the same persistent sound-cell update and queued audio control as the panel. */
static void panel_edit(unsigned owner, unsigned parameter, unsigned value)
{
    CHECK(original_sound_set_extended(proj.sound.v[T_303][owner],
          proj.sound.v[T_303][NBASS + owner], parameter, value));
    engine_bass_sound_refresh(&proj.sound, owner);
    engine_set(T_303, owner, parameter, value);
    render();
}

static void capture_lock(unsigned owner, unsigned step, unsigned preset)
{
    engine_note_lock_capture(1);
    CHECK(engine_bass_note_capture(proj.arr.lane, 0, owner, step, &proj.sound) == 0);
    engine_note_lock_capture(0);
    int index = bass_note_find(proj.arr.lane, 0, owner, step);
    CHECK(index >= 0 && proj.arr.lane[index].val[30] == preset);
}

static void play_to_step(unsigned step)
{
    for (unsigned i = 0; i < 512; ++i) {
        render();
        if (seq.playing && seq.t[TRK_BASS0].started && seq.t[TRK_BASS0 + 1].started &&
            eng_step[TRK_BASS0] == step && eng_step[TRK_BASS0 + 1] == step) return;
    }
    CHECK(0);
}

int main(void)
{
    load_force = 0;
    boot();
    check_catalog();
    for (unsigned owner = 0; owner < NBASS; ++owner) {
        for (unsigned legacy = 0; legacy < ORIGINAL_JUNO_WIDE_PRESET; ++legacy) {
            select_preset(owner, legacy);
            CHECK(!owners[owner].synth->typed && !owners[owner].synth->stock_preset);
            CHECK(owners[owner].chorus->factory.mode == 0);
        }
        select_preset(owner, ORIGINAL_JUNO_WIDE_PRESET);
        check_bridge(owner, 1.f / 3.f);
    }
    CHECK(owners[0].chorus->factory.history != owners[1].chorus->factory.history);
    for (unsigned owner = 0; owner < NBASS; ++owner) first_note(owner);
    puts("WIDE: IDs E1–E5, stock selector, independent mode-2 chorus, typed first notes on A and B");

    float locked_mix[NBASS], base_mix[NBASS], legacy_locked_mix[NBASS], legacy_base_mix[NBASS];
    for (unsigned owner = 0; owner < NBASS; ++owner) {
        juno6_chorus_t *chorus = owners[owner].chorus;
        const float initial_wet = chorus->factory.mix;
        const uint32_t initial_step = chorus->factory.phase_step;
        panel_edit(owner, 0, 127);                   /* Wave */
        panel_edit(owner, 13, 97);                   /* ChoRate */
        panel_edit(owner, 14, 24 + 12 * owner);      /* Chorus */
        check_bridge(owner, 1.f);
        CHECK(chorus->factory.mix != initial_wet && chorus->factory.phase_step != initial_step);
        locked_mix[owner] = chorus->factory.mix;
        bpart_t *part = &proj.pat[0].bass[owner];
        for (unsigned step = 0; step < NSTEPS; ++step) part->step[step].flags = G_REST;
        part->len = 16;
        part->rate = RATE_16;
        part->step[0] = (bstep_t){.flags = G_NOTE, .note = 60 + owner * 7, .velocity = 100};
        part->step[4] = (bstep_t){.flags = G_NOTE, .note = 64 + owner * 7, .velocity = 100};
        part->step[8] = (bstep_t){.flags = G_NOTE, .note = 67 + owner * 7, .velocity = 100};
        capture_lock(owner, 0, ORIGINAL_JUNO_WIDE_PRESET);
        select_preset(owner, 0);
        panel_edit(owner, 0, 64);
        panel_edit(owner, 14, 4 + owner);
        legacy_locked_mix[owner] = chorus->mix;
        capture_lock(owner, 8, 0);
        select_preset(owner, ORIGINAL_JUNO_WIDE_PRESET);
        proj.pat[0].rsv[owner] = 0xE5u;
        panel_edit(owner, 0, 0);
        panel_edit(owner, 14, 103 - 12 * owner);
        panel_edit(owner, 11, 70 + 5 * owner);       /* PWM depth */
        panel_edit(owner, 12, 30 + 10 * owner);      /* PWM width */
        check_pwm(owner, 70 + 5 * owner, 30 + 10 * owner);
        base_mix[owner] = chorus->factory.mix;
        CHECK(base_mix[owner] != locked_mix[owner]);
        check_bridge(owner, 0.f);
    }

    CHECK(project_save() == 0);
    for (unsigned owner = 0; owner < NBASS; ++owner) {
        select_preset(owner, 0);
        CHECK(owners[owner].chorus->factory.mode == 0 && !owners[owner].synth->typed);
    }
    CHECK(ui_project_load() == 0);
    render();  /* Consume the project transaction's audio queue handoff. */
    for (unsigned owner = 0; owner < NBASS; ++owner) {
        CHECK(proj.pat[0].rsv[owner] == 0xE5u);
        CHECK(engine_bass_preset_index(owner) == ORIGINAL_JUNO_WIDE_PRESET);
        CHECK(owners[owner].chorus->factory.mix == base_mix[owner]);
        check_bridge(owner, 0.f);
        check_pwm(owner, 70 + 5 * owner, 30 + 10 * owner);
        CHECK(bass_note_find(proj.arr.lane, 0, owner, 0) >= 0);
        CHECK(bass_note_find(proj.arr.lane, 0, owner, 8) >= 0);
        first_note(owner);
    }
    puts("WIDE: saved project adopts both typed owners, Wave/Chorus and PWM edits, and E5 recalls");

    engine_note_lock_refresh();
    seq_start(&seq);
    play_to_step(0);
    for (unsigned owner = 0; owner < NBASS; ++owner) {
        CHECK(owners[owner].chorus->factory.mix == locked_mix[owner]);
        check_bridge(owner, 1.f);
    }
    play_to_step(4);
    for (unsigned owner = 0; owner < NBASS; ++owner) {
        CHECK(owners[owner].chorus->factory.mix == base_mix[owner]);
        check_bridge(owner, 0.f);
    }
    play_to_step(8);
    for (unsigned owner = 0; owner < NBASS; ++owner) {
        CHECK(owners[owner].chorus->factory.mix == base_mix[owner]);
        check_bridge(owner, 0.f);
    }
    seq_stop(&seq);
    render();
    CHECK(!seq.playing);
    engine_request_notes_off();
    render();
    for (unsigned owner = 0; owner < NBASS; ++owner) {
        select_preset(owner, 0);
        proj.pat[0].rsv[owner] = 0xE1u;
        panel_edit(owner, 0, 0);
        panel_edit(owner, 14, 90 + owner);
        legacy_base_mix[owner] = owners[owner].chorus->mix;
        CHECK(legacy_base_mix[owner] != legacy_locked_mix[owner]);
    }
    seq_start(&seq);
    play_to_step(0);
    for (unsigned owner = 0; owner < NBASS; ++owner) {
        CHECK(!owners[owner].synth->typed && owners[owner].chorus->factory.mode == 0);
        CHECK(owners[owner].synth->macro[0] == 0.f && owners[owner].chorus->mix == legacy_base_mix[owner]);
    }
    play_to_step(8);
    for (unsigned owner = 0; owner < NBASS; ++owner) {
        CHECK(owners[owner].synth->macro[0] > 0.f && owners[owner].chorus->mix == legacy_locked_mix[owner]);
    }
    play_to_step(0);
    for (unsigned owner = 0; owner < NBASS; ++owner) {
        CHECK(owners[owner].synth->macro[0] == 0.f && owners[owner].chorus->mix == legacy_base_mix[owner]);
    }
    seq_stop(&seq);
    render();
    CHECK(!seq.playing);
    printf("WIDE: both owners apply and restore matching locks and skip cross-topology locks (%u checks)\n", checks);
    return 0;
}
