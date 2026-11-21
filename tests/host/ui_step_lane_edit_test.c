/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static int lane_value(int part,int step,int kind)
{int max,cond;return step_lane_value(part,part<NKIT?ui.sel[part]:0,step,kind,&max,&cond);}
static void fresh(int part)
{
    setup(V_SEQ,part);project_defaults();engine_motion_reset();build_pages();
    for(int p=0;p<NPARTS;p++)seq.ppat[p]=0;
    ui.touched=-1;ui.msg_until=0;++now_ms;ui_frame();
}
static void hold_step(int part,int step)
{
    ui.spage=(uint8_t)(step/16);
    if(part<NKIT)sm_set(cur_pat()->drum[part].hit[ui.sel[part]],step);
    else if(part_is_303(part)) {
        cur_pat()->bass[bidx()].step[step].flags=G_NOTE;
        cur_pat()->bass[bidx()].step[step].note=60;
    } else cur_pat()->brk.steps|=1u<<step;
    held_keys=1u<<WHITE_KEY[step%16];++now_ms;input();
    CHECK(held_any_step()==step);
}
static void edit_once(int e)
{
    held_btn=1u<<B_SEQ;++now_ms;input();enc_acc[EN_ALGO]=e;++now_ms;input();ui_frame();
    held_btn=0;++now_ms;input();
}
static void release_step(void)
{held_keys=0;++now_ms;input();ui_frame();}
static void full_motion_table(void)
{
    const int parts[]={PART_909,PART_303A,PART_BRK,PART_BRK};
    const int kinds[]={UI_LANE_VELOCITY,UI_LANE_PAN,UI_LANE_PITCH,UI_LANE_VELOCITY};
    static lane_t before[NLANE];
    for(int n=0;n<4;n++) {
        int part=parts[n];fresh(part);ui_step_lane=(uint8_t)kinds[n];hold_step(part,0);
        for(int k=0;k<NLANE;k++) {
            int owner=k/NPAT,index=motion_alloc(proj.arr.lane,k%NPAT,owner,T_MIX,owner,MX_LEVEL);
            CHECK(index>=0);proj.arr.lane[index].val[1]=50;
        }
        int value=lane_value(part,0,kinds[n]);memcpy(before,proj.arr.lane,sizeof before);
        edit_once(n==3?-1:1);CHECK(lane_value(part,0,kinds[n])==value);
        CHECK(!memcmp(before,proj.arr.lane,sizeof before));CHECK(motion_count(proj.arr.lane)==NLANE);
        CHECK(strstr(ui.msg,"MOTION FULL"));release_step();CHECK(step_lane_has_note(part,0,0));
    }
    /* An existing lane can still change when every slot is occupied. */
    fresh(PART_303A);ui_step_lane=UI_LANE_PAN;hold_step(PART_303A,0);
    int own=motion_alloc(proj.arr.lane,0,PART_303A,T_MIX,PART_303A,MX_PAN);CHECK(own>=0);
    proj.arr.lane[own].val[0]=64;proj.arr.lane[own].val[1]=33;
    for(int k=0;k<NLANE-1;k++)CHECK(motion_alloc(proj.arr.lane,k%NPAT,k/NPAT,T_MIX,k/NPAT,MX_LEVEL)>=0);
    edit_once(1);CHECK(proj.arr.lane[own].val[0]==65 && proj.arr.lane[own].val[1]==33);
    CHECK(motion_count(proj.arr.lane)==NLANE);release_step();
    proj.set.bt|=PROJ_BT_CLK_IN;CHECK(project_validate_for(&proj)==0);
    CHECK(project_save()==0);CHECK(project_load()==0);
    CHECK(lane_value(PART_303A,0,UI_LANE_PAN)==65);
}
static void legacy_and_normal_controls(void)
{
    fresh(PART_909);ui_step_lane=UI_LANE_VELOCITY;hold_step(PART_909,0);
    CHECK(lane_value(PART_909,0,UI_LANE_VELOCITY)==seq.accent_q7);
    int vel=motion_alloc(proj.arr.lane,0,PART_909,T_VEL,0,0);CHECK(vel>=0);
    CHECK(proj.arr.lane[vel].val[0]==MOT_NONE);
    CHECK(lane_value(PART_909,0,UI_LANE_VELOCITY)==seq.accent_q7);
    proj.arr.lane[vel].val[0]=0; /* actual sequencer zero means legacy accent law */
    CHECK(lane_value(PART_909,0,UI_LANE_VELOCITY)==seq.accent_q7);
    edit_once(1);CHECK(proj.arr.lane[vel].val[0]==seq.accent_q7+1);
    CHECK(proj.arr.lane[vel].val[1]==MOT_NONE);release_step();
    fresh(PART_303A);ui_step_lane=UI_LANE_VELOCITY;hold_step(PART_303A,0);
    cur_pat()->bass[0].step[0].velocity=0;
    CHECK(lane_value(PART_303A,0,UI_LANE_VELOCITY)==BASS_VELOCITY_DEFAULT);
    edit_once(1);CHECK(cur_pat()->bass[0].step[0].velocity==BASS_VELOCITY_DEFAULT+1);
    /* Existing unmodified physical K3 velocity, K2 chance and K5 pitch still work. */
    enc_acc[EN_ALGO]=1;++now_ms;input();CHECK(cur_pat()->bass[0].step[0].velocity==BASS_VELOCITY_DEFAULT+2);
    enc_acc[EN_SELECT]=-1;++now_ms;input();CHECK(lane_value(PART_303A,0,UI_LANE_CHANCE)==95);
    ui_step_lane=UI_LANE_PAN;int pan=lane_value(PART_303A,0,UI_LANE_PAN);
    enc_acc[EN_K1]=1;++now_ms;input();CHECK(cur_pat()->bass[0].step[0].note==61);
    CHECK(lane_value(PART_303A,0,UI_LANE_PAN)==pan);release_step();
}
int main(void)
{
    full_motion_table();legacy_and_normal_controls();
    unsigned scenarios=0;
    for(int part=0;part<NPARTS;part++)for(int kind=0;kind<UI_LANE_COUNT;kind++) {
        fresh(part);ui_step_lane=(uint8_t)kind;hold_step(part,0);
        ++now_ms;ui_frame();int before=lane_value(part,0,kind),page=cur_page();
        CHECK(before>=0);
        memcpy(before_fb,fb,sizeof fb);
        held_btn=1u<<B_SEQ;++now_ms;input();
        for(int turn=0;turn<(kind==UI_LANE_CHANCE?8:1);turn++) {
            enc_acc[EN_ALGO]=kind==UI_LANE_CHANCE?-1:part==PART_BRK && kind==UI_LANE_VELOCITY?-64:24;++now_ms;input();
        }
        ui_frame();
        int after=lane_value(part,0,kind);
        CHECK(after!=before);CHECK(changed_rows(227,240,before_fb,fb)>0);
        CHECK(ui.part==part && ui.view==V_SEQ && ui.overlay==O_NONE && cur_page()==page);
        held_btn=0;++now_ms;input();held_keys=0;++now_ms;input();ui_frame();
        CHECK(lane_value(part,0,kind)==after);CHECK(cur_page()==page);
        CHECK(step_lane_has_note(part,part<NKIT?ui.sel[part]:0,0));
        /* Persistence is the real host project writer/reader, not a byte copy. */
        proj.set.bt|=PROJ_BT_CLK_IN; /* SMC fixture forces central-only BT; storage requires clock-in bit. */
        CHECK(project_validate_for(&proj)==0);CHECK(project_save()==0);CHECK(project_load()==0);
        CHECK(lane_value(part,0,kind)==after);
        char name[64];snprintf(name,sizeof name,"edited-part%d-kind%d",part,kind);shot(name);
        printf("EDIT part=%d kind=%d before=%d after=%d persisted=1\n",part,kind,before,after);
        scenarios++;
    }
    printf("PASS physical visible-lane edits: %u scenarios, %u checks; all persisted\n",scenarios,checks);
    return 0;
}
