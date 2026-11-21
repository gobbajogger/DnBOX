/* SPDX-License-Identifier: GPL-3.0-only */
/* Exercise the juno through the production slot selector, renderer and UI. */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"
static void render_blocks(unsigned count)
{
    int32_t audio[512];
    while (count--) engine_render(audio,256);
}
static void panel_turn(int control, int direction)
{
    now_ms += 150;
    enc_acc[control] = direction;
    ui_input_only();
}

static void panel_tap(unsigned button)
{
    held_btn = 1u << button;
    ui_input_only();
    now_ms += 40;
    held_btn = 0;
    ui_input_only();
}

static void settle_selector(int owner)
{
    for (int n = 0; n < 1024 && engine_bass_preset_pending(owner); ++n)
        render_blocks(1);
    CHECK(!engine_bass_preset_pending(owner));
    ui_input_only();
}

static void capture_route(int owner, const char *route, const char *state)
{
    char name[80];
    now_ms += 2000;
    ui_frame();
    snprintf(name, sizeof name, "slot-%c-%s-%s", 'a' + owner, route, state);
    shot(name);
}

/* Independent compact DSP landmarks. No original app parameter or 64-control
 * claim is inferred from these 16 Q127 controls. */
static void check_compact_widgets(int owner)
{
    static const char *const expected[][3] = {
        {"0%", "50%", "100%"}, {"0%", "25%", "50%"}, {"35Hz", "3.1kHz", "12kHz"},
        {"0%", "50%", "100%"}, {"0Hz", "3.5kHz", "7.0kHz"},
        {"2ms", "764ms", "3.0s"}, {"10ms", "772ms", "3.0s"}, {"0%", "50%", "100%"},
        {"10ms", "1.0s", "4.0s"}, {"OFF", "523Hz", "2.0kHz"},
        {"0.1Hz", "2.1Hz", "8.1Hz"}, {"0%", "10%", "20%"}, {"10%", "50%", "90%"},
        {"0.1Hz", "2.6Hz", "5.0Hz"}, {"0%", "25%", "50%"}, {"0%", "50%", "100%"}
    };
    static const int positions[] = {0, 64, 127};
    uint16_t low[19][26];
    for (int id = 0; id < 16; ++id) {
        pref_t r = PR(R_ENG,T_303,owner,id);
        int width = tw(&FONT_T,matrix_name(r)), left = 29 - width/2;
        CHECK(left >= 2 && left + width <= 57);  /* inside the two cell insets */
        for (int raw = 0; raw <= 127; ++raw) {
            char n[16], u[8];
            pref_value_of(r, raw, n, u);
            CHECK(*n && (*u || (id == 9 && raw == 0)));
            CHECK(tw(&FONT_XS,n) <= 27 && tw(&FONT_XS,u) <= 27);
        }
        for (int j = 0; j < 3; ++j) {
            char n[16], u[8], combined[24];
            pref_set(r, positions[j]);
            render_blocks(1);
            pref_value(r, n, u);
            snprintf(combined,sizeof combined,"%s%s",n,u);
            if (strcmp(combined,expected[id][j]))
                fprintf(stderr,"id=%d raw=%d expected=%s actual=%s\n", id, positions[j], expected[id][j], combined);
            CHECK(!strcmp(combined, expected[id][j]));
            if (id >= 5 && id <= 8) continue;  /* dedicated shared envelope graph */
            CHECK(orig_strip_slot(r));
            cv_begin(240,49,C_BLACK);
            draw_param_widget(r, 0, 0, 59, 1, C_HI, 49, 1);
            unsigned changed = 0, ink = 0;
            for (int y = 0; y < 19; ++y) for (int x = 0; x < 26; ++x) {
                uint16_t pixel = cv_px[(y+22)*240+x+3];
                ink += pixel == swap16(SOUND_INK);
                if (!j) low[y][x] = pixel; else changed += pixel != low[y][x];
            }
            /* Zero level leaves the six unlit gain segments in the line color. */
            if (id == 15 && !j) CHECK(!ink && cv_px[(22+18)*240+3] == swap16(C_LINE));
            else CHECK(ink > 10);
            if (j == 2) {
                if (changed < 5) fprintf(stderr,"glyph id=%d changed=%u\n",id,changed);
                CHECK(changed >= 5);
            }
            /* A rising saw starts low; a pulse starts high. The sub graphic
             * has two square cycles at half the carrier frequency. The PWM
             * oscillator is triangular and starts at its minimum. */
            if (id == 0 && j != 1)
                CHECK(cv_px[(22+9+(j ? -7 : 7))*240+3] == swap16(SOUND_INK));
            if (id == 1 && j != 1) {
                int rail = 22+18-(j ? 7 : 0);
                CHECK(cv_px[rail*240+5] == swap16(SOUND_INK));
                CHECK(cv_px[rail*240+17] == swap16(SOUND_INK));
            }
            if (id == 10 && j != 1) {
                int crossings = 0;
                for (int x = 3; x < 3+26; ++x)
                    crossings += cv_px[(22+9)*240+x] == swap16(SOUND_INK);
                CHECK(j ? crossings >= 10 : crossings == 0);
            }
        }
    }

    /* The filter knee and level meter follow this owner's sounding envelope.
     * Preserve the same control values between the idle and sounding frames. */
    int ids[] = {4, 15};
    uint16_t idle[2][26*19];
    pref_set(PR(R_ENG,T_303,owner,2), 32);
    pref_set(PR(R_ENG,T_303,owner,5), 0);
    pref_set(PR(R_ENG,T_303,owner,7), 127);
    render_blocks(1);
    for (int playing=0; playing<2; ++playing) {
        if (playing) {
            engine_bass_on_velocity(owner, 60, 0, 0, 100);
            render_blocks(8);
        }
        for (int k=0; k<2; ++k) {
            pref_t r=PR(R_ENG,T_303,owner,ids[k]);
            cv_begin(26,19,C_BLACK);
            orig_strip_glyph(r, matrix_value(r), 0, 0, SOUND_INK);
            if (playing) CHECK(memcmp(idle[k], cv_px, sizeof idle[k]));
            else memcpy(idle[k], cv_px, sizeof idle[k]);
        }
    }
    engine_bass_off(owner);
}

