/* SPDX-License-Identifier: GPL-3.0-only */
/* Actual LCD calibration renderer with RAM-only fixture state. No pad/device I/O. */
#define SMC_UI_REAL_PADCAL
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
#include "app/apc40_lcd565.h"
#include "app/smk_padcal.h"
#include "app/apc40_pad_gamma.h"
#include "app/apc40_pad_names.h"
static smk_padcal_t padcal;
static uint32_t padcal_sig;
#include "actual_padcal.inc"
static uint32_t padcal_btn, padcal_pads;
static uint8_t smk_pad_grid;
static struct { uint32_t buttons, pnotes; } fm1_in;
static struct { uint8_t btn[NB], enc[NE]; int8_t dir[NE]; } panel;

static int32_t fm1_enc_take(unsigned encoder) { (void)encoder; return 0; }
static void padled_set(unsigned pad, unsigned r, unsigned g, unsigned b)
{ (void)pad; (void)r; (void)g; (void)b; }
static void padled_flush(void) {}
uint8_t smk_padcal_on(void) { return padcal.on; }
#define SMK_WHITE_NOTE WHITE_KEY
#include "actual_padcal_dispatch.inc"
#undef SMK_WHITE_NOTE

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
static void board_button(unsigned button)
{
    fm1_in.buttons = 1u << button;
    smk_padcal_frame();
    fm1_in.buttons = 0;
    smk_padcal_frame();
}

static void test_smc_restore(void)
{
    CHECK(padcal.on && padcal.orient && !clip_grid_mode);
    board_button(B_PLAY);
    CHECK(!padcal.orient);
    board_button(B_OCTUP);
    CHECK(padcal.page == 1);
    ui_frame();
    shot("padcal-before-pickup");
    memcpy(before_fb, fb, sizeof fb);

    const pref_t assigned = smc_target(0);
    CHECK(assigned.kind != R_NONE);
    const int value = pref_get(assigned);
    CHECK(value > 0);
    const smk_padcal_t calibrated = padcal;
    const uint32_t sig = padcal_sig;
    fader(0, 0);
    CHECK(smc_ui.on && smc_ui.kind == 0 && smc_ui.strip == 0 && !smc_peek->on);
    CHECK(pref_get(assigned) == value);
    const uint32_t deadline = smc_ui.until;
    CHECK(deadline - now_ms == SMC_UI_MS);
    ui_frame();
    shot("padcal-smc-pickup");
    CHECK(changed_rows(MAIN_Y, KNOB_Y, before_fb, fb) > 20);
    now_ms = deadline - 1;
    ui_frame();
    CHECK(smc_ui.on && padcal_sig == sig);
    now_ms = deadline;
    ui_frame();
    shot("padcal-smc-expiry");
    CHECK(!smc_ui.on && padcal.on && padcal_sig == sig);
    CHECK(!memcmp(&padcal, &calibrated, sizeof padcal));
    unsigned stale_rows = changed_rows(MAIN_Y, KNOB_Y, before_fb, fb);
    now_ms += 240;
    ui_frame();
    shot("padcal-smc-expiry-plus-240ms");
    unsigned later_rows = changed_rows(MAIN_Y, KNOB_Y, before_fb, fb);
    printf("Pad Colours SMC expiry: differing body rows at expiry=%u, at +240 ms=%u\n",
           stale_rows, later_rows);
    CHECK(stale_rows == 0 && later_rows == 0);
}

int main(void)
{
    for (unsigned i = 0; i < NB; ++i) panel.btn[i] = i;
    for (unsigned i = 0; i < NE; ++i) {
        panel.enc[i] = i;
        panel.dir[i] = 1;
    }
    setup(V_HOME, PART_909);
    button_tap(B_GLO);
    CHECK(ui.view == V_GLO && ui.overlay == O_LIST);
    for (int i = 0; i < LIST_MAX && list_rows[ui.list_sel].a != 5; ++i) {
        enc_acc[EN_SELECT] = 1;
        input();
    }
    CHECK(list_rows[ui.list_sel].kind == R_SECTION && list_rows[ui.list_sel].a == 5);
    button_tap(B_SEL);
    CHECK(!strcmp(list_title, "SYSTEM"));
    for (int i = 0; i < LIST_MAX && list_rows[ui.list_sel].a != ACT_PADCAL; ++i) {
        enc_acc[EN_SELECT] = 1;
        input();
    }
    CHECK(list_rows[ui.list_sel].kind == R_ACT && list_rows[ui.list_sel].a == ACT_PADCAL);
    button_tap(B_SEL);
    CHECK(ui.overlay == O_NONE && padcal.on);
    render_state("padcal-orientation");
    test_smc_restore();
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
