/* SPDX-License-Identifier: GPL-3.0-only */
/* Actual SMC routes and final framebuffer, with an independent native-font mask. */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static void opaque_text(int x,int y,const felucca_font_t *font,const char *text,uint16_t color)
{
    unsigned pixels=0,missing=0;
    uint16_t expected=swap16(color);
    cv_begin(240,138,C_BLACK);cv_text(10,0,font,text,C_WHITE);
    for(int yy=0;yy<font->h;yy++)for(int xx=0;xx<240;xx++)if(cv_px[yy*240+xx]==swap16(C_WHITE)) {
        int sx=x+xx-10,sy=y+yy;
        if(!pixels && !color && sx>=0&&sx<240&&sy>=0&&sy<240) {
            expected=fb[sy*240+sx];
            if(expected!=swap16(C_GRAY)&&expected!=swap16(C_HI)&&expected!=swap16(C_DIM))
                fprintf(stderr,"SMC_CAPTION_GEOMETRY text=[%s] invalid foreground ink at=%d,%d\n",text,sx,sy);
            CHECK(expected==swap16(C_GRAY)||expected==swap16(C_HI)||expected==swap16(C_DIM));
        }
        pixels++;
        if(sx<0||sx>=240||sy<0||sy>=240||fb[sy*240+sx]!=expected)missing++;
    }
    if(!pixels||missing)fprintf(stderr,"SMC_CAPTION_GEOMETRY text=[%s] at=%d,%d missing=%u/%u\n",text,x,y,missing,pixels);
    CHECK(pixels && !missing);
}
static void pattern_sound(const char *name,int waiting,int field)
{
    setup(V_SEQ,PART_303A);clip_grid_mode=1;clip_waiting=(uint8_t)waiting;
    if(field) {ck.pg=1;knob(0,1);}
    else {smk_clip_lcd_force();ui_frame();fader(0,0);}
    ui_frame();CHECK(smc_ui_active());
    pref_t target=smc_knob_target(4);CHECK(target.kind==R_BPATSND);
    opaque_text(135-tw(&FONT_T,"SOUND")/2,MAIN_Y+14,&FONT_T,"SOUND",C_GRAY);
    shot(name);
}
int main(void)
{
    pattern_sound("smc-caption-pattern-sound-grid",0,0);
    pattern_sound("smc-caption-pattern-sound-wait",1,0);
    pattern_sound("smc-caption-pattern-sound-field",0,1);
    setup(V_FX,PART_909);smc.view=V_FX;smc_ui_begin(2,0);smc_ui.kind=3;smc_ui.action=SMC_FILL_NOTE;
    CHECK(smc_fx_bus_now(SMC_FILL_NOTE)>=0);ui_frame();
    shot("smc-caption-fx-before-audit");
    unsigned verified=0;static const unsigned note_row[4]={16,8,0,24};
    for(int n=0;n<8;n++)for(int row=0;row<4;row++) {
        const char *caption=smc_button_caption((int)note_row[row]+n);
        if(strcmp(caption,"STUT") && strcmp(caption,"SHUF"))continue;
        const felucca_font_t *font=tw(&FONT_T,caption)<=20?&FONT_T:&FONT_XS;
        const capm_t *cap=caps(font);
        int y=MAIN_Y+62+row*12+1+(9-(cap->bot-cap->top+1))/2-cap->top;
        opaque_text(n*30+18-tw(font,caption)/2,y,font,caption,0);verified++;
    }
    CHECK(verified>=9);shot("smc-caption-fx-grid");
    printf("PASS SMC final-frame caption geometry (%u FX labels, %u checks)\n",verified,checks);return 0;
}
