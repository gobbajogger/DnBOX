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
    /* Real held SEL help route; modal body must not remove status battery. */
    setup(V_HOME,PART_303A);held_btn=1u<<B_SEL;input();
    now_ms+=HELP_HOLD+1;input();CHECK(ui.help==B_SEL+1);
    for(int level=0;level<4;level++) {
        host_battery_level=level;now_ms+=50;ui_frame();
        CHECK(ui.help==B_SEL+1);
        for(int cell=0;cell<3;cell++)CHECK(fb[8*240+222+cell*4]==swap16(cell<level?SOUND_GRAY:SOUND_LINE));
    }
    shot("header-help");
    held_btn=0;input();CHECK(!ui.help);
    setup(V_HOME,PART_303A);ask(ACT_CLEAR_PART,PART_303A,"CLEAR PART?","TEST ONLY");
    CHECK(ui.overlay==O_ASK);
    for(int level=0;level<4;level++) {
        host_battery_level=level;now_ms+=50;ui_frame();CHECK(ui.overlay==O_ASK);
        for(int cell=0;cell<3;cell++)CHECK(fb[8*240+222+cell*4]==swap16(cell<level?SOUND_GRAY:SOUND_LINE));
    }
    shot("header-confirmation");
    button_tap(B_HOME);CHECK(ui.overlay==O_NONE);
    /* Hardware pickup producer sends this exact status through say(). */
    setup(V_HOME,PART_303A);say("PICK F1 UP",0);
    for(int level=0;level<4;level++) {
        host_battery_level=level;now_ms+=50;ui_frame();
        CHECK(!strcmp(ui.msg,"PICK F1 UP") && ui.msg_until>now_ms);
        for(int cell=0;cell<3;cell++)CHECK(fb[8*240+222+cell*4]==swap16(cell<level?SOUND_GRAY:SOUND_LINE));
    }
    shot("header-pickup");
    /* Real header source, fixture clip body/clock; not a physical-device test. */
    setup(V_SEQ,PART_303A);clip_grid_mode=1;
    for(int clock=0;clock<2;clock++)for(int level=0;level<4;level++) {
        fixture_clip_external_clock=(uint8_t)clock;host_battery_level=level;now_ms+=50;ui_frame();
        CHECK(clip_grid_mode && seq.clk_src==SEQ_CLK_AUTO);
        for(int cell=0;cell<3;cell++)CHECK(fb[8*240+222+cell*4]==swap16(cell<level?SOUND_GRAY:SOUND_LINE));
        if(level==3) {
            shot(clock?"header-clip-external":"header-clip-internal");
            if(!clock)memcpy(before_fb,fb,sizeof fb);
            else {
                CHECK(changed_rows(0,20,before_fb,fb)>0);
                for(int y=0;y<18;y++)for(int x=0;x<126;x++)CHECK(before_fb[y*240+x]==fb[y*240+x]);
            }
        }
    }
    fixture_clip_external_clock=0;clip_grid_mode=0;
    host_battery_level=3;
    printf("PASS four battery levels and header invalidation: %u checks\n",checks);
    return 0;
}
