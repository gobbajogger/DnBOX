/* SPDX-License-Identifier: GPL-3.0-only */
/* Exercise Synth One in the production slot selector and header labels. */
#define SMC_UI_MAIN retained_fixture_main
#define plat_midi_mark host_midi_mark
#include "smc_ui_test.c"
#undef plat_midi_mark

/* The production engine reads the host MIDI watermark between the odd and
 * even publication counters. Observe one real in-progress request there. */
static void (*during_publication)(void);
uint32_t plat_midi_mark(void)
{
    if (during_publication) {
        void (*observe)(void) = during_publication;
        during_publication = 0;
        observe();
    }
    return host_midi_mark();
}
static void render_blocks(unsigned count)
{
    int32_t audio[512];
    while (count--) engine_render(audio,256);
}
#if !X0X_SYNTHONE_PATCH
/* Compare ink in the completed framebuffer with independently composed labels,
 * readouts and pictograms. A missing or misplaced item fails its own row. */
static void selector_row(int row, const char *label, const char *value, int glyph, int selected)
{
    int y = 24 + row * 67;
    uint16_t ink = selected ? SOUND_INK : SOUND_WHITE;
    uint16_t face = selected ? SOUND_SELECT : SOUND_FACE;
    cv_begin(218, 60, face);
    cv_text(0, 0, &FONT_XS, label, ink);
    const felucca_font_t *font = tw(&FONT_M, value) <= 187 ? &FONT_M : &FONT_B;
    CHECK(tw(font, value) <= 187);
    text_r(217, base_y(&FONT_M,35,font), font, value, ink);
    ui_pictogram(91, 19, glyph, 2, ink);
    for (int dy = 0; dy < 60; ++dy) {
        int equal = !memcmp(fb + (y + 3 + dy) * 240 + 12, cv_px + dy * 218, 218 * sizeof(*fb));
        if (!equal) fprintf(stderr, "Stock INIT row %s, value %s, mismatched line %d\n",label,value,dy);
        CHECK(equal);
    }
}

static void selector_frame(const char *bank, const char *preset, const char *status, int locked)
{
    ui_frame();
    selector_row(0, "ENGINE", bank, UI_G_SOUND, 1);
    selector_row(1, "PRESET", preset, locked ? UI_G_LOCK : UI_G_LIST, 0);
    selector_row(2, "STATUS", status, !strcmp(status,"READY") ? UI_G_CHECK : UI_G_HOLD, 0);
    /* Number positions follow the sixteen physical white-key steps in bank 1. */
    uint16_t background = fb[230 * 240 + 239];
    for (int k = 0; k < 16; ++k) {
        char number[4];
        snprintf(number, sizeof number, "%d", k+1);
        cv_begin(14, 13, swap16(background));
        text_c(7, 0, &FONT_T, number, SOUND_GRAY);
        int pixels = 0, mismatch = 0;
        for (int y = 0; y < 13; ++y)
            for (int x = 0; x < 14; ++x) {
                uint16_t expected = cv_px[y * 14 + x];
                if (expected == background) continue;
                ++pixels;
                mismatch += fb[(224+y) * 240 + 7 + k*14 + x] != expected;
            }
        CHECK(pixels && !mismatch);
    }
}

static void publishing_frame(void)
{
    CHECK(engine_bass_preset_pending(bidx()) == 6);
    selector_frame("SYNTH ONE", "UPDATING", "WAIT", 1);
    shot(bidx() ? "stock-init-updating-b" : "stock-init-updating-a");
}

static void widest_factory(int voice)
{
    int longest = 0, which_bank = -1, which_preset = -1;
    for (unsigned bank = ORIGINAL_SOUND_CHICAGO; bank <= ORIGINAL_SOUND_MIAMI; ++bank)
        for (unsigned i = 0; i < original_sound_factories(bank); ++i) {
            const char *name = original_sound_name(bank,i);
            int width = tw(&FONT_M,name);
            if (width > longest) { longest = width; which_bank = bank; which_preset = i; }
        }
    CHECK(longest > 187 && which_preset >= 0);
    const char *name = original_sound_name(which_bank,which_preset);
    char caption[FACTORY_NAME_MAX];
    size_t count = strlen(name);
    CHECK(count < sizeof caption);
    for (size_t i = 0; i <= count; ++i) {
        char c = name[i];
        caption[i] = c >= 'a' && c <= 'z' ? (char)(c-'a'+'A') : c;
    }
    CHECK(!engine_bass_select_preset(voice,which_bank,which_preset,&proj.sound));
    CHECK(engine_bass_preset_pending(voice) == 2);
    selector_frame(which_bank == ORIGINAL_SOUND_CHICAGO ? "CHICAGO" : "MIAMI", caption, "HELD", 0);
    printf("Longest admitted stock name: %s (%d px at medium size)\n",name,longest);
    shot(voice ? "stock-init-long-name-b" : "stock-init-long-name-a");
}

