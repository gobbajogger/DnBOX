/* SPDX-License-Identifier: GPL-3.0-only */
#define main simulator_main
#include "../../host/x0x_host.c"
#undef main
#include <assert.h>

int main(void)
{
    boot();
    int32_t block[512];
    for (int v = 0; v < 2; ++v) {
        assert(engine_bass_select_preset(v, ORIGINAL_SOUND_SYNTHONE, 0, &proj.sound) == 0);
        for (unsigned i = 0; i < 1000 && engine_bass_preset_pending(v); ++i) engine_render(block, 256);
        assert(engine_bass_bank(v) == ORIGINAL_SOUND_SYNTHONE);
        pref_t r = PR(R_ENG, T_303, v, ORIGINAL_SYNTHONE_CRUSH);
        assert(ui_original_synthone_rows[0][0] == ORIGINAL_SYNTHONE_CRUSH);
        assert(!strcmp(ui_original_name(ORIGINAL_SOUND_SYNTHONE, r.c), "CRUSH RATE"));
        pref_t width = PR(R_ENG, T_303, v, ORIGINAL_SYNTHONE_WIDTH);
        assert(ui_original_synthone_rows[0][1] == ORIGINAL_SYNTHONE_WIDTH);
        assert(!strcmp(ui_original_name(ORIGINAL_SOUND_SYNTHONE, width.c), "STEREO WIDTH"));
        assert(pref_get(r) == 0 && pref_get(width) == 0);
        char number[32] = {0}, unit[8] = {0};
        assert(original_value_text(r, 0, number, unit) && !strcmp(number,"48000") && !strcmp(unit,"Hz"));
        pref_set(width, 67 + v);
        pref_set(r, 127);
        engine_render(block, 256);
        assert(pref_get(r) == 127);
        assert(original_value_text(r, 127, number, unit) && !strcmp(number,"2048") && !strcmp(unit,"Hz"));
        assert(engine_bass_sound_validate(&proj.sound) == 0);
        assert(pref_get(r) == 127);
        engine_bass_base_restore(1u << v);
        assert(pref_get(r) == 127 && pref_get(width) == 67 + v);
        pref_set(r, 0);
        engine_render(block, 256);
        assert(pref_get(r) == 0 && pref_get(width) == 67 + v);
    }
    /* Both owners must survive the ordinary storage API after their selected
     * insert arenas have been reused by different engines. */
    const int rates[2] = {115, 29};
    for (int v = 0; v < 2; ++v)
        pref_set(PR(R_ENG, T_303, v, ORIGINAL_SYNTHONE_CRUSH), rates[v]);
    engine_render(block, 256);
    sound_t saved = proj.sound;
    assert(project_save() == 0);
    for (int v = 0; v < 2; ++v) {
        int bank = v ? ORIGINAL_SOUND_MIAMI : ORIGINAL_SOUND_CHICAGO;
        assert(engine_bass_select_preset(v, bank, 0, &proj.sound) == 0);
        for (unsigned i = 0; i < 1000 && engine_bass_preset_pending(v); ++i) engine_render(block, 256);
        assert(engine_bass_bank(v) == bank && !engine_bass_preset_pending(v));
    }
    assert(project_load() == 0);
    engine_apply_sound(&proj.sound);
    engine_render(block, 256);
    assert(!memcmp(&saved, &proj.sound, sizeof saved));
    for (int v = 0; v < 2; ++v) {
        assert(engine_bass_bank(v) == ORIGINAL_SOUND_SYNTHONE);
        assert(pref_get(PR(R_ENG, T_303, v, ORIGINAL_SYNTHONE_CRUSH)) == rates[v]);
        assert(pref_get(PR(R_ENG, T_303, v, ORIGINAL_SYNTHONE_WIDTH)) == 67 + v);
    }
    puts("Both Synth One pages preserved independent rate and stereo width across project save, engine switches and reload.");
    return 0;
}
