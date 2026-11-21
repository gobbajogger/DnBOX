/* SPDX-License-Identifier: GPL-3.0-only */
#define main simulator_main
#include "../../host/x0x_host.c"
#undef main
#include <assert.h>

int main(int argc, char **argv)
{
    assert(argc == 5);
    int kit = atoi(argv[1]), mode = atoi(argv[2]), restore = atoi(argv[3]);
    assert(kit == PART_808 || kit == PART_909);
    boot();
    int32_t out[512];
    pref_t dist = PR(R_ENG, T_808, D8_KIT, D8_KIT_DIST);
    pref_t drive = PR(R_ENG, T_808, D8_KIT, D8_KIT_DRIVE);
    pref_t glue = PR(R_ENG, T_808, D8_KIT, D8_KIT_GLUE);
    assert(!pref_get(dist) && !pref_get(drive) && !pref_get(glue));
    pref_t shared = PR(R_ENG, T_FX, 0, FX_DIST);
    int old_global = pref_get(shared);
    int shaper = mode <= 7 ? mode : 0;
    for (int i = 1; i <= 7; ++i) {
        pref_set(dist, i);
        assert(pref_get(dist) == i);
    }
    pref_set(dist, shaper);
    pref_set(drive, shaper ? 89 : 0);
    pref_set(glue, mode == 8 ? 108 : 0);
    assert(pref_get(shared) == old_global);
    if (restore) {
        sound_t saved = proj.sound;
        pref_set(dist, 0);
        pref_set(drive, 0);
        pref_set(glue, 0);
        for (int i = 0; i < 16; ++i)
            engine_render(out, 256);
        if (restore == 1) {
            proj.sound = saved;
        } else {
            proj.sound.v[T_808][D8_KIT][D8_KIT_DIST] = (uint8_t)shaper;
            proj.sound.v[T_808][D8_KIT][D8_KIT_DRIVE] = shaper ? 89 : 0;
            proj.sound.v[T_808][D8_KIT][D8_KIT_GLUE] = mode == 8 ? 108 : 0;
            assert(!memcmp(&proj.sound, &saved, sizeof saved));
        }
        /* Equal active CRUSH history is required for sample-identical output. */
        engine_apply_sound(&proj.sound);
    }
    for (int i = 0; i < 16; ++i)
        engine_render(out, 256);
    assert(pref_get(dist) == shaper && pref_get(drive) == (shaper ? 89 : 0));
    FILE *file = fopen(argv[4], "wb");
    assert(file);
    int audible = 0;
    for (int i = 0; i < 60; ++i) {
        if (i == 0 || i == 35) engine_drum(kit, kit == PART_808 ? D8_SD : DR_SD, 0.95f);
        engine_render(out, 256);
        for (int j = 0; j < 512; ++j) audible |= out[j] != 0;
        assert(fwrite(out, sizeof out, 1, file) == 1);
    }
    assert(audible && !fclose(file));
    return 0;
}