static void physical_detent(int column, int delta)
{
    now_ms += 128;
    enc_acc[EN_K1 + column] = delta;
    ui_input_only();
}

static void short_list(int voice)
{
    const char *const names[] = {"crush", "engine", "preset", "status"};
    const int kinds[] = {R_ENG, R_BENGINE, R_BPRESET, R_BSTATUS};
    button_tap(B_SEL);
    CHECK(ui.overlay == O_LIST && ui.list_n == 4);
    for (int row = 0; row < 4; ++row) {
        if (row) {
            now_ms += 128;
            enc_acc[EN_SELECT] = 1;
            ui_input_only();
        }
        pref_t r = list_rows[row];
        CHECK(ui.list_top == 0 && ui.list_sel == row && r.kind == kinds[row] && r.b == voice);
        if (row == 0) {
            CHECK(r.c == ORIGINAL_SYNTHONE_CRUSH && !strcmp(pref_name(r), "CRUSH RATE"));
            char number[16], unit[8];
            pref_value(r, number, unit);
            CHECK(!strcmp(number, "48000") && !strcmp(unit, "Hz"));
        }
        ui_frame();
        if (row) { /* Preserve the three archived selector captures. */
            char name[64];
            snprintf(name, sizeof name, "stock-init-list-%s-%c", names[row], 'a' + voice);
            shot(name);
        }
    }
    button_tap(B_HOME);
    CHECK(ui.overlay == O_NONE && ui.view == V_PART && bidx() == voice && cur_page() == 1);
    selector_frame("SYNTH ONE", "INIT", "READY", 1);
}

static void stock_selector(int voice)
{
    CHECK(pg.n == 2 && !strcmp(pg.title[0], "EFFECT") && !strcmp(pg.title[1], "SOUND"));
    *page_of(V_PART) = 1;
    for (int col = 0; col < 3; ++col) CHECK(pg.r[1][col].b == voice);
    CHECK(pg.r[1][0].kind == R_BENGINE && pg.r[1][1].kind == R_BPRESET);
    CHECK(pg.r[1][2].kind == R_BSTATUS && pg.r[1][3].kind == R_NONE);
    CHECK(!pref_read_only(pg.r[1][0]) && pref_read_only(pg.r[1][1]) && pref_read_only(pg.r[1][2]));
    uint32_t serial = engine_bass_choice_serial();
    for (int col = 1; col < 4; ++col) physical_detent(col, 1);
    CHECK(engine_bass_choice_serial() == serial && !engine_bass_preset_pending(voice));
    selector_frame("SYNTH ONE", "INIT", "READY", 1);
    shot(voice ? "stock-init-ready-b" : "stock-init-ready-a");
    short_list(voice);

    /* K5 changes the requested bank while a held Synth One note retains its
     * applied owner. K6 now selects the pending bank's real factory preset. */
    engine_bass_on_velocity(voice,60,0,0,100);
    render_blocks(4);
    widest_factory(voice);
    engine_bass_cancel_preset(voice);
    during_publication = publishing_frame;
    physical_detent(0, 1);
    CHECK(!during_publication);
    CHECK(engine_bass_bank(voice) == ORIGINAL_SOUND_SYNTHONE);
    CHECK(engine_bass_requested_bank(voice) == ORIGINAL_SOUND_JUNO);
    CHECK(engine_bass_preset_pending(voice) == 2);
    selector_frame("SUPERJ6", "POLY", "HELD", 0);
    physical_detent(1, 1);
    CHECK(engine_bass_requested_preset(voice) == 1);
    selector_frame("SUPERJ6", "PAD", "HELD", 0);
    shot(voice ? "stock-init-pending-b" : "stock-init-pending-a");
    engine_bass_off(voice);
    for (int n = 0; n < 1024 && engine_bass_preset_pending(voice); ++n) render_blocks(1);
    ui_input_only();
    CHECK(engine_bass_bank(voice) == ORIGINAL_SOUND_JUNO && engine_bass_preset_index(voice) == 1);
    CHECK(pg.n > 1 && cur_page() == pg.n-1 && pg.r[cur_page()][0].kind == R_BENGINE);

    /* Returning from the longer bank must keep the actual SOUND control
     * selected and clamp the former last page to the stock INIT page. */
    physical_detent(0,-1);
    for (int n = 0; n < 1024 && engine_bass_preset_pending(voice); ++n) render_blocks(1);
    ui_input_only();
    CHECK(engine_bass_bank(voice) == ORIGINAL_SOUND_SYNTHONE);
    CHECK(pg.n == 2 && *page_of(V_PART) == 1 && cur_page() == 1);
    selector_frame("SYNTH ONE", "INIT", "READY", 1);
}
#endif

