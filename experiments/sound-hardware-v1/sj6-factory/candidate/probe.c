/* SPDX-License-Identifier: GPL-3.0-only */
/* Private native diagnostic for the optional production factory transaction. */
#define SMC_UI_MAIN unused_ui_checks
#include "smc_ui_test.c"
#include "app/original_runtime.h"
#include <stddef.h>

static original_runtime_part *instrument;
static original_runtime_workspace render_work;

void __real_juno6_init(juno6_t *s, float rate);
void __wrap_juno6_init(juno6_t *s, float rate)
{
    __real_juno6_init(s, rate);
    /* This test selects precisely one live Juno through the engine. */
    CHECK(instrument == NULL);
    instrument = (original_runtime_part *)((char *)s - offsetof(original_runtime_part, dsp.juno.synth));
}

static unsigned float_word(float value)
{
    uint32_t word;
    memcpy(&word, &value, sizeof word);
    return word;
}

static void snapshot(void)
{
    juno6_t *s = &instrument->dsp.juno.synth;
    juno6_chorus_t *c = &instrument->dsp.juno.chorus;
    printf("engine %d %d %08x\n", engine_bass_bank(0), engine_bass_preset_index(0), float_word(seq_tempo(&seq)));
    printf("master %08x\n", float_word(s->master_gain));
    printf("chorus %08x %08x %08x\n", float_word(c->rate_hz), float_word(c->depth_ms), float_word(c->mix));
    for (unsigned layer = 0; layer < DNBX_LAYERS; ++layer) {
        dnbx_layer_params params;
        CHECK(dnbx_get_layer(&s->core, layer, &params) == DNBX_OK);
        printf("layer %u %u %08x %08x %08x %08x %08x %08x %08x\n",
               layer, params.oscillator_mode, float_word(params.waveform), float_word(params.pulse_width),
               float_word(params.tune_semitones), float_word(params.amp.attack_s),
               float_word(params.amp.decay_s), float_word(params.amp.sustain), float_word(params.amp.release_s));
    }
    for (unsigned i = 0; i < DNBX_RAW_CONTROLS; ++i) {
        float readback;
        if (dnbx_read_raw(&s->core, i, &readback) == DNBX_OK)
            printf("stored %u %08x\n", i, float_word(readback));
    }
}

int main(int argc, char **argv)
{
    if (argc == 1) return 0; /* The repository host builder invokes the test once. */
    CHECK(argc == 2);
    if (!strcmp(argv[1], "--schema")) {
        for (unsigned id = 0; id < 220; ++id) {
            sj6_factory_row one = {0};
            sj6_factory_add_word(&one, id, 0x3f000000u);
            if (one.present) printf("accepted %u\n", id);
        }
        return 0;
    }
    unsigned controls, frames, block, count, frame[32], note[32], velocity[32];
    CHECK(scanf("%u%u%u%u", &controls, &frames, &block, &count) == 4);
    CHECK(controls <= 220 && count <= 32 && frames && block && block <= 256);
    setup(V_PART, PART_303A);
    CHECK(engine_bass_select_preset(0, ORIGINAL_SOUND_JUNO, 0, &proj.sound) == 0);
    int32_t stereo[512];
    for (unsigned i = 0; i < 1024 && engine_bass_preset_pending(0); ++i) engine_render(stereo, 256);
    CHECK(instrument && !engine_bass_preset_pending(0) && instrument->kind == ORIGINAL_SOUND_JUNO);
    sj6_factory_row row = {0};
    for (unsigned i = 0; i < controls; ++i) {
        unsigned id, bits;
        CHECK(scanf("%u%x", &id, &bits) == 2 && id < 220);
        sj6_factory_add_word(&row, id, bits);
    }
    for (unsigned i = 0; i < count; ++i) {
        CHECK(scanf("%u%u%u", &frame[i], &note[i], &velocity[i]) == 3 &&
              frame[i] < frames && (!i || frame[i] >= frame[i-1]) && note[i] < 128 && velocity[i] < 128);
    }
    CHECK(engine_superj6_factory_load(0, &row));
    memset(&row, 0x55, sizeof row); /* The readback must outlive the caller's input storage. */
    engine_render(stereo, 1);
    CHECK(instrument->dsp.juno.synth.typed && !instrument->dsp.juno.synth.pending_field_mask);
    snapshot();
    FILE *out = fopen(argv[1], "wb");
    CHECK(out != NULL);
    unsigned event = 0;
    for (unsigned pos = 0; pos < frames;) {
        while (event < count && frame[event] == pos) {
            CHECK(original_runtime_key(instrument, ORIGINAL_INPUT_KEYS, note[event], velocity[event]) == ORIGINAL_RUNTIME_OK);
            ++event;
        }
        unsigned n = block - pos % block;
        if (n > frames - pos) n = frames - pos;
        if (event < count && n > frame[event] - pos) n = frame[event] - pos;
        CHECK(n > 0);
        float left[256], right[256];
        CHECK(original_runtime_render(instrument, &render_work, left, right, n) == ORIGINAL_RUNTIME_OK);
        for (unsigned j = 0; j < n; ++j) {
            uint32_t words[2] = {float_word(left[j]), float_word(right[j])};
            for (unsigned channel = 0; channel < 2; ++channel)
                for (unsigned byte = 0; byte < 4; ++byte)
                    CHECK(fputc((words[channel] >> (8 * byte)) & 255u, out) != EOF);
        }
        pos += n;
    }
    CHECK(event == count && fclose(out) == 0);
    return 0;
}
