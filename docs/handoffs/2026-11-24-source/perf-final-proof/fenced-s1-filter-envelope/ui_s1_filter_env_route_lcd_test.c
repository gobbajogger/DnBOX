/* SPDX-License-Identifier: GPL-3.0-only */
#include "lfo-test-helpers.h"
void host_lfo_capture(unsigned o,const float*l,const float*r,unsigned n){(void)o;(void)l;(void)r;(void)n;}
int main(void)
{
    static const char *const shots[]={"filter-env-route-off","filter-env-route-lfo1","filter-env-route-lfo2","filter-env-route-both"};
    setup(V_PART,PART_303A); select_patch(0); build_pages(); visit("FILTER MODE");
    CHECK(!read_slot(0,56).position);
    for(unsigned i=0;i<4;i++) {
        if(i) detent(3,1);
        CHECK(read_slot(0,56).position==i);
        now_ms+=2400;ui_frame();shot(shots[i]);
    }
    puts("PASS actual FILTER MODE LCD for OFF, LFO 1, LFO 2, BOTH on physical K4");
}
