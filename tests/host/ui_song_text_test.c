/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static unsigned empty_text,end_text,explanatory_text;
static int32_t observed_song_text(int32_t x,int32_t y,const felucca_font_t *f,const char *s,uint16_t c)
{
    empty_text+=!strcmp(s,"EMPTY");end_text+=!strcmp(s,"END");
    explanatory_text+=strstr(s,"WHITE KEYS ADD BARS")!=0;
    return cv_text(x,y,f,s,c);
}
#define cv_text observed_song_text
#include "song_text_current.inc"
#include "song_text_before.inc"
#undef cv_text
static uint16_t actual[240*69],observed[240*69],prior[240*69];
static void render_song(void (*draw)(int),int band,uint16_t *dst)
{
    cv_begin(240,69,C_BLACK);cv_band(0,69);draw(band);
    memcpy(dst,cv_px,sizeof actual);
}
int main(void)
{
    unsigned changed=0;
    for(int length=0;length<=2;length++)for(int selected=0;selected<=length;selected++)for(int band=0;band<2;band++) {
        setup(V_SONG,PART_909);song()->len=(uint8_t)length;ui.song_sel=(uint8_t)selected;
        song_t before=*song();screen_state_t screen=screen_state();int playing=seq.playing;
        empty_text=end_text=explanatory_text=0;
        render_song(draw_song,band,actual);render_song(observed_draw_song,band,observed);
        CHECK(!memcmp(actual,observed,sizeof actual));CHECK(!explanatory_text);
        if(!band){CHECK(length?end_text==1:empty_text==1);}
        else CHECK(!empty_text && !end_text);
        render_song(before_draw_song,band,prior);
        if(length||band)CHECK(!memcmp(actual,prior,sizeof actual));
        else {CHECK(explanatory_text==1);CHECK(memcmp(actual,prior,sizeof actual)!=0);changed++;}
        CHECK(!memcmp(&before,song(),sizeof before));unchanged(screen);CHECK(seq.playing==playing);
    }
    CHECK(changed==1);
    printf("PASS SONG EMPTY/END actual graphics, 12 states, %u checks; no input/hardware or full-screen acceptance\n",checks);
    return 0;
}
