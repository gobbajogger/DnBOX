/* SPDX-License-Identifier: GPL-3.0-only */
/* Real firmware UI raster, actual engine metadata, both kits and all voices. */
#define SMC_UI_MAIN unused_smc_ui_main
#include "smc_ui_test.c"
int main(void)
{
    for (int kit=0; kit<2; kit++) for (int voice=0; voice<NDRUM; voice++) {
        setup(V_PART,kit); ui.sel[kit]=voice; build_pages();
        for(int step=0;step<16;step+=4)sm_set(cur_pat()->drum[kit].hit[voice],step);
        project_t *before=malloc(sizeof proj);CHECK(before);memcpy(before,&proj,sizeof proj);
        ui_frame();CHECK(!memcmp(before,&proj,sizeof proj));free(before);
        char name[64]; snprintf(name,sizeof name,"r22-%d-voice-%02d",kit?808:909,voice); shot(name);
    }
    /* Section/page metadata must survive added per-control labels. */
    setup(V_PART,PART_909);build_pages();
    cv_begin(240,2*BAND_H,C_BLACK);cv_band(0,BAND_H);draw_drum_voice(0);
    uint16_t label_region[40*12];
    for(int y=0;y<12;y++)for(int x=0;x<40;x++)label_region[y*40+x]=cv_px[(8+y)*240+11+x];
    cv_begin(240,2*BAND_H,RGB(36,48,54));cv_text(11,8,&FONT_T,"SOUND",SOUND_GRAY);
    for(int y=0;y<12;y++)for(int x=0;x<40;x++)CHECK(label_region[y*40+x]==cv_px[(8+y)*240+11+x]);
    /* Every inactive multi-control row retains distinct captions and cells. */
    setup(V_PART,PART_909);*page_of(V_PART)=1;build_pages();
    cv_begin(240,2*BAND_H,C_BLACK);cv_band(0,BAND_H);draw_drum_voice(0);
    uint16_t inactive_pixels[240*BAND_H];memcpy(inactive_pixels,cv_px,sizeof inactive_pixels);
    /* Shared four-track centre is34, not the retired hardcoded33.
     * The original exact22 pixel failure remains in its immutable ledger. */
    cv_begin(240,BAND_H,SOUND_FACE);text_c(34,20,&FONT_T,"TUNE",SOUND_GRAY);
    unsigned caption_ink=0;
    for(int y=20;y<32;y++)for(int x=16;x<54;x++)if(cv_px[y*240+x]!=swap16(SOUND_FACE)) {
        caption_ink++;CHECK(inactive_pixels[y*240+x]==cv_px[y*240+x]);
    }
    CHECK(caption_ink>0);
    /* A 2px rounded stroke retains a solid centre with coverage on both sides.
     * Reverting to the frozen 1px Wu renderer removes these shoulders. */
    cv_begin(240,BAND_H,C_BLACK);
    cv_line_aa(20.f,10.f,20.f,30.f,C_WHITE);
    CHECK(cv_px[20*240+20]==swap16(C_WHITE));
    CHECK(cv_px[20*240+19]!=swap16(C_BLACK));
    CHECK(cv_px[20*240+21]!=swap16(C_BLACK));
    /* The fork's curved bowl must not regress to the old square corners. */
    cv_begin(240,BAND_H,C_BLACK);dv_tune_fork(40,10,1,1,.27f,C_WHITE);
    CHECK(cv_px[21*240+49]==swap16(C_BLACK));
    CHECK(cv_px[21*240+56]!=swap16(C_BLACK));
    /* Real elapsed-time ease, both directions; no overshoot and <=0.5% at 192ms. */
    pref_t r=PR(R_ENG,T_909,0,0);
    memset(ui_ease,0,sizeof ui_ease);now_ms=1000;ui_ease_last_ms=1000;
    CHECK(ui_ease_float(r,0)==0);CHECK(ui_ease_float(r,127)==0);
    float previous=0;
    for(int i=0;i<12;i++) {now_ms+=16;ui_ease_tick();float y=ui_ease_float(r,127);CHECK(y>=previous && y<=127);previous=y;}
    CHECK(previous>126.365f);
    ui_ease_float(r,0);previous=127;
    for(int i=0;i<12;i++) {now_ms+=16;ui_ease_tick();float y=ui_ease_float(r,0);CHECK(y<=previous && y>=0);previous=y;}
    CHECK(previous<.635f);
    /* Long pauses advance real elapsed time, not one truncated frame. */
    memset(ui_ease,0,sizeof ui_ease);now_ms=1000;ui_ease_last_ms=1000;
    CHECK(ui_ease_float(r,0)==0);ui_ease_float(r,127);now_ms+=500;ui_ease_tick();
    CHECK(ui_ease_float(r,127)>126.99f);
    /* A complete UI re-init may not animate from the previous project's value. */
    ui_init();CHECK(ui_ease_float(r,8)==8);
    /* Pitch ratio is not Tune's time constant; never reuse the tuning fork. */
    setup(V_PART,PART_909);cv_begin(240,BAND_H,C_BLACK);
    dv_glyph(PR(R_ENG,T_909,0,0),45,30,10,1,1,C_WHITE);
    uint16_t tune_pixels[240*BAND_H];memcpy(tune_pixels,cv_px,sizeof tune_pixels);
    cv_begin(240,BAND_H,C_BLACK);dv_glyph(PR(R_ENG,T_909,0,5),45,30,10,1,1,C_WHITE);
    CHECK(memcmp(tune_pixels,cv_px,sizeof tune_pixels)!=0);
    /* Presentation range never clamps the saved raw pan or changes DSP law. */
    const int pan_raw_cases[]={0,1,32,63,64,65,96,127};
    const char *pan_expected[]={"L50","L50","L25","L1","C","R1","R25","R50"};
    for(int i=0;i<8;i++) {
        char num[32],unit[16];
        pref_value_of(PR(R_ENG,T_MIX,0,MX_PAN),pan_raw_cases[i],num,unit);
        CHECK(!strcmp(num,pan_expected[i]));CHECK(!unit[0]);
    }
    for(int raw=0;raw<128;raw++) {
        int d=ui_pan_display(raw);CHECK(d>=-50 && d<=50);
        if(raw)CHECK(d>=ui_pan_display(raw-1));
        CHECK(ui_pan_pos(raw,100)==d+50);
    }
    for(int d=-50;d<=50;d++)CHECK(ui_pan_display(ui_pan_raw(d))==d);
    CHECK(ui_pan_raw(-50)==0);CHECK(ui_pan_raw(50)==127);
    CHECK(ui_pan_pos(0,36)==0);CHECK(ui_pan_pos(64,36)==18);CHECK(ui_pan_pos(127,36)==36);
    printf("r22-drum PASS (%u checks, 22 actual voice renders)\n",checks);
    return 0;
}
