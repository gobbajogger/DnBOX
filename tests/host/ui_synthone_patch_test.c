/* SPDX-License-Identifier: GPL-3.0-only */
/* Real SMK input roles, production UI, project storage and both audio owners. */
#define SMC_UI_MAIN existing_smc_fixture_main
#include "smc_ui_test.c"

static void select_bank(unsigned part, unsigned bank)
{
    CHECK(engine_bass_select_preset(part, bank, 0, &proj.sound) == 0);
    for (unsigned n = 0; n < 1000 && engine_bass_preset_pending(part); n++)
        audio_block();
    CHECK(!engine_bass_preset_pending(part));
    CHECK(engine_bass_bank(part) == (int)bank);
}

static void capture(unsigned part, int page, int knob, const char *stage)
{
    char file[72];
    snprintf(file, sizeof file, "bass-%c-page-%d-k%d-%s", 'A' + part,
             page + 1, knob < 0 ? 0 : UI_FIRST_KNOB + knob, stage);
    draw_header();
    draw_main();
    draw_knobs();
    shot(file);
}

static void check_readout(pref_t ref, const s1_patch_descriptor *d, float physical)
{
    char n[24], u[12], expected[24] = {0}, unit[12] = {0};
    unit_value(d->unit, s1_patch_display(d, physical), expected, unit);
    pref_value(ref, n, u);
    CHECK(!strcmp(n, expected) && !strcmp(u, unit));
    CHECK(!strcmp(pref_name(ref), d->label));
}

static void check_part(unsigned owner)
{
    static const unsigned ids[] = {2,5,16,17,19,22,23,24,25,26,27,28,29};
    static const char *const titles[] = {"OSCILLATOR", "FILTER", "FILTER ADSR", "AMP ADSR", "SOUND"};
    int visited = 0;
    ui.part = (uint8_t)(PART_303A + owner);
    set_view(V_PART);
    ui_input_only(); /* Observe current bank before turning a physical control. */
    CHECK(pg.n == 5);
    for (int p = 0; p < pg.n; ++p) {
        *page_of(V_PART) = (uint8_t)p;
        CHECK(!strcmp(pg.title[p], titles[p]));
        ui.touched = -1;
        ui.msg_until = 0;
        capture(owner, p, -1, "overview");
        for (int k = 0; k < 4; ++k) {
            pref_t ref = pg.r[p][k];
            if (ref.kind == R_NONE) continue;
            if (p == 4) {
                CHECK(k < 3 && ref.b == owner);
                CHECK(ref.kind == (k == 0 ? R_BENGINE : k == 1 ? R_BPRESET : R_BSTATUS));
                continue;
            }
            CHECK(ref.kind == R_S1PATCH && ref.b == owner && ref.c == visited);
            const s1_patch_descriptor *d = s1_patch_at(ref.c);
            CHECK(d && d->factory_id == ids[visited++]);
            CHECK(pref_desc(ref)->max == S1_PATCH_STEPS && !pref_read_only(ref));
            if (p < 2) CHECK(tw(&FONT_T, matrix_name(ref)) <= 55);
            s1_patch_value before, peer, after, peer_after;
            CHECK(engine_s1_patch_read(owner, d->factory_id, &before));
            CHECK(engine_s1_patch_read(1-owner, d->factory_id, &peer));
            check_readout(ref, d, before.physical);
            sound_t saved = proj.sound;
            ui.touched = -1;
            now_ms += 1000;
            capture(owner, p, k, "before");
            /* Use multiple slow physical detents so the displayed scale moves
             * visibly even where one q127 increment rounds to the same label. */
            int delta = before.position < 64 ? 12 : -12;
            for (int i = 0; i < 12; ++i) {
                now_ms += 100;
                enc_acc[EN_K1 + k] = delta < 0 ? -1 : 1;
                ui_input_only();
                CHECK(engine_s1_patch_pending());
                if (!i) {
                    CHECK(engine_s1_patch_read(owner, d->factory_id, &after));
                    CHECK(after.physical == before.physical);
                    CHECK(project_save() == -8);
                }
                audio_block();
            }
            CHECK(engine_s1_patch_read(owner, d->factory_id, &after));
            CHECK(after.revision == before.revision && after.position == before.position + delta);
            CHECK(after.physical == s1_patch_physical(d, after.position));
            CHECK(engine_s1_patch_read(1-owner, d->factory_id, &peer_after));
            CHECK(peer_after.physical == peer.physical && peer_after.revision == peer.revision);
            CHECK(!memcmp(saved.v[T_303][1-owner], proj.sound.v[T_303][1-owner], ORIGINAL_SOUND_BYTES));
            CHECK(!memcmp(&saved.bass[1-owner], &proj.sound.bass[1-owner], sizeof saved.bass[0]));
            check_readout(ref, d, after.physical);
            capture(owner, p, k, "touched");
            now_ms += 1200;
            capture(owner, p, k, "applied");
        }
    }
    CHECK(visited == S1_PATCH_COUNT);
    *page_of(V_PART) = 0;
    button_tap(B_SEL);
    CHECK(ui.overlay == O_LIST);
    int seen = 0;
    for (int i = 0; i < ui.list_n; ++i) {
        pref_t r = list_rows[i];
        if (r.kind == R_S1PATCH) {
            CHECK(r.b == owner && r.c == seen++);
            s1_patch_value v;
            CHECK(s1_read(r, &v));
            check_readout(r, s1_patch_at(r.c), v.physical);
        }
    }
    CHECK(seen == S1_PATCH_COUNT);
    button_tap(B_HOME);
}

