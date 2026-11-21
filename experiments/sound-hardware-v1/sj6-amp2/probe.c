/* SPDX-License-Identifier: GPL-3.0-only */
#include "original_runtime.h"
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Replay may also compile against the previous facade to demonstrate the RED. */
#ifndef JUNO6_APP_PARAM_BASE
#define JUNO6_APP_PARAM_BASE 256u
#endif

#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #expr); exit(1); } } while (0)
enum { frames = 256, records = 640 };
static original_runtime_part low, high, peer, saved;
static original_runtime_workspace workspace;

static dnbx_part *core(original_runtime_part *owner)
{
    return &owner->dsp.juno.synth.core;
}

static void route(original_runtime_part *owner, unsigned id, float raw)
{
    CHECK(original_runtime_param(owner, JUNO6_APP_PARAM_BASE + id, raw) == ORIGINAL_RUNTIME_OK);
}

static void init(original_runtime_part *owner, unsigned factory)
{
    uint8_t cell[32];
    CHECK(original_sound_init(cell, ORIGINAL_SOUND_JUNO, factory));
    CHECK(original_runtime_init(owner, cell, 120, NULL, NULL) == ORIGINAL_RUNTIME_OK);
    CHECK(original_runtime_param(owner, 14, 0) == ORIGINAL_RUNTIME_OK); /* dry chorus */
    for (unsigned i = 0; i < 2; ++i) {
        dnbx_layer_params layer;
        CHECK(dnbx_get_layer(core(owner), i, &layer) == DNBX_OK);
        layer.amp = (dnbx_envelope_params){.attack_s=.002f, .decay_s=.02f, .sustain=.4f, .release_s=.03f};
        layer.pan = i ? 1 : -1;
        layer.gain = .55f;
        CHECK(dnbx_set_layer(core(owner), i, &layer) == DNBX_OK);
    }
}

static double oracle(unsigned stage, float raw)
{
    if (stage == 0) return .002 + 15.998 * pow(raw, 3.33);
    if (stage == 2) return pow(raw, 2.5);
    return .002 + 23.998 * pow(raw, 3.35);
}

static float *stage_field(dnbx_layer_params *p, unsigned stage)
{
    switch (stage) {
    case 0: return &p->amp.attack_s;
    case 1: return &p->amp.decay_s;
    case 2: return &p->amp.sustain;
    default: return &p->amp.release_s;
    }
}

static void render(original_runtime_part *p, float *left, float *right)
{
    CHECK(original_runtime_render(p, &workspace, left, right, frames) == ORIGINAL_RUNTIME_OK);
}

static void note(original_runtime_part *p, unsigned velocity)
{
    CHECK(original_runtime_key(p, ORIGINAL_INPUT_KEYS, 60, velocity) == ORIGINAL_RUNTIME_OK);
}

static void corpus(FILE *f, double *largest_error, unsigned *long_decays)
{
    unsigned bound = 0, stored = 0, globals = 0;
    for (unsigned record = 0; record < records; ++record) {
        uint16_t count;
        uint8_t present[DNBX_RAW_CONTROLS] = {0};
        float expected[DNBX_RAW_CONTROLS] = {0};
        CHECK(fread(&count, sizeof count, 1, f) == 1 && count <= DNBX_RAW_CONTROLS);
        init(&low, 0);
        /* Deliberately different from each bank's globally owned key lock. */
        for (unsigned id = 212; id <= 214; ++id) {
            expected[id] = 0.456f;
            CHECK(dnbx_store_raw(core(&low), id, expected[id]) == DNBX_RAW_EXTERNALLY_OWNED);
            present[id] = 1;
        }
        for (unsigned n = 0; n < count; ++n) {
            uint8_t id;
            float raw;
            CHECK(fread(&id, 1, 1, f) == 1 && id < DNBX_RAW_CONTROLS);
            CHECK(fread(&raw, sizeof raw, 1, f) == 1 && isfinite(raw));
            if (id >= 212 && id <= 214) { ++globals; continue; }
            CHECK(!present[id]);
            present[id] = 1;
            expected[id] = raw;
            if (id < 119 || id > 122) {
                CHECK(dnbx_store_raw(core(&low), id, raw) == DNBX_RAW_STORED_ONLY);
                ++stored;
                continue;
            }
            unsigned stage = id - 119;
            ++bound;
            dnbx_layer_params primary, previous, actual;
            CHECK(dnbx_get_layer(core(&low), 0, &primary) == DNBX_OK);
            CHECK(dnbx_get_layer(core(&low), 1, &previous) == DNBX_OK);
            route(&low, id, raw);
            CHECK(memcmp(&primary, &core(&low)->layer[0].params, sizeof primary) == 0);
            CHECK(dnbx_get_layer(core(&low), 1, &actual) == DNBX_OK);
            float physical = *stage_field(&actual, stage);
            *stage_field(&previous, stage) = physical;
            CHECK(memcmp(&previous, &actual, sizeof actual) == 0);
            double error = fabs(physical - oracle(stage, raw));
            CHECK(isfinite(physical) && error <= 1e-5);
            if (error > *largest_error) *largest_error = error;
            if (stage == 1 && physical > 16) ++*long_decays;
            float readback;
            CHECK(dnbx_read_raw(core(&low), id, &readback) == DNBX_OK);
            CHECK(memcmp(&readback, &raw, sizeof readback) == 0);
        }
        for (unsigned id = 0; id < DNBX_RAW_CONTROLS; ++id) {
            float readback = 0;
            int status = dnbx_read_raw(core(&low), id, &readback);
            if (present[id]) {
                CHECK(status == DNBX_OK && memcmp(&readback, expected + id, sizeof readback) == 0);
            } else {
                CHECK(status == DNBX_E_ABSENT);
            }
        }
    }
    CHECK(bound == 2560 && stored == 87640 && globals == 75);
    CHECK(fgetc(f) == EOF && !ferror(f));
}

