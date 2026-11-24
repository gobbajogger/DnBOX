/* SPDX-License-Identifier: GPL-3.0-only */
#include "lfo-test-helpers.h"

int main(void)
{
    setup(V_PART, PART_303A);
    select_patch(0);
    build_pages();
    visit("EFFECT");
    ui_frame();
    shot("s1-effect-init");
    detent(1, 127);
    detent(2, 127);
    CHECK(read_slot(0, 44).position == 127);
    CHECK(read_slot(0, 40).position == 127);
    now_ms += 2400;
    ui_frame();
    shot("s1-effect-100");
    puts("PASS real Synth One EFFECT renderer on INIT and after both physical MIX turns");
    return 0;
}