/* Traverse the shipping compact engine through SMK panel events. The four
 * factory cells use their own Q127 macros and share no original AU field IDs. */
static void check_panel_route(int owner)
{
    static const char *const presets[] = {"poly", "pad", "pluck", "brass"};
    static const char *const groups[] = {"oscillator", "filter", "envelope", "lfo-chorus"};
    static const unsigned ids[][4] = {{0,1,12,15}, {2,3,4,9}, {5,6,7,8}, {10,11,13,14}};
    setup(V_HOME, PART_303A + owner);
    proj.set.bt |= PROJ_BT_CLK_IN;
    panel_tap(B_EDIT);
    CHECK(ui.view == V_PART && ui.overlay == O_NONE);
    for (int n = 0; n < pg.n && pg.r[cur_page()][0].kind != R_BENGINE; ++n)
        panel_turn(EN_SELECT, 1);
    CHECK(pg.r[cur_page()][0].kind == R_BENGINE);
    int peer = 1 - owner;
    uint8_t peer_cell[ORIGINAL_SOUND_BYTES];
    memcpy(peer_cell, proj.sound.v[T_303][peer], sizeof peer_cell);
    for (int n = 0; n < 3 && engine_bass_bank(owner) != ORIGINAL_SOUND_JUNO; ++n) {
        panel_turn(EN_K1, 1);
        settle_selector(owner);
        CHECK(pg.r[cur_page()][0].kind == R_BENGINE);
    }
    CHECK(engine_bass_bank(owner) == ORIGINAL_SOUND_JUNO && pg.n == 5);
    CHECK(pref_desc(pg.r[cur_page()][1])->max == 3);
    for (int preset = 0; preset < 4; ++preset) {
        if (preset) {
            panel_turn(EN_PRESET, 1);
            settle_selector(owner);
        }
        CHECK(engine_bass_preset_index(owner) == preset);
        CHECK(!strcmp(matrix_preset_name(owner), original_sound_name(ORIGINAL_SOUND_JUNO, preset)));
        capture_route(owner, "sound", presets[preset]);
    }
    for (int page = 0; page < 4; ++page) {
        panel_turn(EN_SELECT, 1);
        CHECK(cur_page() == page);
        capture_route(owner, "matrix", groups[page]);
        for (int k = 0; k < 4; ++k) {
            pref_t r = pg.r[page][k];
            CHECK(r.kind == R_ENG && r.a == T_303 && r.b == owner && r.c == ids[page][k]);
            CHECK(!pref_read_only(r) && pref_desc(r)->max == 127 && *matrix_name(r));
            int before = pref_get(r), step = before < 127 ? 1 : -1;
            panel_turn(EN_K1 + k, step);
            render_blocks(1);
            CHECK(pref_get(r) == before + step);
            CHECK(proj.sound.v[T_303][owner][r.c] == before + step);
        }
    }
    panel_tap(B_SEL);
    CHECK(ui.overlay == O_LIST);
    unsigned seen = 0;
    for (int row = 0; row < 19; ++row) {
        pref_t r = list_rows[ui.list_sel];
        CHECK(r.b == owner);
        if (row < 16) {
            unsigned id = ids[row / 4][row % 4];
            CHECK(r.kind == R_ENG && r.a == T_303 && r.c == id);
            CHECK(!(seen & (1u << id)));
            seen |= 1u << id;
            if (row % 4 == 0) {
                capture_route(owner, "list", groups[row / 4]);
                int before = pref_get(r), step = before < 127 ? 1 : -1;
                panel_turn(EN_ALGO, step);
                render_blocks(1);
                CHECK(pref_get(r) == before + step);
            }
        } else {
            CHECK(r.kind == (row == 16 ? R_BENGINE : row == 17 ? R_BPRESET : R_BSTATUS));
            if (row == 16) capture_route(owner, "list", "sound");
            if (r.kind == R_BSTATUS) capture_route(owner, "list", "last");
        }
        if (row < 18) panel_turn(EN_SELECT, 1);
    }
    CHECK(seen == 0xffff && !memcmp(peer_cell, proj.sound.v[T_303][peer], sizeof peer_cell));
    panel_tap(B_HOME);
    CHECK(ui.overlay == O_NONE);
    CHECK(project_save() == 0);
    uint8_t saved[ORIGINAL_SOUND_BYTES];
    memcpy(saved, proj.sound.v[T_303][owner], sizeof saved);
    panel_turn(EN_K1, 1);
    render_blocks(1);
    CHECK(memcmp(saved, proj.sound.v[T_303][owner], sizeof saved));
    CHECK(project_load() == 0);
    engine_apply_sound(&proj.sound);
    render_blocks(1);
    CHECK(!memcmp(saved, proj.sound.v[T_303][owner], sizeof saved));
    CHECK(!memcmp(peer_cell, proj.sound.v[T_303][peer], sizeof peer_cell));
    check_compact_widgets(owner);
}