int main(void)
{
    setup(V_PART,PART_303A);
    for (int voice=0;voice<2;voice++) {
        int other=engine_bass_bank(1-voice);
        engine_bass_off(voice);
        CHECK(engine_bass_select_preset(voice,ORIGINAL_SOUND_SYNTHONE,0,&proj.sound)==0);
        for (int n=0;n<1024 && engine_bass_preset_pending(voice);n++) render_blocks(1);
        CHECK(!engine_bass_preset_pending(voice));
        CHECK(engine_bass_bank(voice)==ORIGINAL_SOUND_SYNTHONE);
        CHECK(engine_bass_bank(1-voice)==other);
        CHECK(engine_bass_preset_index(voice)==0);
        CHECK(engine_bass_sound_bank(&proj.sound,voice)==ORIGINAL_SOUND_SYNTHONE);
        ui.part=(uint8_t)(PART_303A+voice);build_pages();
#if X0X_ARP_HOOKS
        CHECK(pg.n == UI_ORIGINAL_SYNTHONE_PAGES + 2);
        CHECK(!strcmp(pg.title[UI_ORIGINAL_SYNTHONE_PAGES - 1], "ARPEGGIATOR"));
#else
        CHECK(pg.n == UI_ORIGINAL_SYNTHONE_PAGES + 1);
#endif
        CHECK(!strcmp(pg.title[pg.n - 1], "SOUND"));
#if !X0X_SYNTHONE_PATCH
        stock_selector(voice);
        CHECK(engine_bass_bank(1-voice)==other);
#endif
        engine_bass_on_velocity(voice,60,0,0,100);
        render_blocks(32);
        float level,mod;unsigned state,down;
        CHECK(engine_bass_voice_view(voice,&level,&mod,&state,&down));
        CHECK(down);
        engine_bass_off(voice);
        render_blocks(32);
        CHECK(engine_bass_voice_view(voice,&level,&mod,&state,&down));
        CHECK(!down);
        ui.help=0;ui.msg_until=0;draw_main();draw_knobs();draw_footer();draw_header();
        CHECK(!strcmp(part_header_snapshot.name,voice?"S1 B":"S1 A"));
        shot(voice?"synthone-b":"synthone-a");
        ui.help=1;draw_header();shot(voice?"synthone-b-help":"synthone-a-help");ui.help=0;
        CHECK(engine_bass_select_preset(voice,ORIGINAL_SOUND_CHICAGO,0,&proj.sound)==0);
        for(int n=0;n<1024 && engine_bass_preset_pending(voice);n++) render_blocks(1);
        CHECK(!engine_bass_preset_pending(voice));
        CHECK(engine_bass_bank(voice)==ORIGINAL_SOUND_CHICAGO);
    }
#if X0X_S1C
    /* High presets must not alias to low ones, switch engine, or lose their
     * exact ID when the project and typed undo restore the saved cell. */
    for (int voice = 0; voice < 2; ++voice) {
        ui.part = (uint8_t)(PART_303A + voice);
        CHECK(engine_bass_select_preset(voice, ORIGINAL_SOUND_SYNTHONE, 0, &proj.sound) == 0);
        for (int n=0;n<1024 && engine_bass_preset_pending(voice);n++) render_blocks(1);
        CHECK(engine_bass_bank(voice) == ORIGINAL_SOUND_SYNTHONE);
        for (int preset = 128; preset <= 694; preset += 566) {
            pref_t r = PR(R_BPRESET,0,voice,0);
            pref_set(r, preset);
            CHECK(engine_bass_requested_bank(voice) == ORIGINAL_SOUND_SYNTHONE);
            CHECK(engine_bass_requested_preset(voice) == preset);
            for (int n=0;n<1024 && engine_bass_preset_pending(voice);n++) render_blocks(1);
            CHECK(engine_bass_preset_index(voice) == preset);
            CHECK(original_sound_s1_program(proj.sound.v[T_303][voice]) == (unsigned)preset);
            CHECK(engine_bass_sound_validate(&proj.sound) == 0);
            engine_bass_base_restore(1u << voice);
            CHECK(engine_bass_preset_index(voice) == preset);
        }
    }
#endif
    printf("Synth One slot integration: %u checks PASS\n",checks);return 0;
}
