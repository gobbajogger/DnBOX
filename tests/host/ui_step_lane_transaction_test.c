/* SPDX-License-Identifier: GPL-3.0-only */
/* Real input/editor code with observable substitutes for host no-op CQ guards. */
#include <stdint.h>
static uint32_t regression_cq_lock(void);
static void regression_cq_unlock(uint32_t state);
static void regression_prepare_scope(int part);
static void regression_read_scope(int part);
static void regression_store_scope(int part);
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static unsigned monitor,depth,locks,unlocks,outside_prepare,outside_read;
static int interrupt_lane=-1;
#define LOCK_TOKEN 0xA53C19E7u
static void require_marker(int condition,const char *marker)
{checks++;if(!condition){fprintf(stderr,"FAIL %s\n",marker);exit(1);}}
static uint32_t regression_cq_lock(void)
{
    uint32_t state=plat_cq_lock();
    if(!monitor)return state;
    require_marker(!depth,"BREAK_TRANSACTION_NESTED_LOCK");
    /* Audio records immediately before ALNK0 is masked. The editor must read
     * this updated byte after entering its transaction, not use a stale copy. */
    if(interrupt_lane>=0) {proj.arr.lane[interrupt_lane].val[0]=100;interrupt_lane=-1;}
    depth=1;locks++;return LOCK_TOKEN;
}
static void regression_cq_unlock(uint32_t state)
{
    if(monitor) {
        require_marker(depth==1,"BREAK_TRANSACTION_UNBALANCED_UNLOCK");
        require_marker(state==LOCK_TOKEN,"BREAK_TRANSACTION_RESTORE_TOKEN");
        depth=0;unlocks++;state=0;
    }
    plat_cq_unlock(state);
}
static void regression_prepare_scope(int part)
{if(monitor && part==PART_BRK && !depth)outside_prepare++;}
static void regression_read_scope(int part)
{if(monitor && part==PART_BRK && !depth)outside_read++;}
static void regression_store_scope(int part)
{if(monitor && part==PART_BRK)require_marker(depth==1,"BREAK_STORE_OUTSIDE_TRANSACTION");}
static void reset_observer(void)
{monitor=depth=locks=unlocks=outside_prepare=outside_read=0;interrupt_lane=-1;}
static void fresh(int part,int kind)
{
    monitor=0;setup(V_SEQ,part);project_defaults();engine_motion_reset();build_pages();
    for(int p=0;p<NPARTS;p++)seq.ppat[p]=0;
    ui_step_lane=(uint8_t)kind;
    if(part==PART_BRK)cur_pat()->brk.steps|=1u;
    else sm_set(cur_pat()->drum[part].hit[0],0);
    held_keys=1u<<WHITE_KEY[0];++now_ms;input();CHECK(held_any_step()==0);
    held_btn=1u<<B_SEQ;++now_ms;input();ui.dirty=0;reset_observer();
}
static void turn(int delta,int expected_locks)
{
    monitor=1;enc_acc[EN_ALGO]=delta;++now_ms;input();monitor=0;
    require_marker(!depth && locks==(unsigned)expected_locks && unlocks==locks,"BREAK_TRANSACTION_RELEASE");
}
static void fill_motion(void)
{
    for(int n=0;n<NLANE;n++)CHECK(motion_alloc(proj.arr.lane,n%NPAT,n/NPAT,T_MIX,n/NPAT,MX_LEVEL)>=0);
}
int main(void)
{
    fresh(PART_BRK,UI_LANE_PAN);
    int own=motion_alloc(proj.arr.lane,0,PART_BRK,T_MIX,PART_BRK,MX_PAN);CHECK(own>=0);
    proj.arr.lane[own].val[0]=64;proj.arr.lane[own].val[1]=33;
    interrupt_lane=own;turn(1,1);
    require_marker(proj.arr.lane[own].val[0]==101,"BREAK_TRANSACTION_STALE_READ");
    require_marker(!outside_prepare && !outside_read,"BREAK_READ_OUTSIDE_TRANSACTION");
    CHECK(proj.arr.lane[own].val[1]==33 && ui.dirty);
    /* A second detent uses the committed value, with a fresh balanced guard. */
    ui.dirty=0;reset_observer();turn(1,1);CHECK(proj.arr.lane[own].val[0]==102 && ui.dirty);
    fresh(PART_BRK,UI_LANE_PITCH);fill_motion();static lane_t before[NLANE];memcpy(before,proj.arr.lane,sizeof before);
    turn(1,1);CHECK(!ui.dirty && !memcmp(before,proj.arr.lane,sizeof before));CHECK(strstr(ui.msg,"MOTION FULL"));
    /* Free through the existing removal API, then retry the same held target. */
    engine_motion_clear(0);CHECK(!proj.arr.lane[0].used);reset_observer();turn(1,1);CHECK(ui.dirty);
    fresh(PART_BRK,UI_LANE_PITCH);pref_t pitch=find_ref(T_BRK,0,"Pitch");CHECK(pitch.kind==R_ENG);
    own=motion_alloc(proj.arr.lane,0,PART_BRK,pitch.a,pitch.b,pitch.c);CHECK(own>=0);
    proj.arr.lane[own].val[0]=(uint8_t)pref_desc(pitch)->max;turn(1,1);CHECK(!ui.dirty);
    fresh(PART_BRK,UI_LANE_VELOCITY);turn(1,1);CHECK(!ui.dirty && !motion_count(proj.arr.lane));
    fresh(PART_BRK,UI_LANE_PAN);ui.brk_held=-1;monitor=1;CHECK(!step_lane_turn(1));monitor=0;CHECK(!depth && locks==1 && unlocks==1 && !ui.dirty);
    fresh(PART_BRK,UI_LANE_PAN);monitor=1;CHECK(!step_lane_turn(0));monitor=0;CHECK(!locks && !unlocks && !ui.dirty);
    fresh(PART_BRK,UI_LANE_CHANCE);turn(-1,1);
    own=motion_find(proj.arr.lane,0,PART_BRK,T_PROB,PART_BRK<<4,0);CHECK(own>=0 && proj.arr.lane[own].val[0]==95 && ui.dirty);
    fresh(PART_BRK,UI_LANE_CHANCE);fill_motion();turn(-1,1);CHECK(!ui.dirty && strstr(ui.msg,"MOTION FULL"));
    fresh(PART_909,UI_LANE_VELOCITY);fill_motion();turn(1,0);CHECK(!ui.dirty && strstr(ui.msg,"MOTION FULL"));
    printf("PASS BREAK lane transaction: %u checks; boundary update retained, scopes balanced, chance not nested, failure/no-op clean\n",checks);
    return 0;
}
