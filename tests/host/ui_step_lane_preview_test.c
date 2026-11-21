/* SPDX-License-Identifier: GPL-3.0-only */
/* Actual physical label activation, stored-value adapter and production render. */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static int value(int part,int voice,int step,int kind)
{int maximum,conditional;return step_lane_value(part,voice,step,kind,&maximum,&conditional);}
static void cycle_lane(void)
{
    held_btn=1u<<B_SEQ;++now_ms;input();
    held_btn|=1u<<B_SEL;++now_ms;input();
    held_btn&=~(1u<<B_SEL);++now_ms;input();
    held_btn=0;++now_ms;input();ui_frame();
}
static void live_lane_scenarios(void)
{
    for(int part=0;part<NPARTS;part++) {
        setup(V_SEQ,part);ui_step_lane=0;++now_ms;ui_frame();
        int page=cur_page();
        project_t *saved=malloc(sizeof proj);CHECK(saved);memcpy(saved,&proj,sizeof proj);
        for(int kind=1;kind<=UI_LANE_COUNT;kind++) {
            memcpy(before_fb,fb,sizeof fb);cycle_lane();
            CHECK(ui_step_lane==kind%UI_LANE_COUNT);
            CHECK(ui.view==V_SEQ && ui.overlay==O_NONE && cur_page()==page);
            CHECK(!memcmp(saved,&proj,sizeof proj));
            CHECK(changed_rows(227,240,before_fb,fb)>0);
            CHECK(!changed_rows(20,227,before_fb,fb)); /* roll and four knobs retained */
            char name[64];snprintf(name,sizeof name,"live-part%d-lane%d",part,ui_step_lane);shot(name);
        }
        /* Persistent pattern windows: the lane follows the visible absolute steps. */
        if(part!=PART_BRK)for(int window=0;window<4;window++) {
            ui.spage=(uint8_t)window;ui_step_lane=UI_LANE_PITCH;
            if(part_is_303(part)) {
                cur_pat()->bass[part-PART_303A].len=64;
                cur_pat()->bass[part-PART_303A].step[window*16].note=(uint8_t)(40+window*10);
            } else cur_pat()->drum[part].len=64;
            ++now_ms;ui_frame();CHECK(step_of(0)==window*16);
            char name[64];snprintf(name,sizeof name,"window-part%d-page%d",part,window);shot(name);
        }
        free(saved);
        button_tap(B_SEL);CHECK(ui.overlay==O_LIST); /* ordinary SELECT still opens list */
    }
}
int main(void)
{
    live_lane_scenarios();
    /* The lane cannot steal plain page navigation, lists, or HELP. */
    setup(V_SEQ,PART_909);ui_step_lane=0;
    int page=cur_page();button_tap(B_SEQ);CHECK(cur_page()==(page+1)%pg.n);CHECK(!ui_step_lane);
    set_view(V_FX);build_pages();page=cur_page();cycle_lane();
    CHECK(!ui_step_lane); /* chord is contextual: ordinary FX/list roles remain */
    setup(V_SEQ,PART_909);ui_step_lane=0;
    held_btn=1u<<B_SEQ;++now_ms;input();now_ms+=HELP_HOLD+1;input();
    CHECK(ui.help==B_SEQ+1);
    held_btn|=1u<<B_SEL;++now_ms;input();held_btn&=~(1u<<B_SEL);++now_ms;input();
    CHECK(!ui_step_lane);held_btn=0;++now_ms;input();CHECK(!ui.help);
    setup(V_SEQ,PART_909);
    CHECK(value(PART_909,0,0,UI_LANE_VELOCITY)==seq.accent_q7);
    sm_set(cur_pat()->drum[PART_909].accent,0);
    CHECK(value(PART_909,0,0,UI_LANE_VELOCITY)==127);
    drum_vel_set_step(PART_909,0,0,33);
    CHECK(value(PART_909,0,0,UI_LANE_VELOCITY)==33);
    CHECK(value(PART_909,1,0,UI_LANE_VELOCITY)==127);
    int k=motion_alloc(proj.arr.lane,seq.ppat[PART_909],PART_909,T_PROB,0,0);CHECK(k>=0);
    proj.arr.lane[k].val[4]=25;proj.arr.lane[k].val[5]=PROB_COND;
    CHECK(value(PART_909,0,4,UI_LANE_CHANCE)==25);
    CHECK(value(PART_909,0,6,UI_LANE_CHANCE)==100);
    int maximum,conditional;
    CHECK(step_lane_value(PART_909,0,5,UI_LANE_CHANCE,&maximum,&conditional)==PROB_COND);
    CHECK(conditional && maximum==100);
    pat_of(PART_303A)->bass[0].step[32].note=71;
    pat_of(PART_303A)->bass[0].step[32].velocity=93;
    CHECK(value(PART_303A,0,32,UI_LANE_PITCH)==71);
    CHECK(value(PART_303A,0,32,UI_LANE_VELOCITY)==93);
    CHECK(value(PART_BRK,0,0,UI_LANE_VELOCITY)==-1); /* no invented volume mapping */
    CHECK(value(-1,0,0,UI_LANE_PAN)==-1);
    CHECK(value(PART_909,NDRUM,0,UI_LANE_PAN)==-1);
    project_t *before=malloc(sizeof proj);CHECK(before);memcpy(before,&proj,sizeof proj);
    for(int part=0;part<NPARTS;part++)for(int kind=0;kind<UI_LANE_COUNT;kind++)for(int step=0;step<NSTEPS;step++) {
        (void)value(part,0,step,kind);CHECK(!memcmp(before,&proj,sizeof proj));
    }
    free(before);
    for(int part=0;part<NPARTS;part++)for(int kind=0;kind<UI_LANE_COUNT;kind++) {
        setup(V_SEQ,part);ui_step_lane=(uint8_t)kind;
        blit_hash[3]=0;draw_knobs();draw_step_lane();cv_commit_knobs(3);
        char name[64];snprintf(name,sizeof name,"preview-part%d-lane%d",part,kind);shot(name);
    }
    printf("PASS physical SEQ+SELECT lane cycle, stored adapter and production/preview renders: %u checks; literal touchscreen NOT_RUN\n",checks);
    return 0;
}
