/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN retained_fixture
#include "smc_ui_test.c"

static uint16_t unassigned[240 * 240], assigned[240 * 240];

static void complete_frame(void)
{
    ui.msg_until = 0;
    now_ms += 60;
    ui_frame();
}

static unsigned diff_at(int x, int y, int width, int height, const uint16_t *prior)
{
    unsigned changed = 0;
    for (int j = y; j < y + height; ++j)
        for (int i = x; i < x + width; ++i)
            changed += fb[j * 240 + i] != prior[j * 240 + i];
    return changed;
}

int main(void)
{
    for (int theme = 0; theme < 2; ++theme) {
        setup(V_SEQ, PART_BRK);
        pref_set(PR(R_PALETTE, 0, 0, 0), theme ? NPALETTES - 1 : 0);
        pref_set(PR(R_BRKSLICE, 0, 0, 0), 3);
        audio_block();
        CHECK(engine_brk_count(0) == 32);
        brkpart_t *bp = &cur_pat()->brk;
        bp->steps = 0xffff;
        int lane = motion_alloc(proj.arr.lane, 0, PART_BRK, T_BRK, 0, 1);
        CHECK(lane >= 0);
        for (int page = 0; page < 3; ++page) {
            ui.page[V_SEQ][PART_BRK] = page;
            build_pages();
            CHECK(cur_page() == page);
            for (int mode = 0; mode < 4; ++mode) {
                ui_step_lane = mode;
                memset(bp->slice, 0, sizeof bp->slice);
                memset(proj.arr.lane[lane].val, MOT_NONE, sizeof proj.arr.lane[lane].val);
                complete_frame();
                memcpy(unassigned, fb, sizeof unassigned);

                for (int step = 0; step < 16; ++step)
                    bp->slice[step] = 17 + step;
                complete_frame();
                CHECK(diff_at(0, 0, 240, 219, unassigned) == 0);
                printf("theme=%d page=%d lane=%d assignment changed rows:", theme, page, mode);
                for (int y = 211; y < 240; ++y) {
                    unsigned pixels = diff_at(0, y, 240, 1, unassigned);
                    if (pixels) printf(" %d=%u", y, pixels);
                }
                putchar('\n');
                fflush(stdout);
                CHECK(diff_at(0, 229, 240, 11, unassigned) == 0);
                memcpy(assigned, fb, sizeof assigned);
                cv_begin(240, KNOB_H, C_BLACK);
                for (int step = 0; step < 16; ++step) {
                    char number[4];
                    int center = 69 + step * 10;
                    put_i(number, 17 + step);
                    CHECK(tw(&FONT_T, number) <= 10);
                    text_c(center, vc(&FONT_T, 219 - KNOB_Y, 9), &FONT_T, number, C_HI);
                    CHECK(diff_at(center - 5, 219, 10, 8, unassigned) > 0);
                }
                for (int y = 219; y < 229; ++y)
                    for (int x = 0; x < 240; ++x)
                        if (cv_px[(y - KNOB_Y) * 240 + x] == swap16(C_HI))
                            CHECK(fb[y * 240 + x] == swap16(C_HI));

                for (int step = 0; step < 16; ++step)
                    proj.arr.lane[lane].val[step] = 13;
                CHECK((lock_marks(-1, -1, 0) & 0xffffu) == 0xffffu);
                complete_frame();
                CHECK(diff_at(0, 0, 240, 238, assigned) == 0);
                CHECK(diff_at(0, 238, 240, 2, assigned) == 96);
                for (int step = 0; step < 16; ++step)
                    for (int y = 238; y < 240; ++y)
                        for (int x = 68 + step * 10; x < 71 + step * 10; ++x)
                            CHECK(fb[y * 240 + x] == swap16(C_AMB));
                char name[48];
                snprintf(name, sizeof name, "break-metadata-theme-%d-page-%d-mode-%d", theme, page, mode);
                shot(name);
            }
        }
    }
    printf("Verified 2 themes, 3 pages, 4 lanes, 16 two-digit assignments and locks per scenario; %u checks\n", checks);
    return 0;
}
