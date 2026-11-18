/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
int main(void)
{
    for(int view=0;view<NVIEWS;view++) {
        if(view==V_GLO)continue;
        setup(view,PART_909);any_button=0;
        const char *a,*b,*c;footer_text(&a,&b,&c);
        CHECK(!a || !strstr(a,"HOLD ANY BUTTON"));
        CHECK(!b || !strstr(b,"HOLD ANY BUTTON"));
        CHECK(!c || !strstr(c,"HOLD ANY BUTTON"));
    }
    puts("PASS no startup explanatory footer; help/input behavior separately tested");return 0;
}
