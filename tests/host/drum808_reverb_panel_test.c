/* SPDX-License-Identifier: GPL-3.0-only */
#define main simulator_main
#include "../../host/x0x_host.c"
#undef main
#include <assert.h>

int main(int argc, char **argv)
{
    assert(argc == 5);
    int kit = atoi(argv[1]), mode = atoi(argv[2]), reload = atoi(argv[3]);
    boot();
    int32_t out[512];
    for (int i = 0; i < 16; ++i) engine_render(out, 256);
    ui.part = PART_808;
    ui.view = V_PART;
    build_pages();
    pref_t knob[5] = {0};
    unsigned found = 0;
    for (int p = 0; p < pg.n; ++p) {
        for (int i = 0; i < 4; ++i) {
            pref_t r = pg.r[p][i];
            if (r.kind != R_ENG || r.a != T_808 || r.b != D8_KIT ||
                r.c < D8_KIT_RV_BUS || r.c > D8_KIT_RV_LEVEL) continue;
            assert(!strcmp(pg.title[p], r.c == D8_KIT_RV_BUS ? "RV BUS" : "KIT RV"));
            int k = r.c - D8_KIT_RV_BUS;
            knob[k] = r;
            found |= 1u << k;
        }
    }
    assert(found == 31 && !pref_get(knob[0]));
    pref_t shared = PR(R_ENG, T_FX, 0, FX_RV_LEVEL);
    pref_set(shared, 0);
    int send = -1;
    for (int i = 0; i < drum808_nparams(D8_SD); ++i)
        if (!strcmp(drum808_param(D8_SD,i)->name,"Rev")) send=i;
    assert(send >= 0);
    pref_set(PR(R_ENG, T_808, D8_SD, send), 127);
    static const int baseline[5] = {0,70,60,55,93};
    for (int i = 1; i < 5; ++i)
        pref_set(knob[i], mode == i+1 ? (i == 2 ? 0 : i == 4 ? 31 : 127) : baseline[i]);
    pref_set(knob[0], mode != 0);
    assert(!pref_get(shared));
    for (int i = 0; i < 16; ++i) engine_render(out, 256);
    sound_t saved = proj.sound;
    if (reload)
        for (int i = 0; i < 5; ++i) pref_set(knob[i],0);
    for (int i = 0; i < 16; ++i) engine_render(out, 256);
    if (reload == 1) {
        proj.sound = saved;
        if (!mode) memset(&proj.sound.v[T_808][D8_KIT][D8_KIT_RV_BUS],0,5);
        engine_apply_sound(&proj.sound);
    } else if (reload == 2) {
        proj.sound.v[T_808][D8_KIT][D8_KIT_RV_BUS] = (uint8_t)(mode != 0);
        for (int i = 1; i < 5; ++i)
            proj.sound.v[T_808][D8_KIT][D8_KIT_RV_BUS+i] = (uint8_t)(mode == i+1 ? (i == 2 ? 0 : i == 4 ? 31 : 127) : baseline[i]);
        assert(!memcmp(&proj.sound,&saved,sizeof saved));
        engine_apply_sound(&proj.sound);
    }
    for (int i = 0; i < 32; ++i) engine_render(out, 256);
    assert(pref_get(knob[0]) == (mode != 0) && !pref_get(shared));
    FILE *file = fopen(argv[4],"wb");
    assert(file);
    engine_drum(kit,kit == PART_808 ? D8_SD : DR_SD,0.95f);
    int audible = 0;
    for (int i = 0; i < 260; ++i) {
        engine_render(out,256);
        for (int j=0;j<512;++j) audible |= out[j]!=0;
        assert(fwrite(out,sizeof out,1,file)==1);
    }
    assert(audible && !fclose(file));
    return 0;
}
