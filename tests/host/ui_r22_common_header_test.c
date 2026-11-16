/* SPDX-License-Identifier: GPL-3.0-only */
/* Real normal-view headers must share the R22 pattern/clock/battery material. */
#define SMC_UI_MAIN unused_common_header_smc_main
#include "smc_ui_test.c"
static void right_header(uint16_t *out)
{
    ui.msg_until=0;ui.overlay=O_NONE;ui.help=0;
    /* Equal clock/pattern state, independent of setup/navigation side effects. */
    seq.song_on=0;seq.playing=0;seq.ppat[PART_909]=0;seq.pcue[PART_909]=255;
    draw_header();
    /* A cached header may correctly leave cv_px holding another strip.
     * Compare the actual committed LCD frame, never stale scratch pixels. */
    for(int y=0;y<HDR_H;y++)for(int x=125;x<240;x++)out[y*115+x-125]=fb[y*240+x];
}
int main(void)
{
    uint16_t reference[115*HDR_H],actual[115*HDR_H];
    setup(V_PART,PART_909);right_header(reference);
    const int views[]={V_HOME,V_FX,V_MIX,V_SEQ,V_SONG};
    for(unsigned i=0;i<sizeof views/sizeof views[0];i++) {
        setup(views[i],PART_909);right_header(actual);
        for(unsigned p=0;p<sizeof actual/sizeof actual[0];p++) {
            if(actual[p]!=reference[p])fprintf(stderr,"HEADER_DIFF view%d x%u y%u actual%x expected%x\n",views[i],125+p%115,p/115,actual[p],reference[p]);
            CHECK(actual[p]==reference[p]);
        }
    }
    /* Stopped PART cache must still observe real pattern and transport edits. */
    setup(V_PART,PART_909);right_header(reference);
    seq.ppat[PART_909]=1;draw_header();
    for(int y=0;y<HDR_H;y++)for(int x=125;x<240;x++)actual[y*115+x-125]=fb[y*240+x];
    CHECK(memcmp(actual,reference,sizeof actual)!=0);
    memcpy(reference,actual,sizeof actual);seq.playing=1;draw_header();
    for(int y=0;y<HDR_H;y++)for(int x=125;x<240;x++)actual[y*115+x-125]=fb[y*240+x];
    CHECK(memcmp(actual,reference,sizeof actual)!=0);
    puts("Common R22 normal-view header region/cache PASS");return 0;
}
