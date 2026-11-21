/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN inherited_smc_ui_main
#include "smc_ui_test.c"

static unsigned clock_marks(pref_t rate)
{
    const uint16_t ink = C_WHITE;
    const int center = 90, row = 31;
    unsigned marks = 0;
    int previous = 0;
    cv_begin(240, 55, C_BLACK);
    cv_band(0, 55);
    CHECK(control_glyph(rate, pref_get(rate), center, ink));
    for (int x = center - 18; x <= center + 18; ++x) {
        int marked = cv_px[row * 240 + x] == swap16(ink);
        if (marked && !previous) ++marks;
        previous = marked;
    }
    return marks;
}

static void check_clock(int part, int setting, int clocks)
{
    int32_t pcm[2];
    int rises = 0, previous_step = -1, previous_sample = 0;
    const int samples_per_step = 44100 * 60 * clocks / (125 * 24);
    setup(V_SEQ, part);
    seq.ext = 0;
    seq.bpm = 125;
    cur_pat()->swing = 0;
    if (part < NKIT) cur_pat()->drum[part].len = 16;
    else {
        cur_pat()->bass[part - NKIT].len = 16;
        cur_pat()->bass[part - NKIT].dir = DIR_FWD;
    }
    build_pages();
    pref_t rate = pg.r[cur_page()][1];
    CHECK(rate.kind == (part < NKIT ? R_DRATE : R_BRATE));
    for (int turns = 0; turns < 30 && pref_get(rate) != setting; ++turns) {
        now_ms += 180;
        enc_acc[EN_K1 + 1] += pref_get(rate) < setting ? 1 : -1;
        input();
    }
    CHECK(pref_get(rate) == setting);
    CHECK(clock_marks(rate) == (unsigned)(24 / clocks));
    seq_start(&seq);
    for (int sample = 0; sample < 120000 && rises < 5; ++sample) {
        engine_render(pcm, 1);
        int step = eng_step[part];
        if (!seq.playing || step == previous_step) continue;
        CHECK(step == rises);
        if (rises) CHECK(abs(sample - previous_sample - samples_per_step) <= 1);
        ++rises;
        previous_step = step;
        previous_sample = sample;
    }
    CHECK(rises == 5);
}

static void master_curve(uint16_t *out)
{
    int32_t pcm[512];
    for (int i = 0; i < 8; ++i) engine_render(pcm, 256);
    cv_begin(240, 55, C_BLACK);
    cv_band(0, 55);
    pref_t macro = PR(R_ENG, T_MST, 0, MST_COMP1);
    CHECK(semantic_amount_glyph(macro, pref_get(macro), 90, C_WHITE));
    for (int y = 27; y <= 49; ++y)
        memcpy(out + (y - 27) * 39, cv_px + y * 240 + 71, 39 * sizeof(*out));
}

static void check_master_curve(void)
{
    uint16_t active[23 * 39], unity[23 * 39], dry[23 * 39];
    setup(V_MIX, PART_909);
    pref_set(PR(R_ENG, T_MST, 0, MST_COMP1), 95);
    pref_set(PR(R_ENG, T_MST, 0, MST_RATIO), 4);
    pref_set(PR(R_ENG, T_MST, 0, MST_MIX), 127);
    master_curve(active);
    pref_set(PR(R_ENG, T_MST, 0, MST_RATIO), 0);
    master_curve(unity);
    pref_set(PR(R_ENG, T_MST, 0, MST_RATIO), 4);
    pref_set(PR(R_ENG, T_MST, 0, MST_MIX), 0);
    master_curve(dry);
    CHECK(memcmp(active, unity, sizeof active));
    CHECK(memcmp(active, dry, sizeof active));
    CHECK(!memcmp(unity, dry, sizeof dry));
}

int main(void)
{
    static const int clocks[] = {6, 4, 3, 8, 12, 24};
    const int parts[] = {PART_909, PART_808, PART_303A, PART_303B};
    for (unsigned part = 0; part < sizeof(parts) / sizeof(parts[0]); ++part)
        for (unsigned setting = 0; setting < sizeof(clocks) / sizeof(clocks[0]); ++setting)
            check_clock(parts[part], setting, clocks[setting]);
    check_master_curve();
    printf("PASS 24 clock-to-PCM and meter cases, active/unity/dry compressor curves (%u checks)\n", checks);
    return 0;
}
