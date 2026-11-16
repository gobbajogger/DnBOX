/* SPDX-License-Identifier: GPL-3.0-only */
/* Pan reads L50 .. C .. R50 as a DISPLAY mapping (x0x_pan_disp / x0x_pan_raw) over the unchanged raw
 * 0..127: nothing clamps the stored pot, hard left/right stay reachable, encoders step in display
 * units, a legacy project round-trips byte-identical, and the readout and both markers agree. Production UI, SMC mixer and project store through the SMK host fixture. */
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
    int raw, d, prev = -51;
    for (raw = 0; raw < 128; raw++) {                 /* every raw maps into +-50, monotone */
        d = x0x_pan_disp(raw);
        CHECK(d >= -50 && d <= 50 && d >= prev);
        prev = d;
    }
    CHECK(x0x_pan_disp(0) == -50 && x0x_pan_disp(1) == -50 && x0x_pan_disp(64) == 0 && x0x_pan_disp(127) == 50);
    CHECK(x0x_pan_disp(63) == -1 && x0x_pan_disp(65) == 1 && x0x_pan_disp(126) == 49 && x0x_pan_disp(2) == -49);
    for (d = -50; d <= 50; d++)                       /* the inverse round-trips every display step */
        CHECK(x0x_pan_disp(x0x_pan_raw(d)) == d);
    CHECK(x0x_pan_raw(-50) == 0 && x0x_pan_raw(50) == 127 && x0x_pan_raw(0) == 64);   /* hard L / R reachable */
    CHECK(x0x_pan_raw(-51) == 0 && x0x_pan_raw(51) == 127);
    CHECK(x0x_div_half_away(5, 2) == 3 && x0x_div_half_away(-5, 2) == -3 && x0x_div_half_away(-4, 3) == -1);
    CHECK(x0x_pan_pos(0, 10) == 0 && x0x_pan_pos(64, 10) == 5 && x0x_pan_pos(127, 10) == 10);
    CHECK(x0x_pan_pos(0, 36) == 0 && x0x_pan_pos(64, 36) == 18 && x0x_pan_pos(127, 36) == 36);
}

static void law_unchanged(void)
{
    float l, r;
    x0x_pan_gains(0, &l, &r);   CHECK(l == 1.0f && r == 0.0f);      /* hard left: nothing on the right */
    x0x_pan_gains(127, &l, &r); CHECK(l == 0.0f && r == 1.0f);
    x0x_pan_gains(64, &l, &r);  CHECK(l == 1.0f && r == 1.0f);
}

static pref_t pans[3];

static void turn_to(pref_t r, int e, int n)
{
    int i;
    for (i = 0; i < n; i++) { ui.enc_t[EN_K1] = 0; ui.enc_run[EN_K1] = 0; now_ms += 1000; turn_ref(r, e, EN_K1); }
}

