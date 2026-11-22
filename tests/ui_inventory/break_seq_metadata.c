/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN retained_fixture_main
#include "../host/smc_ui_test.c"

static uint16_t before[240 * 240];

static void capture(const char *name)
{
    now_ms += 60;
    ui_frame();
    shot(name);
}

static unsigned changed(int top, int bottom)
{
    unsigned pixels = 0;
    for (int i = top * 240; i < bottom * 240; ++i)
        pixels += before[i] != fb[i];
    memcpy(before, fb, sizeof before);
    return pixels;
}

int main(void)
{
    setup(V_HOME, PART_909);
    for (int i = 0; ui.part != PART_BRK && i < NPARTS; ++i) {
        now_ms += 100;
        enc_acc[EN_ALGO] = 1;
        input();
    }
    button_tap(B_SEQ);
    CHECK(ui.part == PART_BRK && ui.view == V_SEQ && ui.overlay == O_NONE && !seq.playing);
    brkpart_t *bp = &cur_pat()->brk;
    bp->steps = 1u << 15;
    bp->slice[15] = 0;
    capture("unassigned");
    memcpy(before, fb, sizeof before);

    bp->slice[15] = 3;
    capture("slice-3");
    unsigned slice_pixels = changed(82, 240);

    int lane = motion_alloc(proj.arr.lane, 0, PART_BRK, T_BRK, 0, 1);
    CHECK(lane >= 0);
    proj.arr.lane[lane].val[15] = 13;
    CHECK(lock_marks(-1, -1, 0) & (1u << 15));
    capture("slice-3-with-p-lock");
    unsigned lock_pixels = changed(82, 240);

    printf("BREAK SEQ step16: assigned slice pixels=%u, added lock pixels=%u, checks=%u\n",
           slice_pixels, lock_pixels, checks);
    CHECK(slice_pixels > 0 && lock_pixels > 0);
#ifndef SHIPPING_026
    CHECK(lock_pixels == 6);
    for (int y = 238; y < 240; ++y)
        for (int x = 218; x < 221; ++x)
            CHECK(fb[y * 240 + x] == swap16(C_AMB));
#endif
    return 0;
}
