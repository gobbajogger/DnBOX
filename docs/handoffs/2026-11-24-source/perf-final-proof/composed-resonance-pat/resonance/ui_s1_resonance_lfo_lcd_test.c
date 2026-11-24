/* SPDX-License-Identifier: GPL-3.0-only */
#include "lfo-test-helpers.h"
void host_lfo_capture(unsigned o,const float*l,const float*r,unsigned n){(void)o;(void)l;(void)r;(void)n;}
int main(void)
{
    static const char *const shots[]={"res-route-off","res-route-lfo1","res-route-lfo2","res-route-both"};
    setup(V_PART,PART_303A); select_patch(0); build_pages(); visit("MOD ROUTE");
    CHECK(!read_slot(0,49).position);
    for(unsigned i=0;i<4;i++) {
        if(i) detent(1,1);
        CHECK(read_slot(0,49).position==i);
        now_ms+=2400;ui_frame();shot(shots[i]);
    }
    puts("PASS actual MOD ROUTE LCD for OFF, LFO 1, LFO 2, BOTH on physical K2");
}
