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

static void check_title(const char *expected)
{
    cv_begin(240, HDR_H, C_BLACK);
    sound_slab(0, 0, 240, HDR_H, 0);
    cv_text_clip(4, 2, &FONT_B, expected, part_col(), 4, 125);
    uint16_t ink = swap16(part_col());
    for (int y = 2; y < 17; ++y) {
        int equal = 1;
        for (int x = 4; x < 125; ++x)
            equal &= (fb[y * 240 + x] == ink) == (cv_px[y * 240 + x] == ink);
        CHECK(equal);
    }
}

static void reach_step(int owner, int target)
{
    for (int block = 0; block < 1400; ++block) {
        int32_t audio[512];
        engine_render(audio, 256);
        if (seq.playing && eng_step[TRK_BASS0 + owner] == target)
            return;
    }
    CHECK(0 && "sequencer never reached the boundary step");
}

static void check_tie_boundary(int owner, int wrap, int enforce)
{
    setup(V_SEQ, PART_303A + owner);
    bpart_t *bp = &cur_pat()->bass[owner];
    bp->len = wrap ? 16 : 32;
    memset(bp->step, 0, sizeof bp->step);
    int next = wrap ? 0 : 16;
    bp->step[15] = (bstep_t){60, G_NOTE, 96};
    bp->step[next] = (bstep_t){0, G_TIE, 96};
    button_tap(B_ARP);
    button_tap(B_PLAY);
    reach_step(owner, 15);
    int y = MAIN_Y + note_y(60, 58, 72) + 2;
    capture("tie-out", owner, wrap);
    if (enforce)
        CHECK(fb[y * 240 + 239] == swap16(dim(part_col(), 9)));
    if (!wrap) {
        button_tap(B_SEQ);
        button_tap(B_OCTUP);
        CHECK(ui.spage == 1);
        button_tap(B_ARP);
        CHECK(ui.view == V_GEN && ui.spage == 1);
    }
    reach_step(owner, next);
    capture("tie-in", owner, wrap);
    uint16_t key_row[18];
    memcpy(key_row, fb + y * 240 + 5, sizeof key_row);
    if (enforce) {
        CHECK(fb[y * 240 + 23] == swap16(dim(part_col(), 9)));
        CHECK(fb[y * 240 + 26] == swap16(dim(part_col(), 9)));
    }
    bp->step[next].flags = G_REST;
    capture("tie-in-control", owner, wrap);
    if (enforce)
        CHECK(!memcmp(key_row, fb + y * 240 + 5, sizeof key_row));
}

static void check_slide_and_range(int owner, int enforce)
{
    setup(V_SEQ, PART_303A + owner);
    bpart_t *bp = &cur_pat()->bass[owner];
    bp->len = 16;
    memset(bp->step, 0, sizeof bp->step);
    bp->step[4] = (bstep_t){60, G_NOTE | BS_SLIDE, 96};
    bp->step[5].note = 127;
    capture("seq-slide-rest", owner, 0);
    button_tap(B_ARP);
    capture("gen-slide-rest", owner, 0);
    memcpy(before_fb, fb, sizeof before_fb);
    bp->step[5].note = 60;
    capture("gen-slide-rest-control", owner, 0);
    if (enforce) {
        int y = MAIN_Y + note_y(60, 58, 72) + 2;
        CHECK(!memcmp(before_fb, fb, sizeof before_fb));
        CHECK(fb[y * 240 + col_x(4) + 14] == swap16(C_WHITE));
    }
    bp->step[5] = (bstep_t){127, G_TIE, 96};
    capture("gen-slide-tie", owner, 0);
    memcpy(before_fb, fb, sizeof before_fb);
    bp->step[5].note = 60;
    capture("gen-slide-tie-control", owner, 0);
    if (enforce)
        CHECK(!memcmp(before_fb, fb, sizeof before_fb));
    bp->step[0] = (bstep_t){0, G_NOTE, 96};
    bp->step[15] = (bstep_t){127, G_NOTE, 96};
    capture("gen-extrema", owner, 0);
    if (enforce) {
        int bottom = MAIN_Y + note_y(0, -2, 129) + 2;
        int top = MAIN_Y + note_y(127, -2, 129) + 2;
        CHECK(fb[bottom * 240 + col_x(0) + 4] == swap16(dim(part_col(), 9)));
        CHECK(fb[top * 240 + col_x(15) + 4] == swap16(dim(part_col(), 9)));
    }
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
        const char *title = owner ? "CHICAGO TB-3PO" : "MIAMI";
        if (!baseline)
            check_title(title);
        button_tap(B_ARP);
        CHECK(ui.view == V_GEN && !strcmp(pg.title[cur_page()], "SCALE"));
        check_controls(owner, scale);
        capture("scale", owner, 15);
        if (!baseline)
            check_title(title);
        CHECK(!memcmp(bp, &seeded, sizeof seeded));
        check_tie_boundary(owner, 0, !baseline);
        check_tie_boundary(owner, 1, !baseline);
        check_slide_and_range(owner, !baseline);
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
