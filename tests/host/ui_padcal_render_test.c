/* SPDX-License-Identifier: GPL-3.0-only */
/* Actual LCD calibration renderer with RAM-only fixture state. No pad/device I/O. */
#define smk_padcal_draw substitute_padcal_draw
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
#undef smk_padcal_draw
#include "app/apc40_lcd565.h"
#include "app/smk_padcal.h"
#include "app/apc40_pad_gamma.h"
#include "app/apc40_pad_names.h"
static smk_padcal_t padcal;
static uint32_t padcal_sig;
#include "actual_padcal.inc"
static unsigned states;
static void render_state(const char *capture)
{
    smk_padcal_t saved=padcal;padcal_sig=~0u;
    draw_header();draw_knobs();CHECK(smk_padcal_draw(240,2*BAND_H));
    CHECK(!memcmp(&saved,&padcal,sizeof padcal));cv_commit_rect(1,0,MAIN_Y);
    if(capture)shot(capture);
    memcpy(before_fb,fb,sizeof fb);CHECK(!smk_padcal_draw(240,2*BAND_H));
    CHECK(!memcmp(before_fb,fb,sizeof fb));
    if(!padcal.orient) {
        uint8_t entry=smk_padcal_entry(padcal.page,padcal.sel);
        unsigned shift=padcal.real?0:smk_padcal_boost_shift(APC40_LCD565[entry]);
        for(unsigned i=0;i<16;i++) {
            int x=(i&7)*30,y=(i>>3)*30;
            uint8_t n=smk_padcal_entry(padcal.page,i);
            uint16_t colour=i==padcal.sel?smk_padcal_boost565(APC40_LCD565[n],shift):APC40_LCD565[n];
            CHECK(x>=0 && x+30<=240 && y+30<=2*BAND_H);
            CHECK(fb[(MAIN_Y+y+5)*240+x+5]==swap16(colour));
            CHECK(fb[(MAIN_Y+y+15)*240+x+15]==swap16(smk_padcal_is_ok(&padcal,n)?smk_padcal_dot565(colour):colour));
            CHECK(fb[(MAIN_Y+y)*240+x]==swap16(i==padcal.sel?C_WHITE:C_BLACK));
        }
    }
    states++;
}
int main(void)
{
    setup(V_HOME,PART_909);button_tap(B_GLO);CHECK(ui.view==V_GLO && ui.overlay==O_LIST);
    ui.list_sel=5;button_tap(B_SEL);CHECK(!strcmp(list_title,"SYSTEM"));
    int row=-1;for(int i=0;i<ui.list_n;i++)if(list_rows[i].kind==R_ACT && list_rows[i].a==ACT_PADCAL)row=i;
    CHECK(row>=0);ui.list_sel=(uint8_t)row;button_tap(B_SEL);CHECK(ui.overlay==O_NONE);
    /* The host enter hook is a stub; initialize only the renderer's RAM state.
     * Header context came through the genuine GLOBAL > SYSTEM action, not HOME. */
    smk_padcal_start(&padcal,APC40_LCD565,PAD_GAMMA_LUT,PAD_GAIN_Q8,PAD_FLOOR_LEVEL);
    CHECK(padcal.on && padcal.orient);render_state("padcal-orientation");padcal.orient=0;
    for(int page=0;page<8;page++)for(int sel=0;sel<16;sel++)for(int real=0;real<2;real++)for(int marked=0;marked<2;marked++) {
        padcal.page=(uint8_t)page;padcal.sel=(uint8_t)sel;padcal.real=(uint8_t)real;
        memset(padcal.ok,marked?255:0,sizeof padcal.ok);
        char name[64];snprintf(name,sizeof name,"padcal-page-%d",page);
        render_state(sel==0 && !real && !marked?name:NULL);
    }
    CHECK(states==513);
    printf("PASS actual padcal LCD: %u checks across %u RAM-only states; nine sample captures, not calibration/device qualification\n",checks,states);
    return 0;
}
