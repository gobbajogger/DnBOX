/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"

static void check_controls(int owner, const int expected[4])
{
    for (int k = 0; k < 4; ++k) {
        pref_t r = pg.r[cur_page()][k];
        CHECK(r.kind == R_GEN && r.a == owner && r.c == expected[k]);
    }
}

static void capture(const char *kind, int owner, int phase)
{
    char name[64];
    snprintf(name, sizeof name, "%s-%d-%02d", kind, owner, phase);
    memset(cv_tile_valid, 0, sizeof cv_tile_valid);
    memset(blit_hash, 0, sizeof blit_hash);
    ui_frame();
    shot(name);
}

int main(void)
{
    int baseline = getenv("GEN_ROLL_BASELINE") != NULL;
    static const int generate[] = {G_DENS, G_ACC, G_SLIDE, G_OCTS};
    static const int scale[] = {G_ROOT, G_SCALE, G_BASE, G_MUT};
    for (int owner = 0; owner < 2; ++owner) {
        setup(V_SEQ, PART_303A + owner);
        bpart_t *bp = &cur_pat()->bass[owner];
        bp->len = 16;
        for (int step = 0; step < bp->len; ++step)
            bp->step[step] = (bstep_t){48 + step % 13, G_REST, 96};
        bp->step[0].flags = G_NOTE | BS_ACCENT;
        bp->step[1].flags = G_TIE;
        bp->step[4].flags = G_NOTE | BS_SLIDE;
        bp->step[5].flags = G_NOTE;
        bp->step[8].flags = G_NOTE;
        bp->step[12].flags = G_NOTE;
        bp->step[15].flags = G_NOTE | BS_SLIDE;
        bpart_t seeded = *bp;
        capture("seq", owner, 0);
        button_tap(B_ARP);
        CHECK(ui.view == V_GEN && !strcmp(pg.title[cur_page()], "GENERATE"));
        check_controls(owner, generate);
        button_tap(B_PLAY);
        int32_t first_block[512];
        engine_render(first_block, 256);
        CHECK(seq.playing);
        unsigned samples = 256;
        for (int phase = 0; phase < 16; ++phase) {
            while (eng_step[TRK_BASS0 + owner] != phase && samples < 44100u * 10) {
                int32_t audio[512];
                engine_render(audio, 256);
                samples += 256;
                now_ms = 1000 + samples * 1000 / 44100;
            }
            CHECK(eng_step[TRK_BASS0 + owner] == phase);
            printf("EVENT %d %d %u\n", owner, phase, samples);
            capture("gen", owner, phase);
            if (!baseline) {
                int pitch_y = 20 + note_y(49, 46, 62);
                CHECK(fb[pitch_y * 240 + 6] != fb[pitch_y * 240 + 21]);
                CHECK(fb[(pitch_y + 2) * 240 + col_x(phase) + 1] == swap16(C_LINE));
                CHECK(fb[25 * 240 + col_x(phase) - 1] == swap16((phase & 3) ? C_LINE : C_DIM));
            }
        }
        button_tap(B_ARP);
        CHECK(ui.view == V_GEN && !strcmp(pg.title[cur_page()], "SCALE"));
        check_controls(owner, scale);
        capture("scale", owner, 15);
        CHECK(!memcmp(bp, &seeded, sizeof seeded));
    }
    for (int page = 0; page < 2; ++page) {
        setup(V_FX, PART_303A);
        *page_of(V_FX) = page;
        build_pages();
        for (int value = 0; value <= 127; value += value ? 63 : 64) {
            pref_set((pref_t)PR(R_ENG, T_FX, 0, page ? FX_RV_DECAY : FX_DL_FDBK), value);
            capture("fx", page, value);
        }
    }
    printf("PASS Miami GEN (%d checks)\n", checks);
    return 0;
}