static unsigned listen(unsigned stage)
{
    float l0[frames], r0[frames], l1[frames], r1[frames];
    unsigned changes = 0, audible_primary = 0;
    init(&low, 0);
    high = low;
    route(&low, 119 + stage, 0);
    route(&high, 119 + stage, .75f);
    note(&low, 110);
    note(&high, 110);
    for (unsigned block = 0; block < 128; ++block) {
        if (block == 64) { note(&low, 0); note(&high, 0); }
        render(&low, l0, r0);
        render(&high, l1, r1);
        CHECK(memcmp(l0, l1, sizeof l0) == 0);
        for (unsigned i = 0; i < frames; ++i) {
            CHECK(isfinite(l0[i]) && isfinite(r0[i]) && isfinite(r1[i]));
            if (stage == 3 && block < 64) CHECK(r0[i] == r1[i]);
            if (fabsf(l0[i]) > .001f) ++audible_primary;
            if (r0[i] != r1[i]) ++changes;
        }
    }
    CHECK(audible_primary && changes > frames);
    return changes;
}

static void ranges(void)
{
    dnbx_part generic;
    dnbx_layer_params p;
    init(&low, 0);
    route(&low, 120, 1);
    route(&low, 122, 1);
    CHECK(original_runtime_param(&low, 16, .6f) == ORIGINAL_RUNTIME_OK);
    CHECK(dnbx_get_layer(core(&low), 1, &p) == DNBX_OK);
    CHECK(p.amp.decay_s == 24 && p.amp.release_s == 24);
    CHECK(dnbx_init(&generic, 44100) == DNBX_OK);
    CHECK(dnbx_set_layer(&generic, 1, &p) == DNBX_E_RANGE);
    p.filter.decay_s = 24;
    CHECK(dnbx_set_layer(core(&low), 1, &p) == DNBX_E_RANGE);
    saved = low;
    CHECK(!juno6_set(&low.dsp.juno.synth, JUNO6_APP_PARAM_BASE + 119, NAN));
    CHECK(!juno6_set(&low.dsp.juno.synth, JUNO6_APP_PARAM_BASE + 122, 1.01f));
    CHECK(!juno6_set(&low.dsp.juno.synth, JUNO6_APP_PARAM_BASE + 123, 1));
    CHECK(!juno6_set(&low.dsp.juno.synth, 119, .5f));
    CHECK(memcmp(&saved, &low, sizeof low) == 0);
}

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    FILE *f = fopen(argv[1], "rb");
    CHECK(f != NULL);
    init(&peer, 1);
    note(&peer, 100);
    float left[frames], right[frames];
    render(&peer, left, right);
    saved = peer;
    double error = 0;
    unsigned decays = 0;
    corpus(f, &error, &decays);
    CHECK(fclose(f) == 0 && decays == 22);
    unsigned changed[4];
    for (unsigned i = 0; i < 4; ++i) changed[i] = listen(i);
    CHECK(memcmp(&peer, &saved, sizeof peer) == 0);
    render(&peer, left, right);
    unsigned audible = 0;
    for (unsigned i = 0; i < frames; ++i) if (fabsf(left[i]) > .001f) ++audible;
    CHECK(audible);
    ranges();
    printf("{\"factory_records\":640,\"field_edits\":2560,\"long_decays\":%u,"
           "\"raw_only_fields\":87640,\"global_fields_skipped\":75,\"presence_readbacks\":139520,"
           "\"maximum_absolute_error\":%.12g,\"layer1_changed_samples\":0,"
           "\"layer2_changed_samples\":[%u,%u,%u,%u],\"frames_per_audio_pair\":32768,"
           "\"peer_audible_samples\":%u,\"part_bytes\":%lu,\"juno_bytes\":%lu}\n",
           decays,error,changed[0],changed[1],changed[2],changed[3],audible,
           (unsigned long)sizeof(dnbx_part), (unsigned long)sizeof(juno6_t));
}
