/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static void check_toast(const char *id)
{
    char before[sizeof ui.msg]; memcpy(before,ui.msg,sizeof before);
    uint32_t until=ui.msg_until;
    pattern_t pattern_before=proj.pat[15];
    for(int level=0;level<4;level++) {
        host_battery_level=level;now_ms+=25;ui_frame();
        CHECK(ui.msg_until==until && ui.msg_until>now_ms);
        CHECK(!memcmp(before,ui.msg,sizeof before));
        CHECK(ui.view==V_HOME && ui.part==PART_909);
        CHECK(!memcmp(&pattern_before,&proj.pat[15],sizeof pattern_before));
        for(int x=219;x<233;x++) {
            CHECK(fb[5*240+x]==swap16(SOUND_GRAY));
            CHECK(fb[12*240+x]==swap16(SOUND_GRAY));
        }
        for(int cell=0;cell<3;cell++)for(int y=7;y<11;y++)for(int x=221+cell*4;x<224+cell*4;x++)
            CHECK(fb[y*240+x]==swap16(cell<level?SOUND_GRAY:SOUND_LINE));
    }
    shot(id);
}
static void next_pattern_is_preserved(void)
{
    uint16_t actual[HDR_H*214];
    for(int y=0;y<HDR_H;y++)for(int x=0;x<214;x++)actual[y*214+x]=fb[y*240+x];
    cv_begin(240,HDR_H,C_BLACK);night_panel(0,0,240,HDR_H,0);
    box(0,0,240,HDR_H-1,part_col());
    cv_text(65,vc(&FONT_B,0,19),&FONT_B,"NEXT",C_BLACK);
    frame(104,1,12,17,C_BLACK);
    for(int pos=0;pos<NPARTS;pos++) {
        uint8_t strength[8];
        home_pattern_strength8(&proj.pat[15],proj.arr.lane,15,part_at(pos),proj.set.accent_q7,T_VEL,strength);
        pat_glyph_row(106,2+pos*3,strength,C_BLACK,0);
    }
    cv_text(122,vc(&FONT_B,0,19),&FONT_B,"> 16",C_BLACK);
    for(int y=0;y<HDR_H;y++)for(int x=0;x<214;x++)CHECK(actual[y*214+x]==cv_px[y*240+x]);
}
static void tiny_text_fits(const char *text,int row)
{
    int x=107-tw(&FONT_T,text)/2,y=row?9:-1;
    for(const unsigned char *p=(const unsigned char*)text;*p;p++) {
        unsigned gi=glyph(&FONT_T,*p);
#if defined(FONT_LZ_CROP) || defined(FONT_ALPHA_CROP)
        const uint8_t *d=FONT_T.data+font_offset(&FONT_T,gi);
        unsigned r=font_crop_read(&d);
        int left=x-(int)FONT_T.pad+(int)(r&63u),top=y+(int)((r>>6)&63u);
        unsigned width=(r>>12)&63u,height=r>>18;
        if(width && height) {
            CHECK(left>=4 && left+(int)width<=210);
            CHECK(top>=(row?10:0) && top+(int)height<=(row?19:10));
        }
#endif
        x+=FONT_T.adv[gi];
    }
}
static void long_text_is_complete(void)
{
    uint16_t actual[HDR_H*214];
    for(int y=0;y<HDR_H;y++)for(int x=0;x<214;x++)actual[y*214+x]=fb[y*240+x];
    cv_begin(240,HDR_H,C_BLACK);night_panel(0,0,240,HDR_H,0);
    box(0,0,240,HDR_H-1,part_col());
    const char *first="DnBOX 99.99.99_openbeta  MEM 100%  CPU 100%";
    const char *second="g123456789abc";
    CHECK(tw(&FONT_T,first)<=206 && tw(&FONT_T,second)<=206);
    tiny_text_fits(first,0);tiny_text_fits(second,1);
    text_c(107,-1,&FONT_T,first,C_BLACK);
    text_c(107,9,&FONT_T,second,C_BLACK);
    for(int y=0;y<HDR_H;y++)for(int x=0;x<214;x++)CHECK(actual[y*214+x]==cv_px[y*240+x]);
}
int main(void)
{
    setup(V_HOME,PART_909);say("SAVED",0);check_toast("toast-saved");
    setup(V_HOME,PART_909);sm_set(proj.pat[15].drum[PART_909].hit[0],0);
    sm_set(proj.pat[15].drum[PART_909].hit[0],8);
    say("NEXT: PAT 16",0);check_toast("toast-next-pattern");next_pattern_is_preserved();
    setup(V_HOME,PART_909);run_action(ACT_ABOUT,0);check_toast("toast-about-host-identity");
    setup(V_HOME,PART_909);say("DnBOX 99.99.99_openbeta  MEM 100%  CPU 100% g123456789abc",0);
    check_toast("toast-about-synthetic-long");long_text_is_complete();
    printf("PASS toast battery: 4 states x 4 levels; text/state preserved; %u checks\n",checks);
    return 0;
}
