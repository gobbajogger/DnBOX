/* SPDX-License-Identifier: GPL-3.0-only */
#define main simulator_main
#include "../../host/x0x_host.c"
#undef main
#include <assert.h>

static int setting(int mode, int i)
{
    const int normal[6]={0,7,68,60,93,55}, edited[6]={0,2,0,0,42,127};
    return i ? (mode == i+1 ? edited[i] : normal[i]) : mode != 0;
}

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
    pref_t knob[6] = {0};
    unsigned found = 0;
    for (int p = 0; p < pg.n; ++p) {
        for (int i = 0; i < 4; ++i) {
            pref_t r = pg.r[p][i];
            if (r.kind != R_ENG || r.a != T_808 || r.b != D8_KIT ||
                r.c < D8_KIT_DL_BUS || r.c > D8_KIT_DL_HPF) continue;
            assert(!strcmp(pg.title[p], r.c <= D8_KIT_DL_TIME ? "DL 430ms" : "KIT DL"));
            int k = r.c - D8_KIT_DL_BUS;
            knob[k] = r;
            found |= 1u << k;
        }
    }
    assert(found == 63 && !pref_get(knob[0]));
    pref_t shared = PR(R_ENG, T_FX, 0, FX_DL_LEVEL);
    pref_set(shared, 0);
    int send = -1;
    for (int i = 0; i < drum808_nparams(D8_SD); ++i)
        if (!strcmp(drum808_param(D8_SD,i)->name,"Dly")) send=i;
    assert(send >= 0);
    pref_set(PR(R_ENG, T_808, D8_SD, send), 127);
    for (int i=0;i<6;++i) pref_set(knob[i],setting(mode,i));
    assert(!pref_get(shared));
    int panel_values[6];
    for (int i = 0; i < 6; ++i) panel_values[i] = pref_get(knob[i]);
    ui.part = PART_909;
    build_pages();
    for (int page = 0; page < pg.n; ++page)
        for (int i = 0; i < 4; ++i)
            assert(!(pg.r[page][i].kind == R_ENG && pg.r[page][i].a == T_808));
    ui.part = PART_808;
    build_pages();
    for (int i = 0; i < 6; ++i) assert(pref_get(knob[i]) == panel_values[i]);
    for (int i = 0; i < 16; ++i) engine_render(out, 256);
    sound_t saved = proj.sound;
    if (reload)
        for (int i = 0; i < 6; ++i) pref_set(knob[i],0);
    for (int i = 0; i < 16; ++i) engine_render(out, 256);
    if (reload == 1) {
        proj.sound = saved;
        if (!mode) memset(&proj.sound.v[T_808][D8_KIT][D8_KIT_DL_BUS],0,6);
        engine_apply_sound(&proj.sound);
    } else if (reload == 2) {
        for (int i=0;i<6;++i)
            proj.sound.v[T_808][D8_KIT][D8_KIT_DL_BUS+i]=(uint8_t)setting(mode,i);
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
