/* SPDX-License-Identifier: GPL-3.0-only */
/* Physical graph scales, separate from configured decay duration and axis ink. */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static uint16_t rendered[240*138];
static void label_at(int x,int y,const char *s)
{
    cv_begin(240,138,SOUND_FACE);cv_text(0,0,&FONT_T,s,SOUND_GRAY);
    int width=tw(&FONT_T,s),matched=0;
    CHECK(x>=0 && x+width<=240 && y>=0 && y+FONT_T.h<=138);
    for(int dy=0;dy<FONT_T.h;dy++)for(int dx=0;dx<width;dx++)
        if(cv_px[dy*240+dx]!=swap16(SOUND_FACE)) {
            if(rendered[(y+dy)*240+x+dx]!=cv_px[dy*240+dx])fprintf(stderr,"axis label pixel mismatch: %s at %d,%d\n",s,x+dx,y+dy);
            CHECK(rendered[(y+dy)*240+x+dx]==cv_px[dy*240+dx]);matched++;
        }
    CHECK(matched>0);
}
static void chart(float window,const char *half,const char *end)
{
    setup(V_PART,PART_909);
    drum_curve_t v={.peak=1,.fall=100,.window=window,.elapsed=-1};
    cv_begin(240,138,0);cv_rect(0,0,240,138,SOUND_FACE);
    dv_curve_row(&v,0,20,90,PART_COL[PART_909]);
    memcpy(rendered,cv_px,sizeof rendered);
    label_at(33,78,"0");
    label_at(130-tw(&FONT_T,half)/2,78,half);
    label_at(227-tw(&FONT_T,end),78,end);
    label_at(29-tw(&FONT_T,"100%"),18,"100%");
    label_at(29-tw(&FONT_T,"50%"),45,"50%");
    label_at(29-tw(&FONT_T,"0%"),72,"0%");
    /* Real curve ink is independent of gray labels/guides. A missing curve
     * cannot qualify merely because its axes occupy the panel. */
    int ink=0;
    for(int y=20;y<=76;y++)for(int x=33;x<=227;x++)
        ink+=rendered[y*240+x]==swap16(PART_COL[PART_909]);
    CHECK(ink>100);
}
static void pitch_chart_and_dense(void)
{
    setup(V_PART,PART_909);
    drum_curve_t v={.window=160,.elapsed=-1};
    engine_drum(PART_909,DR_BD,1.f);
    int32_t audio[512];engine_render(audio,256);engine_render(audio,256);
    float elapsed,hz;CHECK(engine_bd909_pitch_view(&elapsed,&hz));
    CHECK(elapsed>=0 && elapsed<=160 && hz>0);
    cv_begin(240,138,0);cv_rect(0,0,240,138,SOUND_FACE);
    dv_curve_row(&v,1,20,90,PART_COL[PART_909]);
    memcpy(rendered,cv_px,sizeof rendered);
    label_at(29-tw(&FONT_T,"640Hz"),18,"640Hz");
    label_at(29-tw(&FONT_T,"113"),45,"113");
    label_at(29-tw(&FONT_T,"20"),72,"20");
    label_at(130-tw(&FONT_T,"80")/2,78,"80");
    label_at(227-tw(&FONT_T,"160ms"),78,"160ms");
    float octaves=log2f(hz/20.f);if(octaves<0)octaves=0;if(octaves>5)octaves=5;
    int x=33+(int)(194.f*elapsed/160.f+.5f);
    int y=76-(int)((76-20)*octaves*.2f+.5f);
    CHECK(x>=33 && x<=227 && y>=20 && y<=76);
    CHECK(rendered[y*240+x]==swap16(SOUND_INK));
    /* Dense native rows must not acquire labels at the cost of their curve. */
    v=(drum_curve_t){.peak=1,.fall=100,.window=1000,.elapsed=-1};
    cv_begin(240,138,0);cv_rect(0,0,240,138,SOUND_FACE);
    dv_curve_row(&v,0,20,50,PART_COL[PART_909]);
    int gray=0,ink=0;
    for(int i=0;i<240*138;i++) {gray+=cv_px[i]==swap16(SOUND_GRAY);ink+=cv_px[i]==swap16(PART_COL[PART_909]);}
    CHECK(gray==0);CHECK(ink>100);
}
static void composed_steps(void)
{
    for(int kit=0;kit<2;kit++)for(int voice=0;voice<NDRUM;voice++) {
        setup(V_PART,kit);ui.sel[kit]=(uint8_t)voice;build_pages();
        memset(cur_pat()->drum[kit].hit[voice],0,sizeof cur_pat()->drum[kit].hit[voice]);
        for(int s=0;s<16;s+=4)sm_set(cur_pat()->drum[kit].hit[voice],s);
        for(int p=0;p<pg.n;p++) {
            *page_of(V_PART)=(uint8_t)p;build_pages();draw_main();draw_knobs();
            for(int s=0;s<16;s++)for(int dy=0;dy<4;dy++)for(int dx=0;dx<11;dx++)
                CHECK(fb[(212+dy)*240+9+s*14+dx]==swap16(s%4==0?PART_COL[kit]:SOUND_LINE));
        }
    }
}
int main(void)
{
    chart(1000,"500","1000ms");
    chart(7500,"3750","7500ms");
    pitch_chart_and_dense();
    composed_steps();
    printf("PASS native graph time window, amplitude axes, actual curve ink: %u checks\n",checks);
    return 0;
}