static void entry_points(void)
{
    int i, k;
    pans[0] = PR(R_ENG, T_MIX, PART_909, MX_PAN);
    pans[1] = find_ref(T_909, 1, "Pan");   /* SD */
    pans[2] = find_ref(T_808, 1, "Pan");
    for (k = 0; k < 3; k++) {
        pref_t r = pans[k];
        CHECK(r.kind == R_ENG && is_pan(r));
        pref_set(r, 0);   CHECK(pref_get(r) == 0);   readout(r, "L50");   /* stored as written: no clamp */
        pref_set(r, 1);   CHECK(pref_get(r) == 1);   readout(r, "L50");
        pref_set(r, 127); CHECK(pref_get(r) == 127); readout(r, "R50");
        pref_set(r, 100); CHECK(pref_get(r) == 100); readout(r, "R29");
        pref_set(r, 64);  CHECK(pref_get(r) == 64);  readout(r, "C");
        turn_to(r, 1, 1);  CHECK(pref_get(r) == 65); readout(r, "R1");   /* one detent = one display step */
        turn_to(r, 1, 1);  CHECK(pref_get(r) == x0x_pan_raw(2)); readout(r, "R2");
        turn_to(r, -1, 2); CHECK(pref_get(r) == 64); readout(r, "C");
        turn_to(r, -1, 60); CHECK(pref_get(r) == 0); readout(r, "L50");  /* hard left reachable */
        turn_to(r, 1, 1);  CHECK(pref_get(r) == x0x_pan_raw(-49)); readout(r, "L49");
        turn_to(r, 1, 120); CHECK(pref_get(r) == 127); readout(r, "R50"); /* hard right reachable */
        pref_set(r, 64);
    }
    /* the SMC knob (relative CC 17, bank PAN on HOME) goes through the same display steps */
    set_view(V_HOME); smc.view = V_HOME; smc.knob_bank = 1; ui_frame();
    {
        pref_t kr = smc_knob_target(1);
        CHECK(kr.kind == R_ENG && kr.a == T_MIX && kr.c == MX_PAN && is_pan(kr));
        pref_set(kr, 64);
        message(0xB0, 17, 1); CHECK(pref_get(kr) == 65); readout(kr, "R1");
        for (i = 0; i < 200; i++) { message(0xB0, 17, 0x40 | 7); now_ms += 5; }
        CHECK(pref_get(kr) == 0); readout(kr, "L50");
        for (i = 0; i < 200; i++) { message(0xB0, 17, 7); now_ms += 5; }
        CHECK(pref_get(kr) == 127); readout(kr, "R50");
        pref_set(kr, 64);
    }
    smc.knob_bank = 0;
}

static void load_roundtrip(void)
{
    static uint8_t first[PLAT_STORE_MAX];
    int n;
    pref_t r = pans[1];
    proj.set.bt |= PROJ_BT_CLK_IN;                    /* the SMC fixture's settings, made a valid saved project */
    /* a legacy project: raw 0 / 1 / 127 / 37 pans load exactly as stored and save back byte-identical */
    pref_set(pans[0], 0); pref_set(r, 127); pref_set(pans[2], 37);
    proj.sound.v[T_MIX][PART_808][MX_PAN] = 1;
    mark_dirty();
    CHECK(ui_project_save() == 0);
    n = store_len[OBJ_SOUND];
    CHECK(n > 0 && n <= PLAT_STORE_MAX);
    memcpy(first, store[OBJ_SOUND], (size_t)n);
    proj.sound.v[T_MIX][PART_909][MX_PAN] = 64; proj.sound.v[r.a][r.b][r.c] = 64;
    CHECK(ui_project_load() == 0);
    CHECK(proj.sound.v[T_MIX][PART_909][MX_PAN] == 0 && proj.sound.v[r.a][r.b][r.c] == 127);
    CHECK(proj.sound.v[T_MIX][PART_808][MX_PAN] == 1 && proj.sound.v[pans[2].a][pans[2].b][pans[2].c] == 37);
    readout(pans[0], "L50"); readout(r, "R50"); readout(pans[2], "L21");
    CHECK(x0x_pan_pos(pref_get(pans[0]), 10) == 0 && x0x_pan_pos(pref_get(r), 36) == 36);
    mark_dirty();
    CHECK(ui_project_save() == 0);
    CHECK(store_len[OBJ_SOUND] == n && !memcmp(first, store[OBJ_SOUND], (size_t)n));   /* byte-identical */
    pref_set(pans[0], 64); pref_set(r, 64); pref_set(pans[2], 64);
}

int main(void)
{
    setup(V_MIX, PART_909);
    helpers();
    law_unchanged();
    entry_points();
    load_roundtrip();
    if (fails) { fprintf(stderr, "pan-display: %d failures\n", fails); return 1; }
    puts("pan-display: OK (raw 0..127 unclamped, L50..C..R50 display, hard L/R reachable, legacy round-trip byte-identical)");
    return 0;
}
