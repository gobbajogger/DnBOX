/* SPDX-License-Identifier: GPL-3.0-only */
/* The pan clips at 50 (L50 .. C .. R50): every entry point clamps through x0x_pan_clamp, the
 * pan law at the clip equals the clip, absolute knobs have no dead travel, legacy files load
 * clamped, and the readout and both markers agree. Production UI, SMC mixer and project store through the SMK host fixture. */
#define SMC_UI_MAIN unused_smc_ui_main
#include "smc_ui_test.c"

static int fails;
#undef CHECK
#define CHECK(c) do { if (!(c)) { fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); fails++; } } while (0)

static void readout(pref_t r, const char *want)
{
    char num[16] = {0}, unit[8] = {0};
    pref_value(r, num, unit);
    if (strcmp(num, want)) fprintf(stderr, "readout %s want %s\n", num, want);
    CHECK(!strcmp(num, want));
}

static void helpers(void)
{
    int t, p, prev = -1;
    CHECK(x0x_pan_clamp(0) == 14 && x0x_pan_clamp(13) == 14 && x0x_pan_clamp(14) == 14);
    CHECK(x0x_pan_clamp(64) == 64 && x0x_pan_clamp(114) == 114 && x0x_pan_clamp(115) == 114 && x0x_pan_clamp(127) == 114);
    CHECK(x0x_pan_amount(0) == -50 && x0x_pan_amount(127) == 50 && x0x_pan_amount(64) == 0);
    CHECK(x0x_pan_from_travel(0) == 14 && x0x_pan_from_travel(64) == 64 && x0x_pan_from_travel(127) == 114);
    for (t = 0; t < 128; t++) {                       /* monotone over the whole travel: no dead zone at either end */
        int v = x0x_pan_from_travel(t);
        CHECK(v >= prev && v >= 14 && v <= 114);
        prev = v;
    }
    CHECK(x0x_pan_from_travel(1) > 14 && x0x_pan_from_travel(126) < 114);
    for (p = 14; p <= 114; p++)                       /* pickup inverse round-trips every clamped value */
        CHECK(x0x_pan_from_travel(x0x_pan_to_travel(p)) == p);
    CHECK(x0x_pan_pos(0, 10) == 0 && x0x_pan_pos(14, 10) == 0 && x0x_pan_pos(64, 10) == 5 && x0x_pan_pos(127, 10) == 10);
    CHECK(x0x_pan_pos(0, 36) == 0 && x0x_pan_pos(64, 36) == 18 && x0x_pan_pos(114, 36) == 36 && x0x_pan_pos(127, 36) == 36);
}

static void law(void)
{
    float l0, r0, l1, r1;
    x0x_pan_gains(0, &l0, &r0); x0x_pan_gains(14, &l1, &r1);
    CHECK(l0 == l1 && r0 == r1 && l1 == 1.0f && r1 > 0.0f && r1 < 1.0f);
    x0x_pan_gains(127, &l0, &r0); x0x_pan_gains(114, &l1, &r1);
    CHECK(l0 == l1 && r0 == r1 && r1 == 1.0f);
    x0x_pan_gains(14, &l0, &r0);
    CHECK(l0 == r1 && r0 == l1);                      /* L50 mirrors R50 exactly */
    x0x_pan_gains(64, &l0, &r0);
    CHECK(l0 == 1.0f && r0 == 1.0f);
    x0x_pan_gains(40, &l0, &r0); x0x_pan_gains(88, &l1, &r1);
    CHECK(l0 == r1 && r0 == l1);
}

static pref_t pans[3];

