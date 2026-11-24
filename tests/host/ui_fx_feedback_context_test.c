/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static uint16_t context[240*240];
static uint32_t project_hash(void) {const uint8_t *p=(const uint8_t*)&proj;uint32_t h=2166136261u;for(unsigned i=0;i<sizeof proj;i++)h=(h^p[i])*16777619u;return h;}

static unsigned frame_delta(const uint16_t *before,int x0,int x1,int y0,int y1)
{
    unsigned delta=0;
    for(int y=y0;y<y1;y++)for(int x=x0;x<x1;x++)
        delta+=before[y*240+x]!=fb[y*240+x];
    return delta;
}

static void touch_in_own_strip(int fx)
{
    setup(V_HOME,PART_909);
    if(fx){button_tap(B_FX);ui_input_only();button_tap(B_FX);ui_input_only();}
    else {button_tap(B_LFO);ui_input_only();}
    int slot=fx?1:0;
    CHECK(ui.view==(fx?V_FX:V_MIX)&&cur_page()==(fx?1:0));
    pref_t control=pg.r[cur_page()][slot];
    CHECK(control.kind==R_ENG&&control.a==(fx?T_FX:T_MIX)&&control.c==(fx?FX_DL_FDBK:MX_LEVEL));
    const char *name=fx?"FEEDBACK":"LEVEL";char n[36],u[8];
    CHECK(!strcmp(pref_name(control),name));
    ui.msg_until=0;ui.touched=-1;ui_frame();shot(fx?"fx-01-idle":"mix-01-idle");
    for(int i=0;i<8;i++){now_ms+=128;enc_acc[EN_K1+slot]=1;ui_input_only();audio_block();}
    CHECK(ui.touched==slot&&ui.touch_until>now_ms);
    pref_value(control,n,u);
    CHECK(!strcmp(n,fx?"40":"-3.2")&&!strcmp(u,fx?"%":"dB"));
    ui_frame();shot(fx?"fx-02-touched":"mix-02-touched");memcpy(context,fb,sizeof context);
    uint16_t label[20*60];
    for(int y=0;y<20;y++)memcpy(label+y*60,fb+(KNOB_Y+5+y)*240+slot*60,60*sizeof(uint16_t));
    cv_begin(240,KNOB_H,C_BLACK);
    text_c(slot*60+30,vc(&FONT_XS,13,10),&FONT_XS,name,C_WHITE);
    unsigned white=0;
    for(int y=5;y<25;y++)for(int x=0;x<60;x++)if(cv_px[y*240+x+slot*60]==swap16(C_WHITE)){
        CHECK(label[(y-5)*60+x]==swap16(C_WHITE)); white++;
    }
    CHECK(white>0);
    now_ms+=1200;ui_frame();shot(fx?"fx-03-settled":"mix-03-settled");
    unsigned center=frame_delta(context,8,232,103,154);
    unsigned footer=frame_delta(context,slot*60,(slot+1)*60,KNOB_Y,240);
    printf("TOUCH owner=%s center_changes=%u bound_footer_changes=%u\n",name,center,footer);
    CHECK(!center&&footer>0&&ui.touched<0);
}

static void unrelated_fx_touch_keeps_feedback_card(void)
{
    setup(V_HOME,PART_909);button_tap(B_FX);ui_input_only();button_tap(B_FX);ui_input_only();
    CHECK(ui.view==V_FX&&cur_page()==1);
    CHECK(pg.r[1][2].a==T_FX&&pg.r[1][2].c==FX_DL_TONE);
    now_ms+=128;enc_acc[EN_K3]=1;ui_input_only();audio_block();
    CHECK(ui.touched==2);
    ui_frame();shot("fx-tone-unrelated-02-touched");memcpy(context,fb,sizeof context);
    now_ms+=1200;ui_frame();shot("fx-tone-unrelated-03-settled");
    CHECK(frame_delta(context,8,232,103,154)>1000);
}

int main(void)
{
    static const int raw[]={0,1,52,126,127};
    static const char *const value[]={"0","1","35","84","85"};
    for(int mode=0;mode<2;mode++)for(unsigned k=0;k<sizeof raw/sizeof raw[0];k++) {
        setup(V_FX,PART_909);*page_of(V_FX)=0;build_pages();
        pref_t r=PR(R_ENG,T_FX,0,FX_DL_FDBK);
        pref_set(PR(R_ENG,T_FX,0,FX_DL_TYPE),mode);pref_set(r,raw[k]);
        char num[16],unit[8],want[20];pref_value(r,num,unit);
        CHECK(!strcmp(num,value[k]));CHECK(!strcmp(unit,"%"));
        put_s(put_s(want,value[k]),"%");
        uint32_t before=project_hash();
        draw_main();memcpy(context,fb,sizeof context);
        CHECK(before==project_hash());
        int width=tw(&FONT_T,want),x0=111-width,y0=141;
        cv_begin(240,138,0);cv_text(0,0,&FONT_T,want,C_WHITE);
        int ink=0;
        for(int y=0;y<FONT_T.h;y++)for(int x=0;x<width;x++)
            if(cv_px[y*240+x]==swap16(C_WHITE)) {
                if(context[(y0+y)*240+x0+x]!=cv_px[y*240+x]) {
                    fprintf(stderr,"feedback-context-mismatch mode=%d raw=%d expected=%s\n",mode,raw[k],want);CHECK(0);
                }
                ink++;
            }
        CHECK(ink>0);
    }
    touch_in_own_strip(1);touch_in_own_strip(0);unrelated_fx_touch_keeps_feedback_card();
    printf("PASS FX configured feedback context/knob boundaries: %u checks\n",checks);
    return 0;
}