static void simultaneous_detents(void)
{
    ui.part = PART_303A;
    set_view(V_PART);
    ui_input_only();
    *page_of(V_PART) = 0;
    const pref_t left = pg.r[0][0], right = pg.r[0][1];
    s1_patch_value a, b, peer, value;
    CHECK(s1_read(left, &a) && s1_read(right, &b));
    CHECK(engine_s1_patch_read(1, 2, &peer));
    now_ms += 1000;
    enc_acc[EN_K1] = 1;
    enc_acc[EN_K1+1] = -1;
    ui_input_only();
    CHECK(!strcmp(ui.msg, "EDIT BUSY - RETRY"));
    CHECK(s1_read(left, &value) && value.physical == a.physical);
    CHECK(s1_read(right, &value) && value.physical == b.physical);
    pref_set(PR(R_S1PATCH,T_303,1,0), peer.position - 1);
    audio_block();
    CHECK(s1_read(left, &value) && value.position == a.position + 1);
    CHECK(s1_read(right, &value) && value.physical == b.physical);
    CHECK(engine_s1_patch_read(1, 2, &value) && value.position == peer.position - 1);
    now_ms += 100;
    enc_acc[EN_K1+1] = -1;
    ui_input_only();
    audio_block();
    CHECK(s1_read(right, &value) && value.position == b.position - 1);
}

int main(void)
{
    setup(V_PART, PART_303A);
    /* The SMC-only fixture replaces the BT byte. Keep its role and restore
     * the shipping clock-input bit before exercising complete project I/O. */
    proj.set.bt |= PROJ_BT_CLK_IN;
    CHECK(project_validate_for(&proj) == 0);
    select_bank(0, ORIGINAL_SOUND_SYNTHONE);
    select_bank(1, ORIGINAL_SOUND_SYNTHONE);
    simultaneous_detents();
    check_part(0);
    check_part(1);
    CHECK(project_validate_for(&proj) == 0);
    CHECK(project_save() == 0);
    sound_t saved = proj.sound;
    s1_patch_value original, fresh;
    CHECK(engine_s1_patch_read(0, 16, &original));
    CHECK(engine_s1_patch_set(0, original.revision, 16, 0) == S1_EDIT_QUEUED);
    select_bank(0, ORIGINAL_SOUND_SYNTHONE);
    CHECK(engine_s1_patch_read(0, 16, &fresh) && fresh.physical == 2000.f);
    CHECK(fresh.revision != original.revision);
    CHECK(engine_s1_patch_set(0, original.revision, 16, 0) == S1_EDIT_REJECTED);
    check_readout(PR(R_S1PATCH,T_303,0,2), s1_patch_at(2), 2000.f);
    CHECK(engine_s1_patch_set(0, fresh.revision, 16, 0) == S1_EDIT_QUEUED);
    CHECK(project_load() == 0);
    engine_apply_sound(&proj.sound);
    audio_block();
    CHECK(!memcmp(&saved, &proj.sound, sizeof saved));
    CHECK(engine_s1_patch_set(0, fresh.revision, 16, 0) == S1_EDIT_REJECTED);
    for (unsigned part = 0; part < 2; ++part)
        for (unsigned i = 0; i < S1_PATCH_COUNT; ++i) {
            pref_t ref = PR(R_S1PATCH, T_303, part, i);
            s1_patch_value v;
            CHECK(s1_read(ref,&v));
            check_readout(ref,s1_patch_at(i),v.physical);
        }
    select_bank(0, ORIGINAL_SOUND_MIAMI);
    CHECK(!s1_read(PR(R_S1PATCH,T_303,0,2), &fresh));
    CHECK(engine_s1_patch_read(1, 16, &fresh));
    printf("Synth One INIT F7: %u checks, two owners, 13 controls each, 88 captures\n", checks);
    return 0;
}
