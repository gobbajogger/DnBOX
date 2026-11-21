/* SPDX-License-Identifier: GPL-3.0-only */
#define SJ6_APPLICATION_MAIN check_wave_and_amp2
#include "compact_application.c"
#undef SJ6_APPLICATION_MAIN
#include <math.h>
#include "master_oracle.inc"

static float to_float(uint32_t bits)
{
    float value;
    memcpy(&value, &bits, sizeof value);
    return value;
}

static float observed_gain(uint32_t bits)
{
    for (unsigned i = 0; i < sizeof observed / sizeof observed[0]; ++i)
        if (observed[i][0] == bits) return to_float(observed[i][1]);
    CHECK(0);
    return 0;
}

static void check_programs(void)
{
    unsigned programs = 0, above_one = 0;
    int32_t stereo[2];
    for (unsigned b = 0; b < BANK_COUNT; ++b) {
        for (unsigned p = 0; p < (unsigned)(bank_start[b+1] - bank_start[b]); ++p) {
            unsigned v = programs % 2, master_seen = 0, field;
            uint32_t bits;
            sj6_record_reader reader;
            CHECK(sj6_select_position(b, p, &reader));
            while (sj6_next_field(&reader, &field, &bits)) {
                if (field != ENGINE_SJ6_MASTER_VOLUME) continue;
                ++master_seen;
                const float raw = to_float(bits);
                float peer_gain = owner[v^1]->master_gain;
                float old_gain = owner[v]->master_gain;
                CHECK(engine_superj6_edit(v, ENGINE_SJ6_MASTER_VOLUME, 0));
                CHECK(engine_superj6_edit(v, ENGINE_SJ6_MASTER_VOLUME, raw));
                CHECK(owner[v]->master_gain == old_gain);
                engine_render(stereo, 1);
                check_raw(v, ENGINE_SJ6_MASTER_VOLUME, raw);
                CHECK(owner[v]->master_gain == observed_gain(bits));
                CHECK(owner[v^1]->master_gain == peer_gain);
                if (raw > 1.f) ++above_one;
            }
            CHECK(master_seen == 1);
            ++programs;
        }
    }
    CHECK(programs == 640 && above_one == 8);
    printf("factory_master_records=%u over_unity=%u\n", programs, above_one);
}

static void check_audio(void)
{
    static original_runtime_part reference, edited, dry;
    static original_runtime_workspace workspace;
    float left[256], right[256], got_left[256], got_right[256], dry_left[256], dry_right[256];
    uint8_t cell[32];
    unsigned compared = 0, sounding_tail = 0, wet_samples = 0;
    CHECK(original_sound_init(cell, ORIGINAL_SOUND_JUNO, 0));
    CHECK(original_runtime_init(&reference,cell,120,NULL,NULL) == ORIGINAL_RUNTIME_OK);
    CHECK(original_runtime_param(&reference,14,1.f) == ORIGINAL_RUNTIME_OK);
    CHECK(original_runtime_key(&reference,ORIGINAL_INPUT_KEYS,60,100) == ORIGINAL_RUNTIME_OK);
    edited = reference;
    dry = reference;
    CHECK(original_runtime_param(&dry,14,0.f) == ORIGINAL_RUNTIME_OK);
    /* Hold one note with an active chorus, then cut voices but retain its delay
     * history. Changing master on a running tail distinguishes post from pre-FX. */
    for (unsigned block = 0; block < 24; ++block) {
        unsigned n = block >= 13 ? 16 : 256;
        if (block == 13) {
            juno6_all_off(&reference.dsp.juno.synth);
            juno6_all_off(&edited.dsp.juno.synth);
            juno6_all_off(&dry.dsp.juno.synth);
        }
        const uint32_t inputs[] = {0x00000000,0x3e800000,0x3f000000,0x3f400000,0x3f800000,0x3fa8f5c3};
        unsigned input = inputs[block % (sizeof inputs / sizeof inputs[0])];
        CHECK(original_runtime_param(&edited,JUNO6_APP_PARAM_BASE + ENGINE_SJ6_MASTER_VOLUME,to_float(input)) == ORIGINAL_RUNTIME_OK);
        CHECK(original_runtime_render(&reference,&workspace,left,right,n) == ORIGINAL_RUNTIME_OK);
        CHECK(original_runtime_render(&edited,&workspace,got_left,got_right,n) == ORIGINAL_RUNTIME_OK);
        CHECK(original_runtime_render(&dry,&workspace,dry_left,dry_right,n) == ORIGINAL_RUNTIME_OK);
        const float gain = observed_gain(input);
        for (unsigned i = 0; i < n; ++i) {
            CHECK(got_left[i] == left[i]*gain && got_right[i] == right[i]*gain);
            if (left[i] != dry_left[i] || right[i] != dry_right[i]) ++wet_samples;
            if (block >= 13) {
                CHECK(dry_left[i] == 0 && dry_right[i] == 0);
                if (left[i] != 0 && right[i] != 0) ++sounding_tail;
            }
            ++compared;
        }
        CHECK(!memcmp(&reference.dsp.juno.chorus,&edited.dsp.juno.chorus,sizeof reference.dsp.juno.chorus));
    }
    CHECK(wet_samples > 100 && sounding_tail > 16);
    printf("master_stereo_frames=%u wet_frames=%u sounding_tail_frames=%u\n",compared,wet_samples,sounding_tail);
}

int main(void)
{
    open_parts();
    CHECK(!engine_superj6_edit(2, ENGINE_SJ6_MASTER_VOLUME, 1.f));
    CHECK(!engine_superj6_edit(0, ENGINE_SJ6_MASTER_VOLUME, NAN));
    CHECK(!engine_superj6_edit(0, ENGINE_SJ6_MASTER_VOLUME, INFINITY));
    CHECK(!engine_superj6_edit(0, ENGINE_SJ6_MASTER_VOLUME, -1.f));
    CHECK(!engine_superj6_edit(0, ENGINE_SJ6_MASTER_VOLUME, nextafterf(JUNO6_FACTORY_MASTER_MAX,2.f)));
    check_programs();
    check_audio();
    return 0;
}
