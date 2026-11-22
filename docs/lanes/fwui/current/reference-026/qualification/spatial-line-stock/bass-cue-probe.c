/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN retained_fixture
#include "smc_ui_test.c"

int main(void)
{
    unsigned cases = 0;
    static const int views[] = {V_SEQ, V_GEN};
    for (int part = PART_303A; part <= PART_303B; part++) {
        for (unsigned i = 0; i < sizeof views / sizeof views[0]; i++) {
            int view = views[i];
            for (unsigned state = 0; state < 8; state++) {
                setup(view, part);
                bpart_t *bp = &cur_pat()->bass[bidx()];
                memset(bp->step, 0, sizeof bp->step);
                bp->len = 32;
                bp->step[0].note = 48;
                bp->step[0].flags = G_NOTE;
                bp->step[14].note = 48;
                bp->step[14].flags = G_NOTE | ((state & 4) ? BS_SLIDE : 0);
                bp->step[15].note = 52;
                bp->step[15].flags = G_NOTE | ((state & 1) ? BS_ACCENT : 0) |
                    ((state & 2) ? BS_SLIDE : 0);
                bp->step[16].note = 55;
                bp->step[16].flags = G_NOTE;
                if (view == V_SEQ) {
                    held_keys = 1u << WHITE_KEY[15];
                    input();
                    CHECK(ui.held_step == 15);
                } else {
                    held_keys = 1u << WHITE_KEY[15];
                    input();
                    CHECK(ui.held_step == -1);
                }
                ui_frame();
                char name[48];
                snprintf(name, sizeof name, "bass-%c-%s-%u", part == PART_303A ? 'a' : 'b',
                         view == V_SEQ ? "seq-held" : "gen", state);
                shot(name);
                cases++;
            }
        }
    }
    CHECK(cases == 32);
    printf("BASS CUE hardware-free route cases=%u checks=%u PASS\n", cases, checks);
}
