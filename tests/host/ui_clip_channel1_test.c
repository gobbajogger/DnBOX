/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"

int main(void)
{
    setup(V_HOME, PART_303A);
    open_global();
    global_section(0);
    int row = -1;
    for (int i = 0; i < ui.list_n; ++i)
        if (list_rows[i].kind == R_CKCH) row = i;
    CHECK(row >= 0);
    list_move(row);
    CHECK(ui.list_sel == row && proj.set.ck_ch == 2);
    pref_t r = list_rows[row];
    int band = (row - ui.list_top) * 23 / BAND_H;
    int y = (row - ui.list_top) * 23 - band * BAND_H;

    cv_begin(240, BAND_H, C_BLACK);
    cv_band(0, BAND_H);
    draw_list(band);
    CHECK(cv_px[(y + 6) * 240 + 200] != swap16(C_AMB));

    turn_ref(r, -1, EN_ALGO);
    CHECK(proj.set.ck_ch == 1 && ui.overlay == O_LIST);
    char n[36], u[8];
    pref_value(r, n, u);
    CHECK(!strcmp(n, "1"));
    CHECK(strstr(ui.msg, "LOOPY PRO") && strstr(ui.msg, "LAUNCH CLIPS"));
    CHECK(ui.msg_until > now_ms);
    ui_frame();
    shot("clip-keys-channel-1-notification");
    now_ms = ui.msg_until + 1;
    ui_frame();
    shot("clip-keys-channel-1-glyph");

    cv_begin(240, BAND_H, C_BLACK);
    cv_band(0, BAND_H);
    draw_list(band);
    CHECK(cv_px[(y + 6) * 240 + 200] == swap16(C_AMB));  /* apex */
    CHECK(cv_px[(y + 9) * 240 + 200] == swap16(C_AMB));  /* exclamation stem */
    CHECK(cv_px[(y + 13) * 240 + 200] == swap16(C_AMB)); /* exclamation dot */
    CHECK(cv_px[(y + 12) * 240 + 200] != swap16(C_AMB)); /* gap */
    pref_set(r, -100);
    CHECK(proj.set.ck_ch == 1);
    pref_set(r, 100);
    CHECK(proj.set.ck_ch == 15); /* channel 16 stays excluded */
    CHECK(!ui.msg_until);
    puts("PASS Clip Keys channel 1: edit, number, triangle, notice, range and unchanged channel 2 default");
    return 0;
}
