/* SPDX-License-Identifier: GPL-3.0-only */
/* Demonstrate why the existing void UI setter alone cannot promise DNR2
 * QUEUED acceptance. This is a diagnostic, not a new endpoint or a fix. */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
int main(void)
{
    setup(V_HOME,PART_303A);
    pref_t ref=PR(R_ENG,T_MST,0,MST_MAKEUP);
    const x0x_param_t *d=pref_desc(ref); CHECK(d && d->max>0);
    pref_set_impl(ref,0,0);run_ms(20);
    CHECK(pref_get(ref)==0);CHECK(engine_master_get(MST_MAKEUP)==0);
    /* More than the real queue's capacity, with no audio consumer. */
    for(unsigned i=0;i<2048;i++)engine_set(T_MIX,0,MX_LEVEL,i&127u);
    pref_set_impl(ref,d->max,0);
    CHECK(pref_get(ref)==d->max);
    run_ms(500);
    CHECK(engine_queue_available_locked()==1024);
    CHECK(engine_master_get(MST_MAKEUP)==0);
    printf("CONFIRMED: full queue admits UI mirror=%u but audio value=%d; checked protocol transaction required (%u checks)\n",d->max,engine_master_get(MST_MAKEUP),checks);
    return 0;
}