int main(void)
{
    setup(V_PART,PART_303A);
    for (int voice=0;voice<2;voice++) {
        int other=engine_bass_bank(1-voice);
        for (int preset=0;preset<4;preset++) {
            engine_bass_off(voice);
            CHECK(engine_bass_select_preset(voice,ORIGINAL_SOUND_JUNO,preset,&proj.sound)==0);
            for (int n=0;n<1024 && engine_bass_preset_pending(voice);n++) render_blocks(1);
            CHECK(!engine_bass_preset_pending(voice));
            CHECK(engine_bass_bank(voice)==ORIGINAL_SOUND_JUNO);
            CHECK(engine_bass_bank(1-voice)==other);
            CHECK(engine_bass_preset_index(voice)==preset);
            CHECK(engine_bass_sound_bank(&proj.sound,voice)==ORIGINAL_SOUND_JUNO);
            ui.part=(uint8_t)(PART_303A+voice);build_pages();
            CHECK(pg.n==UI_ORIGINAL_JUNO_PAGES+1);
            engine_bass_on_velocity(voice,60,0,0,100);
            render_blocks(32);
            float level,mod;unsigned state,down;
            CHECK(engine_bass_voice_view(voice,&level,&mod,&state,&down));
            CHECK(level>0.0f && down);
            engine_bass_off(voice);
            render_blocks(2048);
            CHECK(engine_bass_voice_view(voice,&level,&mod,&state,&down));
            CHECK(!down);
            ui.help=0;ui.msg_until=0;draw_main();draw_knobs();draw_footer();draw_header();
            CHECK(!strcmp(part_header_snapshot.name,voice?"SUPERJ6 B":"SUPERJ6 A"));
            shot(voice?"juno-b":"juno-a");
            ui.help=1;draw_header();shot(voice?"juno-b-help":"juno-a-help");ui.help=0;
        }
        CHECK(engine_bass_select_preset(voice,ORIGINAL_SOUND_CHICAGO,0,&proj.sound)==0);
        for(int n=0;n<1024 && engine_bass_preset_pending(voice);n++) render_blocks(1);
        CHECK(!engine_bass_preset_pending(voice));
        CHECK(engine_bass_bank(voice)==ORIGINAL_SOUND_CHICAGO);
    }
    check_panel_route(0);
    check_panel_route(1);
    printf("Juno slot integration: %u checks PASS\n",checks);return 0;
}

