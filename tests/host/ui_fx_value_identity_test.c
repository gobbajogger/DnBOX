/* SPDX-License-Identifier: GPL-3.0-only */
/* D02: the actual old renderer must fail body/knob numeric identity. */
#define SMC_UI_MAIN unused_smc_ui_main
#include "smc_ui_test.c"
static void draw_feedback(pref_t r)
{
    cv_begin(240,KNOB_H,SOUND_FACE);
    draw_param_widget(r,4,0,59,0,C_WHITE,55,1);
}
int main(void)
{
    setup(V_FX,PART_909);
    pref_t r=PR(R_ENG,T_FX,0,FX_DL_FDBK);
    pref_set(r,0);ui_ease_reset();draw_feedback(r);
    pref_set(r,127);draw_feedback(r);
    uint16_t during[240*KNOB_H];memcpy(during,cv_px,sizeof during);
    ui_ease_reset();draw_feedback(r);
    for(int y=35;y<50;y++)for(int x=4;x<63;x++)CHECK(during[y*240+x]==cv_px[y*240+x]);
    /* The body and widget must use the same actual engine unit/scale. */
    cv_begin(240,2*BAND_H,C_BLACK);cv_band(BAND_H,BAND_H);draw_fx(1);
    uint16_t body[240*2*BAND_H];memcpy(body,cv_px,sizeof body);
    char n[16],u[8],t[24];pref_value(r,n,u);put_s(put_s(t,n),u);
    cv_begin(240,2*BAND_H,SOUND_FACE);text_r(111,121,&FONT_T,t,C_WHITE);
    unsigned ink=0;
    for(int y=121;y<133;y++)for(int x=78;x<112;x++)if(cv_px[y*240+x]!=swap16(SOUND_FACE)) {
        ink++;CHECK(body[y*240+x]==cv_px[y*240+x]);
    }
    CHECK(ink>0);
    puts("FX committed numeric/body/knob identity PASS");return 0;
}
