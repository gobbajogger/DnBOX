/* SPDX-License-Identifier: GPL-3.0-only */
/* Exercise the production formatter, not a mirrored formatter implementation. */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static void check_pan(pref_t r)
{
    char number[32],unit[32];
    CHECK(is_pan(r));
    pref_value_of(r,0,number,unit); CHECK(!strcmp(number,"L50")); CHECK(!*unit);
    pref_value_of(r,64,number,unit); CHECK(!strcmp(number,"C")); CHECK(!*unit);
    pref_value_of(r,127,number,unit); CHECK(!strcmp(number,"R50")); CHECK(!*unit);
    int last=50;
    for(int v=0;v<64;v++) {
        pref_value_of(r,v,number,unit);
        CHECK(number[0]=='L'); int n=atoi(number+1);
        CHECK(n>=1 && n<=last); last=n; CHECK(!*unit);
    }
    last=0;
    for(int v=65;v<=127;v++) {
        pref_value_of(r,v,number,unit);
        CHECK(number[0]=='R'); int n=atoi(number+1);
        CHECK(n>=last && n<=50); last=n; CHECK(!*unit);
    }
}
int main(void)
{
    setup(V_MIX,PART_909);
    check_pan((pref_t){R_ENG,T_MIX,0,MX_PAN});
    unsigned drum_pans=0;
    for(int engine=T_909;engine<=T_808;engine++)
        for(int voice=0;voice<11;voice++)
            for(int parameter=0;parameter<32;parameter++) {
                pref_t r={R_ENG,engine,voice,parameter};
                if(is_pan(r)) { check_pan(r); drum_pans++; }
            }
    CHECK(drum_pans>0);
    printf("PASS production pan L50/C/R50: %u checks\n",checks);
    return 0;
}
