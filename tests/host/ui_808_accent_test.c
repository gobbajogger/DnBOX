/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN inherited_smc_ui_main
#include "smc_ui_test.c"

/* The two peaks occupy the projected 0.0.26 Accent artwork's 44 x 23 cell.
 * Hash native RGB565 words independently of host byte order. */
static uint32_t accent_pixels(void)
{
    uint32_t hash = 2166136261u;
    for (int y = 91; y < 114; ++y)
        for (int x = 96; x < 140; ++x)
            hash = (hash ^ swap16(fb[y * 240 + x])) * 16777619u;
    return hash;
}

int main(void)
{
    const uint32_t idle = 0x22969defu;
    setup(V_PART, PART_808);
    ui.sel[PART_808] = 0;
    build_pages();
    pref_t accent = PR(R_ENG, T_808, NDRUM, 1);
    int page = 0;
    while (page < pg.n && !smc_same(pg.r[page][0], accent)) ++page;
    CHECK(page < pg.n && !strcmp_s(pref_desc(accent)->name, "Accent"));
    *page_of(V_PART) = page;
    build_pages();
    pref_set(accent, 96);
    key_velocity[BLACK_K[0]] = 127;
    CHECK(!seq.playing && !ui.rec && !held_keys);
    load_force = 0;
    audio_due_ms = now_ms;
    ui_due = now_ms + 16;
    run_ms(640);
    CHECK(accent_pixels() == idle);
    run_ms(96);
    CHECK(accent_pixels() == idle);
    held_keys |= 1u << BLACK_K[0];
    run_ms(16);
    CHECK(accent_pixels() == 0x9cec347eu);
    held_keys &= ~(1u << BLACK_K[0]);
    run_ms(48);
    CHECK(accent_pixels() == 0xa2ca61eau);
    run_ms(128);
    CHECK(accent_pixels() == 0xec849ee2u);
    run_ms(6400);
    CHECK(accent_pixels() == idle);
    puts("PASS 808 KIT Accent physical hit, three release-derived outlines, idle and settled");
    return 0;
}
