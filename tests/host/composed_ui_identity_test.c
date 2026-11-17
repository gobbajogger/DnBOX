/* SPDX-License-Identifier: GPL-3.0-only */
/* Current identity/component behavior and cache coherence, not a historical header golden. */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static uint16_t reference_pixels[240 * MAIN_ROWS];
static void reference_header_commit(void){memcpy(reference_pixels,cv_px,240*HDR_H*2);}
#define cv_commit_header reference_header_commit
#include "reference/ui_perf_8be_reference.inc"
#undef cv_commit_header
static void expect_current_battery(void)
{
    /* Four levels are an outlined battery with zero through three filled cells. */
    int level=plat_battery();level=level<0?0:level>3?3:level;
    for(int y=5;y<=12;y++)for(int x=219;x<=234;x++){
        int edge=(x<=232&&(y==5||y==12||x==219||x==232))||(x>=233&&y>=7&&y<=10);
        int cell=y>=7&&y<=10&&x>=221&&x<=231&&(x-221)%4<3;
        if(!edge&&!cell)continue;
        uint16_t expected=swap16(edge?SOUND_GRAY:((x-221)/4<level?SOUND_GRAY:SOUND_LINE));
        if(fb[y*240+x]!=expected)fprintf(stderr,"CURRENT_BATTERY_MISMATCH x=%d y=%d level=%d\n",x,y,level);
        CHECK(fb[y*240+x]==expected);
    }
}
static void expect_current_title(const char *expected)
{
    CHECK(tw(&FONT_B,expected)<=121);

    /* draw_header currently paints the rounded background before status
     * dispatch and again on the ordinary-title path. Preserve both AA passes:
     * a single pass differs at the rounded border before any title ink. */
    cv_begin(240,HDR_H,C_BLACK);night_panel(0,0,240,HDR_H,0);
    sound_slab(0,0,240,HDR_H,0);
    cv_text_clip(4,2,&FONT_B,expected,part_col(),4,125);
    for(int y=0;y<HDR_H;y++)for(int x=4;x<125;x++){
        if(fb[y*240+x]!=cv_px[y*240+x])fprintf(stderr,"CURRENT_TITLE_MISMATCH expected=%s x=%d y=%d\n",expected,x,y);
        CHECK(fb[y*240+x]==cv_px[y*240+x]);
    }
}
static void same_header(void)
{
    /* The normal path must equal a fresh render at identical live state. This
     * retains full-frame cache regression coverage without obsolete pixel art. */
    draw_header();memcpy(reference_pixels,fb,240*HDR_H*2);
    part_header_snapshot_valid=0;blit_hash[0]=0;cv_tile_valid[0]=0;
    draw_header();
    if(memcmp(reference_pixels,fb,240*HDR_H*2))fprintf(stderr,"CURRENT_HEADER_CACHE_MISMATCH view=%u part=%u\n",ui.view,ui.part);
    CHECK(!memcmp(reference_pixels,fb,240*HDR_H*2));
    expect_current_battery();
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
    /* Independent public identity table; never derived from renderer names. */
    static const struct {int kind; const char *name[2]; const char *tag;} identity[]={
        {ORIGINAL_SOUND_CHICAGO,{"CHICAGO A","CHICAGO B"},"chicago"},
        {ORIGINAL_SOUND_MIAMI,{"MIAMI A","MIAMI B"},"miami"},
        {ORIGINAL_SOUND_SYNTHONE,{"S1 A","S1 B"},"synthone"},
        {ORIGINAL_SOUND_JUNO,{"SUPERJ6 A","SUPERJ6 B"},"superj6"}
    };
    for(int voice=0;voice<2;voice++) {
        setup(V_PART,PART_303A+voice);
        for(unsigned bank=0;bank<sizeof identity/sizeof identity[0];bank++) {
            int32_t audio[512];
            CHECK(engine_bass_select_preset(voice,identity[bank].kind,0,&proj.sound)==0);
            for(int n=0;n<1024&&engine_bass_preset_pending(voice);n++)engine_render(audio,256);
            CHECK(!engine_bass_preset_pending(voice));CHECK(engine_bass_bank(voice)==identity[bank].kind);
            build_pages();ui.msg_until=0;
            /* Same part/theme, primed cache: bank change itself must refresh the title. */
            same_header();expect_current_title(identity[bank].name[voice]);
            for(int theme=0;theme<6;theme++) {
                palette_set(theme);draw_main();draw_knobs();draw_footer();same_header();same_header();
                expect_current_title(identity[bank].name[voice]);
                CHECK(!strcmp(part_header_snapshot.name,identity[bank].name[voice]));
                if(theme==0||theme==5){char file[64];snprintf(file,sizeof file,"identity-%s-%c-%s",identity[bank].tag,'a'+voice,theme?"light":"dark");shot(file);}
                say("RESET SOUND",0);same_header();now_ms=ui.msg_until;same_header();
            }
        }
    }
    static const char *drum_title[2][2]={{"909 KICK","909 RIMSHOT"},{"808 KICK","808 RIMSHOT"}};
    for(int kit=0;kit<2;kit++){
        setup(V_PART,kit);
        for(int v=0;v<2;v++){ui.sel[kit]=v?5:0;build_pages();same_header();expect_current_title(drum_title[kit][v]);}
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
