/* SPDX-License-Identifier: GPL-3.0-only */
/* Actual app edit paths must emit external invalidation, while a checked
 * remote scalar transaction leaves its one revision to the protocol core. */
static unsigned source_events;
void dnbox_source_changed(void) { source_events++; }
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
int main(void)
{
    setup(V_HOME,PART_303A);
    source_events=0;
    pref_set(PR(R_ENG,T_MST,0,MST_MAKEUP),12);
    CHECK(pref_get(PR(R_ENG,T_MST,0,MST_MAKEUP))==12);
    CHECK(source_events>0);
    unsigned first=source_events;
    pref_set(PR(R_ENG,T_MST,0,MST_MAKEUP),13);
    pref_set(PR(R_ENG,T_MST,0,MST_MAKEUP),12);
    CHECK(source_events>first); /* external A->B->A cannot disappear */
    source_events=0;
    CHECK(dnbox_remote_scalar_set(T_MST,0,MST_MAKEUP,-1,14)==2);
    CHECK(pref_get(PR(R_ENG,T_MST,0,MST_MAKEUP))==14);
    CHECK(source_events==0); /* core will increment once, no global suppression */
    engine_set(T_MST,0,MST_MAKEUP,15);
    CHECK(source_events>0); /* direct external producer remains independent */
    run_ms(100);source_events=0;
    int next_bank=1-engine_bass_bank(0);
    CHECK(engine_bass_select_preset(0,next_bank,0,&proj.sound)==0);
    CHECK(engine_bass_preset_pending(0));CHECK(source_events>0);
    unsigned pending_events=source_events;run_ms(100);
    CHECK(!engine_bass_preset_pending(0));CHECK(engine_bass_bank(0)==next_bank);
    CHECK(source_events>pending_events);
    unsigned settled_events=source_events;run_ms(100);
    CHECK(source_events==settled_events); /* no revision flood on idle boundaries */
    source_events=0;
    CHECK(engine_bass_select_preset(0,1-next_bank,0,&proj.sound)==0);
    unsigned request_events=source_events;
    engine_bass_cancel_preset(0);
    CHECK(source_events>request_events); /* cancellation changes pending availability now */
    run_ms(100);CHECK(engine_bass_bank(0)==next_bank);
    unsigned cancelled_events=source_events;run_ms(100);
    CHECK(source_events==cancelled_events);
    source_events=0;CHECK(ui_project_begin()==0);CHECK(ui_project_busy());
    CHECK(source_events>0);unsigned begin_events=source_events;
    ui_project_end(0);CHECK(!ui_project_busy());CHECK(source_events>begin_events);
    printf("PASS actual source-origin events: %u checks; not full mutation coverage\n",checks);
    return 0;
}
