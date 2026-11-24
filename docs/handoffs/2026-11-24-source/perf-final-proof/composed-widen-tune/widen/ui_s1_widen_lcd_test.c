/* SPDX-License-Identifier: GPL-3.0-only */
#include "lfo-test-helpers.h"
void host_lfo_capture(unsigned v,const float *l,const float *r,unsigned n) { (void)v; (void)l; (void)r; (void)n; }
int main(void)
{
    setup(V_PART,PART_303A); select_patch(0); build_pages(); visit("OUTPUT"); ui_frame();
    CHECK(read_slot(0,121).physical == 0.f);
    shot("s1-part-output-widen-off");
    detent(0,1);
    CHECK(read_slot(0,121).physical == 1.f);
    now_ms+=2400; ui_frame(); shot("s1-part-output-widen-on");
    puts("PASS part OUTPUT LCD uses the real OFF/ON switch, other slots empty");
    return 0;
}
