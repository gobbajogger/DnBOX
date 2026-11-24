/* SPDX-License-Identifier: GPL-3.0-only */
#include "lfo-test-helpers.h"
#include "../../firmware/src/app/s1c_part.h"
void host_lfo_capture(unsigned v,const float *l,const float *r,unsigned n) { (void)v; (void)l; (void)r; (void)n; }
void host_s1_retrigger(const s1c *s) { (void)s; }
int main(void)
{
    setup(V_PART,PART_303A); select_patch(0); build_pages(); visit("VOICE"); ui_frame();
    const unsigned position=read_slot(0,120).position;
    shot("s1-voice-legato-factory");
    detent(0,position ? -1 : 1);
    CHECK(read_slot(0,120).position == !position);
    now_ms+=2400; ui_frame(); shot("s1-voice-legato-edited");
    visit("OUTPUT"); ui_frame();
    CHECK(read_slot(0,32).physical==.5f);
    shot("s1-output-level-factory");
    detent(1,-127);
    CHECK(read_slot(0,32).physical==0);
    now_ms+=2400; ui_frame(); shot("s1-output-level-zero");
    puts("PASS actual voice and output LCD for mono Legato and independent part Level");
    return 0;
}
