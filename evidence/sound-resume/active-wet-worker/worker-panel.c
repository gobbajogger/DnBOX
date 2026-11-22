/* SPDX-License-Identifier: GPL-3.0-only */
#define AUDIO_PATH_IDENTITY_LIBRARY
#include "tests/host/audio_path_identity.c"
#include "drum808.h"
#include "drum909.h"

unsigned test_worker_mask;
extern volatile uint32_t sound_chorus_probe_mode[2];

static void set_control(int route, int target, int owner, int knob, int value)
{
    pref_t ref = PR(R_ENG, target, owner, knob);
    const x0x_param_t *descriptor = pref_desc(ref);
    assert(descriptor && descriptor->max >= value);
    if (route == 1) {
        engine_set(target, owner, knob, value);
    } else {
        pref_set(ref, value);
        assert(pref_get(ref) == value);
        assert(proj.sound.v[target][owner][knob] == value);
    }
}

int main(int argc, char **argv)
{
    assert(argc == 5);
    const int pair = atoi(argv[1]), route = atoi(argv[3]);
    test_worker_mask = (unsigned)atoi(argv[2]);
    boot();
    engine_guard_enable(0);
    assert(plat_cpu2_set(test_worker_mask != 0) == 0);
    const int left[] = { ORIGINAL_SOUND_MIAMI, ORIGINAL_SOUND_JUNO, ORIGINAL_SOUND_CHICAGO, ORIGINAL_SOUND_SYNTHONE };
    const int right[] = { ORIGINAL_SOUND_CHICAGO, ORIGINAL_SOUND_SYNTHONE, ORIGINAL_SOUND_MIAMI, ORIGINAL_SOUND_JUNO };
    assert(pair >= 0 && pair < 4);
    if (pair) {
        assert(engine_bass_select_preset(0, left[pair], 0, &proj.sound) >= 0);
        assert(engine_bass_select_preset(1, right[pair], 0, &proj.sound) >= 0);
        for (unsigned i = 0; i < 32; ++i) {
            int32_t lr[512];
            engine_render(lr, 256);
        }
    }
    assert(engine_bass_bank(0) == left[pair] && engine_bass_bank(1) == right[pair]);
    if (pair == 1 || pair == 3) {
        const unsigned owner = pair == 1 ? 0u : 1u;
        assert(sound_chorus_probe_mode[owner] == 2);
        assert(sound_chorus_probe_mode[owner ^ 1u] == 0);
    }
    FILE *out = fopen(argv[4], "wb");
    assert(out);
    if (route) {
        set_control(route, T_808, D8_KIT, D8_KIT_GLUE, 90);
        set_control(route, T_808, D8_KIT, D8_KIT_DRIVE, 83);
        set_control(route, T_MIX, PART_808, MX_PAN, 94);
        set_control(route, T_MIX, PART_808, MX_REV, 73);
        set_control(route, T_MIX, PART_808, MX_DLY, 61);
        set_control(route, T_MST, 0, MST_COMP1, 58);
    }
    fill_pattern(1);
    seq_start(&seq);
    unsigned heard = 0;
    for (unsigned block = 0; block < 48; ++block) {
        int32_t lr[512];
        now_ms = (uint32_t)((uint64_t)block * 256u * 1000u / 44100u);
        if (block % 8 == 0) {
            for (unsigned voice = 0; voice < 11; ++voice) {
                engine_drum(0, (int)voice, .86f);
                engine_drum(1, (int)voice, .93f);
            }
            engine_bass_on(0, 48 + block % 5, 1, 0);
            engine_bass_on(1, 55 + block % 7, 1, 1);
        }
        if (block == 12 || block == 30) {
            engine_bass_off(0);
            engine_bass_off(1);
        }
        if (block == 16) engine_smc_solo((1u << PART_303A) | (1u << PART_808));
        if (block == 24) engine_smc_solo(0);
        engine_render(lr, 256);
        assert(engine_bass_status(0) > 0 && engine_bass_status(1) > 0);
        for (unsigned i = 0; i < 512; ++i) heard |= lr[i] != 0;
        assert(fwrite(lr, sizeof lr, 1, out) == 1);
        engine_volume(lr, 256);
        assert(fwrite(lr, sizeof lr, 1, out) == 1);
        for (unsigned part = 0; part < NPARTS; ++part) {
            const uint16_t peak = eng_peak[part];
            assert(fwrite(&peak, sizeof peak, 1, out) == 1);
        }
    }
    assert(heard);
    return fclose(out) != 0;
}
