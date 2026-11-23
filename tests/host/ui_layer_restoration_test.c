/* SPDX-License-Identifier: GPL-3.0-only */
/* A transient readout owns its opaque rectangle, then restores the base frame. */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static void layer_shot(const char *name)
{
    shot(name);
    printf("LAYER_STATE {\"id\":\"%s\",\"view\":%u,\"part\":%u,\"page\":%d,\"overlay\":%u,\"touched\":%d,\"nowMs\":%u,\"touchUntil\":%u}\n",name,ui.view,ui.part,cur_page(),ui.overlay,ui.touched,plat_ms(),ui.touch_until);
}
int main(void)
{
    setup(V_HOME,PART_909);uint16_t base[240*240];memcpy(base,fb,sizeof base);layer_shot("tempo-layer-base");
    ui.touched=4;ui.touch_until=plat_ms()+1000;ui_frame();layer_shot("tempo-layer-overlay");
    unsigned inside=0,outside=0;
    for(int y=MAIN_Y;y<KNOB_Y;y++)for(int x=0;x<240;x++)if(fb[y*240+x]!=base[y*240+x]) {
        if(x>=8&&x<232&&y>=MAIN_Y+BAND_H+14&&y<MAIN_Y+BAND_H+64)inside++;
        else outside++;
    }
    CHECK(inside>100);CHECK(!outside);
    now_ms+=1500;ui_frame();layer_shot("tempo-layer-restored");
    if(getenv("LAYER_MUTATE_RESTORE"))fb[(MAIN_Y+10)*240+10]^=65535;
    unsigned damaged=0;for(int y=MAIN_Y;y<KNOB_Y;y++)for(int x=0;x<240;x++)damaged+=fb[y*240+x]!=base[y*240+x];
    if(damaged)fprintf(stderr,"LAYER_RESTORE_MISSING pixels=%u\n",damaged);
    CHECK(!damaged);
    printf("PASS actual tempo overlay bounds and full base-main restoration (%u overlay pixels, %u checks)\n",inside,checks);return 0;
}
