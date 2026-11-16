/* SPDX-License-Identifier: GPL-3.0-only */
/* A caption crossing the first 69-row strip must survive in the final LCD frame. */
#define SMC_UI_MAIN unused_caption_band_smc_main
#include "smc_ui_test.c"
int main(void)
{
    setup(V_PART,PART_909);ui.sel[PART_909]=0;build_pages();*page_of(V_PART)=1;
    ui.touched=-1;ui.msg_until=0;ui.dirty=1;
    for(int j=0;j<4;j++)blit_hash[j]=0;
    ui_frame();
    uint16_t actual[240*240];memcpy(actual,fb,sizeof actual);
    /* Rasterize the expected font over the real active-row face independently
     * of draw_drum_voice's strip clipping and label visibility guard. */
    cv_begin(240,138,RGB(36,48,54));
    /* R22 compact preceding row is42px; the caption baseline is82, not
     * the retired48px-row baseline88. First exact22 failure is retained. */
    cv_text(11,82,&FONT_T,"PITCH",SOUND_GRAY);
    unsigned ink=0;
    for(int y=82;y<94;y++)for(int x=11;x<45;x++)
        if(cv_px[y*240+x]!=swap16(RGB(36,48,54))) {
            ink++;CHECK(actual[y*240+x]==cv_px[y*240+x]);
        }
    CHECK(ink>0);
    /* A multi-control summary has its per-control caption only. A second
     * section-summary TUNE at x11 produces the observed TUTUNE collision. */
    cv_begin(240,138,SOUND_FACE);text_c(34,40,&FONT_T,"TUNE",SOUND_GRAY);
    for(int y=40;y<52;y++)for(int x=11;x<54;x++)
        CHECK(actual[y*240+x]==cv_px[y*240+x]);
    puts("Normal-frame caption survives strip boundary and TUNE is not overprinted PASS");return 0;
}
