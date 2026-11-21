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
        assert(pref_get(r) == 0);
        char number[32] = {0}, unit[8] = {0};
        assert(original_value_text(r, 0, number, unit) && !strcmp(number,"48000") && !strcmp(unit,"Hz"));
        pref_set(r, 127);
        engine_render(block, 256);
        assert(pref_get(r) == 127);
        assert(original_value_text(r, 127, number, unit) && !strcmp(number,"2048") && !strcmp(unit,"Hz"));
        assert(engine_bass_sound_validate(&proj.sound) == 0);
        assert(pref_get(r) == 127);
        engine_bass_base_restore(1u << v);
        assert(pref_get(r) == 127);
        pref_set(r, 0);
        engine_render(block, 256);
        assert(pref_get(r) == 0);
    }
    puts("Both stock Synth One pages: rate label, encoder edit, persisted-cell restore and return to INIT rate passed.");
    return 0;
}
