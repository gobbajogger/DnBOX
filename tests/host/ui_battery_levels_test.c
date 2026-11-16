/* SPDX-License-Identifier: GPL-3.0-only */
/* Actual production header, including cache invalidation on battery changes. */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
int main(void)
{
    const int levels[]={3,0,1,2,3,2,1,0,-1,4};
    for(int view=0;view<NVIEWS;view++) {
        if(view==V_GLO) continue; /* global is a list overlay, covered separately */
        setup(view,PART_303A);
        for(unsigned j=0;j<sizeof levels/sizeof levels[0];j++) {
            host_battery_level=levels[j]; now_ms+=50; ui_frame();
            int bars=levels[j]<0?0:levels[j]>3?3:levels[j];
            for(int cell=0;cell<3;cell++)for(int y=7;y<11;y++)for(int x=221+cell*4;x<224+cell*4;x++)
                CHECK(fb[y*240+x]==swap16(cell<bars?SOUND_GRAY:SOUND_LINE));
        }
    }
    /* Cacheable drum headers must also redraw when only battery state changes. */
    setup(V_PART,PART_909);
    for(int level=0;level<4;level++) {
        host_battery_level=level;now_ms+=50;ui_frame();
        for(int cell=0;cell<3;cell++)CHECK(fb[8*240+222+cell*4]==swap16(cell<level?SOUND_GRAY:SOUND_LINE));
    }
    setup(V_HOME,PART_303A);button_tap(B_GLO);
    CHECK(ui.view==V_GLO && ui.overlay==O_LIST);
    for(int level=0;level<4;level++) {
        host_battery_level=level;now_ms+=50;ui_frame();
        for(int cell=0;cell<3;cell++)CHECK(fb[8*240+222+cell*4]==swap16(cell<level?SOUND_GRAY:SOUND_LINE));
    }
    host_battery_level=3;
    printf("PASS four battery levels and header invalidation: %u checks\n",checks);
    return 0;
}
