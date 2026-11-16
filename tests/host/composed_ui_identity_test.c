/* SPDX-License-Identifier: GPL-3.0-only */
/* Compare R22 header source/cache regressions and unchanged retained 8be arc/wave. */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static uint16_t reference_pixels[240 * MAIN_ROWS];
static void reference_header_commit(void) { memcpy(reference_pixels,cv_px,240*HDR_H*2); }
#define cv_commit_header reference_header_commit
#include "reference/ui_perf_8be_reference.inc"
#undef cv_commit_header
/* Explicit reviewed metro correction overlay; retained d13 oracle bytes stay authentic.
 * Only the playing triangle at the shared R22 position regains audio-owned phase. */
static void r22_phase_reference_icon(int32_t x,int32_t y,uint16_t colour)
{
    if(x==168 && y==4 && seq.playing)
        colour=!seq.metro_view?C_DIM:seq.metro_view<=4u?C_WHITE:C_HI;
    play_icon(x,y,colour);
}
#define play_icon r22_phase_reference_icon
#include "reference/ui_r22_d13_header_reference.inc"
#undef play_icon
static void same_header(void)
{
    ui_t prior=ui;
    draw_header_r22_reference();ui=prior;
    draw_header();
    if(memcmp(reference_pixels,fb,240*HDR_H*2)) {
        unsigned pixel=0;while(pixel<240*HDR_H && reference_pixels[pixel]==fb[pixel])pixel++;
        fprintf(stderr,"HEADER_MISMATCH view=%u part=%u voice=%u page=%d battery=%d tempo=%.3f rec=%u help=%u overlay=%u x=%u y=%u\n",ui.view,ui.part,is_drum()?ui.sel[ui.part]:0,cur_page(),plat_battery(),(double)seq_tempo(&seq),ui.rec,ui.help,ui.overlay,pixel%240,pixel/240);
    }
    CHECK(!memcmp(reference_pixels,fb,240*HDR_H*2));
}
int main(void)
{
    for(int theme=0;theme<6;theme++)for(int part=0;part<NPARTS;part++) {
        setup(V_PART,part);palette_set(theme);
        for(int state=0;state<32;state++) {
            ui.rec=state&1;seq.playing=(state>>1)&1;seq.metro_view=(uint8_t)(state&3);
            ui.dirty=!!(state&4);ui.save_failed=!!(state&8);
            *page_of(V_PART)=(uint8_t)(state%pg.n);build_pages();
            now_ms+=100;ui.msg_until=ui.help=0;ui.overlay=O_NONE;seq.pcue[part]=255;
            same_header();same_header();
            ui.rec^=1;same_header();seq.metro_view^=1;same_header();
            ui.save_failed^=1;same_header();palette_set((theme+1)%6);same_header();
            palette_set(theme);same_header();ui.help=1;same_header();
            ui.help=0;ui.overlay=O_ASK;same_header();ui.overlay=O_NONE;
            say("RESET SOUND",0);same_header();now_ms=ui.msg_until;same_header();
            cv_begin(240,HDR_H,C_BLACK);cv_rect(0,0,240,HDR_H,C_AMB);cv_commit_header();same_header();
        }
    }
    /* Explicit current R22 semantics and independent cache-input transitions. */
    for(int kit=0;kit<NKIT;kit++) {
        setup(V_PART,kit);seq.playing=0;seq.ext=0;ui.help=0;ui.overlay=O_NONE;ui.msg_until=0;
        for(int voice=0;voice<NDRUM;voice++) {
            ui.sel[kit]=(uint8_t)voice;build_pages();*page_of(V_PART)=0;build_pages();same_header();same_header();
            for(int level=0;level<4;level++) {
                host_battery_level=level;same_header();same_header();
                for(int cell=0;cell<3;cell++)CHECK(fb[8*240+222+cell*4]==swap16(cell<level?SOUND_GRAY:SOUND_LINE));
            }
            for(int tempo=80;tempo<=180;tempo+=50){seq.bpm=(float)tempo;same_header();same_header();}
        }
    }
    setup(V_PART,PART_909);seq.ext=0;
    for(int pattern=0;pattern<4;pattern++){seq.ppat[0]=(uint8_t)pattern;same_header();same_header();}
    for(int playing=0;playing<2;playing++)for(int songpos=0;songpos<4;songpos++){
        seq.song_on=1;seq.playing=(uint8_t)playing;seq.song_pos=(uint8_t)songpos;ui.song_sel=(uint8_t)songpos;same_header();same_header();
    }
    seq.song_on=0;seq.playing=0;
    for(int failed=0;failed<2;failed++)for(int dirty=0;dirty<2;dirty++){ui.save_failed=failed;ui.dirty=dirty;same_header();same_header();}
    setup(V_SEQ,PART_909);cur_pat()->drum[0].len=64;
    for(int page=0;page<4;page++){ui.spage=(uint8_t)page;say("PICK F1 UP",0);same_header();}
    seq.chain_a=0;seq.chain_b=1;same_header();ui.save_failed=1;same_header();
    ui.msg_until=0;clip_grid_mode=1;
    for(int clock=0;clock<2;clock++){fixture_clip_external_clock=(uint8_t)clock;same_header();}
    clip_grid_mode=0;fixture_clip_external_clock=0;host_battery_level=3;
    setup(V_HOME,PART_303A);button_tap(B_GLO);CHECK(ui.view==V_GLO&&ui.overlay==O_LIST);same_header();
    for(int section=0;section<6;section++){
        global_index();ui.list_sel=(uint8_t)section;button_tap(B_SEL);CHECK(ui.view==V_GLO&&ui.overlay==O_LIST);
        for(int row=0;row<ui.list_n;row++){ui.list_sel=(uint8_t)row;same_header();}
    }
    setup(V_HOME,PART_303A);held_btn=1u<<B_SEL;input();now_ms+=HELP_HOLD+1;input();CHECK(ui.help==B_SEL+1);same_header();held_btn=0;input();CHECK(!ui.help);same_header();
    ask(ACT_CLEAR_PART,PART_303A,"CLEAR PART?","TEST ONLY");CHECK(ui.overlay==O_ASK);same_header();button_tap(B_HOME);CHECK(ui.overlay==O_NONE);same_header();
    for(int radius=7;radius<=9;radius++)for(int theme=0;theme<6;theme++)
    for(int bipolar=0;bipolar<2;bipolar++)for(int value=-1;value<=128;value++) {
        palette_set(theme);int x=value%3==0?0:value%3==1?120:239;
        cv_begin(240,69,C_LINE);arc_reference(x,35,radius,value/127.0f,C_GRAY,C_HI,bipolar);
        memcpy(reference_pixels,cv_px,240*69*2);
        cv_begin(240,69,C_LINE);arc(x,35,radius,value/127.0f,C_GRAY,C_HI,bipolar);
        CHECK(!memcmp(reference_pixels,cv_px,240*69*2));
    }
    setup(V_MIX,PART_909);
    for(int state=0;state<240;state++) {
        palette_set(state%6);ui.part=(uint8_t)(state%NPARTS);eng_scope_w=(uint32_t)(state*7)&(SCOPE_N-1u);
        for(unsigned i=0;i<SCOPE_N;i++)eng_scope[i]=state%3==0?0:state%3==1?(int16_t)(i*997+state):32000;
        now_ms+=state%2?100:99;
        cv_begin(240,MAIN_ROWS,C_BLACK);cv_band(0,BAND_H);draw_master_wave_reference();
        memcpy(reference_pixels,cv_px,sizeof reference_pixels);
        cv_begin(240,MAIN_ROWS,C_BLACK);cv_band(0,BAND_H);draw_master_wave();
        CHECK(!memcmp(reference_pixels,cv_px,sizeof reference_pixels));
    }
    printf("Composed UI identity: %u checks PASS\n",checks);return 0;
}
