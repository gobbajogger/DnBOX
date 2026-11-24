/* SPDX-License-Identifier: GPL-3.0-only */
#include "lfo-test-helpers.h"

void host_lfo_capture(unsigned v,const float *l,const float *r,unsigned n) { (void)v; (void)l; (void)r; (void)n; }
int main(void)
{
    setup(V_PART, PART_303A);
    select_patch(0);
    build_pages();
    visit("EFFECT");
    ui_frame();
    CHECK(read_slot(0,39).physical == 700.f);
    shot("s1-reverb-hp-init");
    detent(3, -127);
    CHECK(read_slot(0,39).physical == 80.f);
    now_ms += 2400;
    ui_frame();
    shot("s1-reverb-hp-80");
    puts("PASS actual Reverb HP EFFECT LCD at 700 and 80 Hz");
    return 0;
}
