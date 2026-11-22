#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"

static void choose_preset(unsigned owner, unsigned preset)
{
    int32_t audio[512];
    CHECK(engine_bass_select_preset(owner, ORIGINAL_SOUND_JUNO, preset, &proj.sound) == 0);
    for (unsigned i = 0; i < 2048 && engine_bass_preset_pending(owner); ++i)
        engine_render(audio, 256);
    CHECK(!engine_bass_preset_pending(owner));
    CHECK(engine_bass_preset_index(owner) == (int)preset);
}

static void edit(unsigned owner, unsigned id, unsigned value)
{
    CHECK(original_sound_set_extended(proj.sound.v[T_303][owner],
          proj.sound.v[T_303][NBASS + owner], id, value));
    engine_bass_sound_refresh(&proj.sound, owner);
}

static uint32_t glyph_hash(pref_t control, int value)
{
    cv_begin(40, 32, C_BLACK);
    cv_band(0, 32);
    orig_strip_glyph(control, value, 4, 4, C_WHITE);
    uint32_t hash = 2166136261u;
    unsigned ink = 0;
    for (unsigned i = 0; i < 40 * 32; ++i) {
        hash = (hash ^ cv_px[i]) * 16777619u;
        ink += cv_px[i] != swap16(C_BLACK);
    }
    CHECK(ink > 12);
    return hash;
}

int main(void)
{
    const int values[] = {0, 42, 43, 84, 85, 126, 127};
    const unsigned wave_ids[] = {0, 0, 1, 1, 2, 2, 3};
    const char *const names[] = {"SINE", "TRI", "SAW", "PULSE"};
    uint32_t seen_legacy[NBASS][4];

    for (unsigned owner = 0; owner < NBASS; ++owner) {
        setup(V_PART, PART_303A + owner);
        pref_t wave = PR(R_ENG, T_303, owner, 0);
        pref_t width = PR(R_ENG, T_303, owner, 12);
        char num[32], unit[32];
        choose_preset(owner, 0);
        pref_value_of(wave, 127, num, unit);
        CHECK(!strcmp(num, "100") && !strcmp(unit, "%"));
        pref_value_of(width, 127, num, unit);
        CHECK(!strcmp(num, "90") && !strcmp(unit, "%"));
        edit(owner, 12, 127);
        edit(owner, 0, 127);
        seen_legacy[owner][0] = glyph_hash(wave, 127);
        seen_legacy[owner][1] = glyph_hash(width, 127);
        seen_legacy[owner][2] = glyph_hash(wave, 0);
        seen_legacy[owner][3] = glyph_hash(width, 64);
        printf("LEGACY owner=%u wave=%08x/%08x width=%08x/%08x\n", owner,
               seen_legacy[owner][0], seen_legacy[owner][2], seen_legacy[owner][1], seen_legacy[owner][3]);
    }

    for (unsigned owner = 0; owner < NBASS; ++owner) {
        setup(V_PART, PART_303A + owner);
        pref_t wave = PR(R_ENG, T_303, owner, 0);
        pref_t width = PR(R_ENG, T_303, owner, 12);
        char num[32], unit[32];
        uint32_t shapes[4] = {0};
        choose_preset(owner, ORIGINAL_JUNO_WIDE_PRESET);
        edit(owner, 12, 127);
        pref_value_of(wave, 0, num, unit);
        printf("WIDE owner=%u raw wave=0 displays %s%s\n", owner, num, unit);
        fflush(stdout);
        for (unsigned i = 0; i < sizeof values / sizeof values[0]; ++i) {
            edit(owner, 0, values[i]);
            pref_value_of(wave, values[i], num, unit);
            CHECK(!strcmp(num, names[wave_ids[i]]) && !*unit);
            uint32_t current = glyph_hash(wave, values[i]);
            if (shapes[wave_ids[i]]) CHECK(shapes[wave_ids[i]] == current);
            shapes[wave_ids[i]] = current;
        }
        for (unsigned i = 0; i < 4; ++i)
            for (unsigned j = i + 1; j < 4; ++j)
                CHECK(shapes[i] != shapes[j]);
        pref_value_of(width, 0, num, unit);
        CHECK(!strcmp(num, "10") && !strcmp(unit, "%"));
        pref_value_of(width, 127, num, unit);
        CHECK(!strcmp(num, "50") && !strcmp(unit, "%"));
        CHECK(glyph_hash(width, 64) == glyph_hash(width, 127));
        CHECK(glyph_hash(width, 127) != glyph_hash(width, 0));
        printf("WIDE owner=%u glyphs=%08x/%08x/%08x/%08x width split=10%%/50%%\n",
               owner, shapes[0], shapes[1], shapes[2], shapes[3]);
    }
    printf("PASS WIDE production formatter and glyph pixels, both owners (%u checks)\n", checks);
}
