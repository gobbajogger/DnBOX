/* SPDX-License-Identifier: GPL-3.0-only */
/* Read-only adapter and isolated render preview; no live input path claimed. */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
#include "app/ui_step_lane.inc"
static int value(int part,int voice,int step,int kind)
{int maximum,conditional;return step_lane_value(part,voice,step,kind,&maximum,&conditional);}
int main(void)
{
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
    printf("PASS read-only step lane adapter and20previews: %u checks; live switching NOT_RUN\n",checks);
    return 0;
}
