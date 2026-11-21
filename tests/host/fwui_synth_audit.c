/* SPDX-License-Identifier: GPL-3.0-only */
/* Report the editor controls actually offered on each synth part and render a
 * held note through the selected production runtime, before the shared mixer.
 * This is a necessary F7 check. Passing it cannot establish instrument identity. */
#define SMC_UI_MAIN retained_smc_main
#include "smc_ui_test.c"
#include "app/original_runtime.h"

static unsigned sounding_frames(unsigned kind)
{
    static original_runtime_part runtime;
    static original_runtime_workspace workspace;
    uint8_t cell[ORIGINAL_SOUND_BYTES];
    float left[256], right[256];
    unsigned sounding = 0;

    CHECK(original_sound_init(cell, kind, 0));
    CHECK(original_runtime_init(&runtime, cell, 120, NULL, NULL) == ORIGINAL_RUNTIME_OK);
    CHECK(original_runtime_key(&runtime, ORIGINAL_INPUT_KEYS, 60, 100) == ORIGINAL_RUNTIME_OK);
    for (int block = 0; block < 32; block++) {
        CHECK(original_runtime_render(&runtime, &workspace, left, right, 256) == ORIGINAL_RUNTIME_OK);
        for (int frame = 0; frame < 256; frame++) {
            CHECK(isfinite(left[frame]) && isfinite(right[frame]));
            sounding += left[frame] != 0 || right[frame] != 0;
        }
    }
    CHECK(original_runtime_retire(&runtime) == ORIGINAL_RUNTIME_OK);
    return sounding;
}

static unsigned editor_controls(unsigned kind, int voice)
{
    int32_t audio[512];
    uint8_t seen[256] = {0};
    unsigned count = 0;
    setup(V_PART, PART_303A + voice);
    CHECK(engine_bass_select_preset(voice, kind, 0, &proj.sound) == 0);
    for (int i = 0; i < 1024 && engine_bass_preset_pending(voice); i++)
        engine_render(audio, 256);
    CHECK(!engine_bass_preset_pending(voice) && (unsigned)engine_bass_bank(voice) == kind);
    build_pages();
    for (int page = 0; page < pg.n; page++) {
        *page_of(V_PART) = page;
        build_pages();
        ui_frame();
        for (int knob = 0; knob < 4; knob++) {
            pref_t ref = pg.r[page][knob];
            if (ref.kind != R_ENG || ref.a != T_303 || ref.b != voice) continue;
            char number[32], unit[16];
            pref_value_of(ref, matrix_value(ref), number, unit);
            printf("CONTROL bank=%u part=%c page=%d K%d id=%u label=[%s] value=[%s%s]\n",
                   kind, 'A' + voice, page, UI_FIRST_KNOB + knob, ref.c, matrix_name(ref), number, unit);
            if (!seen[ref.c]) {
                seen[ref.c] = 1;
                count++;
            }
        }
    }
    return count;
}

int main(void)
{
    const unsigned banks[] = {ORIGINAL_SOUND_SYNTHONE, ORIGINAL_SOUND_JUNO};
    const char *names[] = {"Synth One", "SuperJ6 slot"};
    int missing = 0;
    for (int index = 0; index < 2; index++) {
        unsigned audible = sounding_frames(banks[index]);
        printf("%s runtime: %u/8192 nonzero stereo frames\n", names[index], audible);
        if (!audible) missing = 1;
        for (int voice = 0; voice < NBASS; voice++) {
            unsigned controls = editor_controls(banks[index], voice);
            printf("%s editor %c: %u distinct engine controls\n", names[index], 'A' + voice, controls);
            if (!controls || (banks[index] == ORIGINAL_SOUND_JUNO && controls < 64)) missing = 1;
        }
    }
    return missing;
}