static void entry_points(void)
{
    int i, k;
    pans[0] = PR(R_ENG, T_MIX, PART_909, MX_PAN);
    pans[1] = find_ref(T_909, 1, "Pan")   /* SD */;
    pans[2] = find_ref(T_808, 1, "Pan");
    for (k = 0; k < 3; k++) {
        pref_t r = pans[k];
        CHECK(r.kind == R_ENG && is_pan(r));
        pref_set(r, 0);   CHECK(pref_get(r) == 14); readout(r, "L50");
        pref_set(r, 127); CHECK(pref_get(r) == 114); readout(r, "R50");
        pref_set(r, 13);  CHECK(pref_get(r) == 14);
        pref_set(r, 115); CHECK(pref_get(r) == 114);
        pref_set(r, 100); CHECK(pref_get(r) == 100); readout(r, "R36");
        pref_set(r, 64);  CHECK(pref_get(r) == 64); readout(r, "C");
        for (i = 0; i < 80; i++) turn_ref(r, -4, EN_K1);   /* an encoder stops at the clip, no dead travel */
        CHECK(pref_get(r) == 14); readout(r, "L50");
        ui.enc_t[EN_K1] = 0; ui.enc_run[EN_K1] = 0; now_ms += 1000;
        turn_ref(r, 1, EN_K1); CHECK(pref_get(r) > 14 && pref_get(r) < 20);   /* the first detent back off the clip moves at once */
        for (i = 0; i < 80; i++) turn_ref(r, 4, EN_K1);
        CHECK(pref_get(r) == 114); readout(r, "R50");
        /* a legacy raw value that bypassed every writer still reads as the clip */
        proj.sound.v[r.a][r.b][r.c] = 0;   readout(r, "L50");
        proj.sound.v[r.a][r.b][r.c] = 127; readout(r, "R50");
        pref_set(r, 64);
    }
    /* the SMC knob (relative CC 16..23, bank PAN on HOME): through the real queue, it stops at the clip */
    set_view(V_HOME); smc.view = V_HOME; smc.knob_bank = 1; ui_frame();
    {
        pref_t k = smc_knob_target(1);
        CHECK(k.kind == R_ENG && k.a == T_MIX && k.c == MX_PAN && is_pan(k));
        pref_set(k, 64);
        for (i = 0; i < 100; i++) { message(0xB0, 17, 0x7F - 7); now_ms += 5; smc_poll(); }   /* left, fast */
        CHECK(pref_get(k) == 14); readout(k, "L50");
        CHECK(smc_scaled(k) == 0);                          /* the arc: the full travel at L50 */
        for (i = 0; i < 200; i++) { message(0xB0, 17, 8); now_ms += 5; smc_poll(); }
        CHECK(pref_get(k) == 114); readout(k, "R50");
        CHECK(smc_scaled(k) == 127);
        pref_set(k, 64); CHECK(smc_scaled(k) == 64);
    }
    smc.knob_bank = 0;
}

static void load_roundtrip(void)
{
    pref_t r = pans[1];
    proj.set.bt |= PROJ_BT_CLK_IN;                    /* the SMC fixture's settings, made a valid saved project */
    pref_set(pans[0], 0); pref_set(r, 127);
    CHECK(ui_project_save() == 0);
    proj.sound.v[T_MIX][PART_909][MX_PAN] = 64; proj.sound.v[r.a][r.b][r.c] = 64;
    CHECK(ui_project_load() == 0);
    CHECK(proj.sound.v[T_MIX][PART_909][MX_PAN] == 14 && proj.sound.v[r.a][r.b][r.c] == 114);
    /* a legacy file: raw 0 / 127 stored past the clip load as L50 / R50, the format unchanged */
    proj.sound.v[T_MIX][PART_909][MX_PAN] = 0; proj.sound.v[r.a][r.b][r.c] = 127;
    proj.sound.v[T_MIX][PART_808][MX_PAN] = 1;
    mark_dirty();
    CHECK(ui_project_save() == 0);
    proj.sound.v[T_MIX][PART_909][MX_PAN] = 64; proj.sound.v[r.a][r.b][r.c] = 64;
    CHECK(ui_project_load() == 0);
    CHECK(proj.sound.v[T_MIX][PART_909][MX_PAN] == 14 && proj.sound.v[r.a][r.b][r.c] == 114);
    CHECK(proj.sound.v[T_MIX][PART_808][MX_PAN] == 14);
    readout(pans[0], "L50"); readout(r, "R50");
    CHECK(x0x_pan_pos(pref_get(pans[0]), 10) == 0 && x0x_pan_pos(pref_get(r), 36) == 36);
    pref_set(pans[0], 64); pref_set(r, 64);
}

int main(void)
{
    setup(V_MIX, PART_909);
    helpers();
    law();
    entry_points();
    load_roundtrip();
    if (fails) { fprintf(stderr, "pan-clip: %d failures\n", fails); return 1; }
    puts("pan-clip: OK");
    return 0;
}
