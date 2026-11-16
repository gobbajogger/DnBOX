/* SPDX-License-Identifier: GPL-3.0-only */
/* Real kick panel fields must format the same engineering values used by DSP. */
#define SMC_UI_MAIN unused_kick_units_smc_main
#include "smc_ui_test.c"
#include "dsp/drum909.h"
int main(void)
{
    setup(V_PART,PART_909);
    drum909_t *state=malloc(sizeof *state);CHECK(state);drum909_init(state);
    for(int raw=0;raw<128;raw++) {
        float physical=0; char n[16],u[8],en[16],eu[8];
        drum909_set(state,DR_BD,0,raw);
        CHECK(drum909_param_si(DR_BD,0,raw,&physical)==X0X_U_MS);
        CHECK(!memcmp(&physical,&state->bt[0].tune,sizeof physical));
        pref_value_of(PR(R_ENG,T_909,DR_BD,0),raw,n,u);
        unit_value(X0X_U_MS,state->bt[0].tune,en,eu);
        CHECK(!strcmp(n,en));CHECK(!strcmp(u,eu));
        drum909_set(state,DR_BD,5,raw);
        CHECK(drum909_param_si(DR_BD,5,raw,&physical)==X0X_U_RATIO);
        CHECK(!memcmp(&physical,&state->bt[0].pitch_mod,sizeof physical));
        pref_value_of(PR(R_ENG,T_909,DR_BD,5),raw,n,u);
        unit_value(X0X_U_RATIO,state->bt[0].pitch_mod,en,eu);
        CHECK(!strcmp(n,en));CHECK(!strcmp(u,eu));
        CHECK(drum909_param_si(DR_BD,1,raw,&physical)==X0X_U_PCT);
    }
    free(state);
    puts("909 kick actual Tune ms / Pitch ratio-to-semitone readouts PASS, all 128 raw values");
    return 0;
}
