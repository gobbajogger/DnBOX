/* SPDX-License-Identifier: GPL-3.0-only */
/* Development negative fixture. Nonzero exit means the actual missing-battery
 * defect, not passing acceptance. Production source remains unchanged. */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static unsigned missing;
static void capture_toast(const char *id)
{
    char original[64];memcpy(original,ui.msg,sizeof original);
    host_battery_level=3;now_ms+=25;ui_frame();
    CHECK(ui.msg_until>now_ms);CHECK(!strcmp(original,ui.msg));shot(id);
    unsigned wrong=0;
    for(int cell=0;cell<3;cell++)for(int y=7;y<11;y++)for(int x=221+cell*4;x<224+cell*4;x++)
        wrong+=fb[y*240+x]!=swap16(SOUND_GRAY);
    missing+=wrong>0;
    printf("TOAST %s chars=%zu boldWidth=%d smallWidth=%d batteryWrongPixels=%u text=%s\n",
        id,strlen(ui.msg),tw(&FONT_B,ui.msg),tw(&FONT_XS,ui.msg),wrong,ui.msg);
}
int main(void)
{
    setup(V_HOME,PART_909);say("SAVED",0);capture_toast("toast-saved");
    setup(V_HOME,PART_909);say("NEXT: PAT 16",0);capture_toast("toast-next-pattern");
    setup(V_HOME,PART_909);run_action(ACT_ABOUT,0);capture_toast("toast-about-host-identity");
    /* Explicit stress input, not the current build identity. */
    setup(V_HOME,PART_909);say("DnBOX 99.99.99_openbeta  MEM 100%  CPU 100% g123456789abc",0);
    capture_toast("toast-about-synthetic-long");
    printf("TOAST BATTERY %s: %u of 4 states missing battery\n",missing?"FAIL":"PASS",missing);
    return missing?1:0;
}
