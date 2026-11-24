/* SPDX-License-Identifier: GPL-3.0-only */
#define ARP_FIXTURE_MAIN retained_arp_main
#include "ui_arp_engine_test.c"

/* This test has no new receiver API dependencies so the same source runs
 * against the preimage. It writes only external build-output PCM. */
static int unchanged_legacy_suite(void)
{
    const char *dir = getenv("SMC_UI_SHOTS");
    char path[1024];
    CHECK(dir && snprintf(path, sizeof path, "%s/legacy.pcm", dir) < (int)sizeof path);
    FILE *stream = fopen(path, "wb");
    CHECK(stream);
    const unsigned banks[] = {ORIGINAL_SOUND_JUNO, ORIGINAL_SOUND_SYNTHONE,
                              ORIGINAL_SOUND_CHICAGO, ORIGINAL_SOUND_MIAMI};
    for (unsigned b = 0; b < sizeof banks / sizeof banks[0]; ++b) for (unsigned owner = 0; owner < 2; ++owner) {
        setup(V_PART, PART_303A + owner);
        proj.set.bt |= PROJ_BT_CLK_IN;
        CHECK(!engine_bass_select_preset(owner, banks[b], 0, &proj.sound));
        for (unsigned n = 0; n < 1024 && engine_bass_preset_pending(owner); ++n) render(1);
        CHECK(engine_bass_bank(owner) == (int)banks[b] && !engine_bass_preset_pending(owner));
        engine_bass_on(owner, 57, 0, 0);
        int32_t pcm[512], nonzero = 0;
        for (unsigned n = 0; n < 24; ++n) {
            if (n == 16) engine_bass_off(owner);
            engine_render(pcm, 256);
            for (unsigned i = 0; i < sizeof pcm / sizeof pcm[0]; ++i) nonzero |= pcm[i];
            CHECK(fwrite(pcm, sizeof pcm, 1, stream) == 1);
        }
        CHECK(nonzero);
    }
    CHECK(!fclose(stream));
    puts("PASS 49,152 stereo frames across two owners and four legacy sound paths");
    return 0;
}

int main(void)
{
    CHECK(!unchanged_legacy_suite());
    const char *dir = getenv("SMC_UI_SHOTS");
    char path[1024];
    CHECK(dir && snprintf(path, sizeof path, "%s/legacy-active-chorus.pcm", dir) < (int)sizeof path);
    FILE *stream = fopen(path, "wb");
    CHECK(stream);
    for (unsigned owner = 0; owner < 2; ++owner) {
        setup(V_PART, PART_303A + owner);
        proj.set.bt |= PROJ_BT_CLK_IN;
        CHECK(!engine_bass_select_preset(owner, ORIGINAL_SOUND_JUNO, 0, &proj.sound));
        for (unsigned n = 0; n < 1024 && engine_bass_preset_pending(owner); ++n) render(1);
        CHECK(!engine_bass_preset_pending(owner));
        engine_set(T_303, owner, 13, 41);
        engine_set(T_303, owner, 14, 100);
        render(1);
        engine_bass_on(owner, 62, 0, 0);
        for (unsigned n = 0; n < 24; ++n) {
            if (n == 16) engine_bass_off(owner);
            int32_t pcm[512];
            engine_render(pcm, 256);
            CHECK(fwrite(pcm, sizeof pcm, 1, stream) == 1);
        }
    }
    CHECK(!fclose(stream));
    puts("PASS 12,288 stereo frames of active legacy compact chorus across both owners");
    return 0;
}
