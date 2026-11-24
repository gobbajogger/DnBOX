/* SPDX-License-Identifier: GPL-3.0-only */
#define ARP_FIXTURE_MAIN retained_arp_main
#include "ui_arp_engine_test.c"

enum object_operation { EXPORT_OBJECT, MOUNT_OBJECT, VERIFY_OBJECT };

static void copy_object(const char *directory, const char *name, enum object_operation operation)
{
    char namebuf[1024];
    uint8_t bytes[4 + sizeof(sound_t)];
    snprintf(namebuf, sizeof namebuf, "%s/%s", directory, name);
    if (operation != EXPORT_OBJECT) {
        FILE *f = fopen(namebuf, "rb");
        CHECK(f && fread(bytes, sizeof bytes, 1, f) == 1 && fgetc(f) == EOF && !fclose(f));
        if (operation == MOUNT_OBJECT) {
            CHECK(!plat_store_save(OBJ_SOUND, bytes, sizeof bytes));
        } else {
            uint8_t saved[sizeof bytes];
            CHECK(plat_store_load(OBJ_SOUND, saved, sizeof saved) == (int)sizeof saved);
            CHECK(!memcmp(saved, bytes, sizeof saved));
        }
    } else {
        CHECK(plat_store_load(OBJ_SOUND, bytes, sizeof bytes) == (int)sizeof bytes);
        FILE *f = fopen(namebuf, "wb");
        CHECK(f && fwrite(bytes, sizeof bytes, 1, f) == 1 && !fclose(f));
    }
}

#if X0X_S1_EXTRA
enum { FRAMES = 4096 };
static int32_t lead[FRAMES*2], tail[FRAMES*2];

static void choose(unsigned owner, unsigned bank)
{
    CHECK(!engine_bass_select_preset(owner, bank, 0, &proj.sound));
    for (unsigned n = 0; n < 1024 && engine_bass_preset_pending(owner); ++n) render(1);
    CHECK(!engine_bass_preset_pending(owner) && engine_bass_bank(owner) == (int)bank);
}

static void visit(unsigned owner, const char *title)
{
    ui.part = PART_303A+owner;
    set_view(V_PART);
    build_pages();
    for (int i = 0; i < pg.n && strcmp(title, pg.title[cur_page()]); ++i) {
        now_ms += 180;
        enc_acc[EN_SELECT] = 1;
        input();
    }
    CHECK(!strcmp(title, pg.title[cur_page()]));
}

static void knob_turn(unsigned col, int delta)
{
    now_ms += 180;
    turn((int)col, delta);
}

static void stabilize(void)
{
    engine_init(proj.pat, &proj.arr.song, proj.arr.lane, &proj.sound);
    engine_apply_sound(&proj.sound);
    render(24);
    engine_fx_mute(3u);
}

static void begin_pair(void)
{
    stabilize();
    engine_bass_on(0, 57, 0, 0);
    engine_bass_on(1, 62, 0, 0);
    render(4);
}

static void record(int32_t *pcm)
{
    for (unsigned i = 0; i < FRAMES; i += 256) engine_render(pcm+2*i, 256);
}

static void expect_owned_pcm(unsigned owner)
{
    unsigned changed[2] = {0};
    for (unsigned i = 0; i < FRAMES*2; ++i) changed[i&1] += lead[i] != tail[i];
    printf("held edit owner=%u changed %u own frames, %u peer frames\n",
           owner, changed[owner], changed[1-owner]);
    CHECK(changed[owner] > FRAMES/2 && changed[1-owner] == 0);
}

static lane_t *find_lock(unsigned owner)
{
    for (unsigned i = 0; i < NLANE; ++i) {
        lane_t *l = &proj.arr.lane[i];
        if (l->used && l->t == BASS_NOTE_TARGET && l->part == PART_303A+owner) return l;
    }
    return NULL;
}

static void add_motion(void)
{
    lane_t *l = &proj.arr.lane[NLANE-1];
    memset(l, 0, sizeof *l);
    l->used = 1; l->t = T_MIX; l->part = PART_303A; l->v = PART_303A; l->i = MX_PAN;
    memset(l->val, MOT_NONE, sizeof l->val);
    l->val[3] = 32;
}

static void set_arp(unsigned owner)
{
    engine_arp_settings cfg;
    CHECK(engine_arp_settings_read(owner, &cfg));
    CHECK(engine_arp_set(owner, cfg.revision, ARP_F_GATE, owner ? 71 : 64) == S1_EDIT_QUEUED);
    render(1);
}

static void capture_locked(unsigned s1, unsigned sj6)
{
    engine_note_lock_capture(1);
    CHECK(!engine_bass_note_capture(proj.arr.lane, 0, s1, 0, &proj.sound));
    CHECK(!engine_bass_note_capture(proj.arr.lane, 0, sj6, 0, &proj.sound));
    engine_note_lock_capture(0);
    lane_t *a = find_lock(s1), *b = find_lock(sj6);
    CHECK(a && b);
    CHECK(!engine_bass_note_validate(a) && !engine_bass_note_validate(b));
    CHECK(!memcmp(a->val+BASS_NOTE_USED, proj.sound.v[T_303][S1_EXTRA_ROW+s1], S1_EXTRA_BYTES));
    CHECK(!memcmp(b->val+BASS_NOTE_USED, proj.sound.v[T_303][ORIGINAL_SJ6_EXTRA_ROW+sj6], ORIGINAL_SJ6_EXTRA_SIZE));
}

static s1_patch_value read_morph(unsigned owner)
{
    s1_patch_value value;
    CHECK(engine_s1_patch_read(owner, 0, &value));
    return value;
}

static void prove_playback(unsigned s1, unsigned sj6)
{
    s1_patch_value old = read_morph(s1);
    int old_sj6 = engine_superj6_knob_position(sj6, SJ6_LFO2_PWM_DEPTH);
    visit(s1, "OSCILLATOR");
    knob_turn(2, 127);
    visit(sj6, "LFO2 PWM");
    knob_turn(1, -37);
    s1_patch_value base = read_morph(s1);
    int base_sj6 = engine_superj6_knob_position(sj6, SJ6_LFO2_PWM_DEPTH);
    CHECK(old.physical != base.physical && old_sj6 != base_sj6);
    for (unsigned v = 0; v < NBASS; ++v) {
        bpart_t *line = &proj.pat[0].bass[v];
        for (unsigned n = 0; n < NSTEPS; ++n) line->step[n].flags = G_REST;
        line->len = 16; line->rate = RATE_16; line->dir = DIR_FWD; line->transpose = 24;
        line->step[0].flags = line->step[4].flags = G_NOTE;
        line->step[0].note = 36; line->step[4].note = 43;
        line->step[0].velocity = line->step[4].velocity = 100;
        seq.ppat[TRK_BASS0+v] = 0;
    }
    engine_note_lock_refresh();
    engine_motion_enable(1);
    seq_start(&seq);
    for (unsigned n = 0; n < 2000 && !read_morph(s1).note_locked; ++n) render(1);
    s1_patch_value locked = read_morph(s1);
    CHECK(locked.note_locked && locked.physical == old.physical);
    CHECK(engine_superj6_knob_position(sj6, SJ6_LFO2_PWM_DEPTH) == old_sj6);
    CHECK(engine_s1_patch_set(s1, base.revision, 0, 0) == S1_EDIT_BUSY);
    unsigned heard_motion = 0;
    for (unsigned n = 0; n < 2000 && read_morph(s1).note_locked; ++n) {
        render(1);
        if (engine_motion_value(NLANE-1) == 32) heard_motion = 1;
    }
    CHECK(heard_motion);
    s1_patch_value restored = read_morph(s1);
    CHECK(!restored.note_locked && restored.physical == base.physical);
    CHECK(engine_superj6_knob_position(sj6, SJ6_LFO2_PWM_DEPTH) == base_sj6);
    seq_stop(&seq);
    render(4);
}

static void journey(unsigned s1, const char *out)
{
    unsigned sj6 = 1-s1;
    char legacy[24], mixed[24];
    snprintf(legacy, sizeof legacy, "legacy-%u.obj", s1);
    snprintf(mixed, sizeof mixed, "mixed-%u.obj", s1);
    setup(V_PART, PART_303A+s1);
    memset(proj.arr.lane, 0, sizeof proj.arr.lane);
    engine_note_lock_refresh();
    proj.set.bt |= PROJ_BT_CLK_IN;
    choose(s1, ORIGINAL_SOUND_SYNTHONE);
    choose(sj6, ORIGINAL_SOUND_JUNO);
    memset(proj.sound.v[T_303][S1_EXTRA_ROW+s1], 0, S1_EXTRA_BYTES);
    uint8_t opaque[S1_EXTRA_BYTES] = {0x80u, 0x80u, 0x91u};
    memcpy(proj.sound.v[T_303][S1_EXTRA_ROW+sj6], opaque, sizeof opaque);
    proj.sound.v[T_MIX][PART_303A][MX_PAN] = 0;
    proj.sound.v[T_MIX][PART_303B][MX_PAN] = 127;
    for (unsigned v = 0; v < 2; ++v) {
        proj.sound.v[T_MIX][PART_303A+v][MX_REV] = 0;
        proj.sound.v[T_MIX][PART_303A+v][MX_DLY] = 0;
    }
    proj.sound.v[T_MST][0][MST_RATIO] = proj.sound.v[T_MST][0][MST_PUMP] = 0;
    proj.sound.v[T_MST][0][MST_LIMIT] = proj.sound.v[T_MST][0][MST_MODE] = 0;
    stabilize();
    visit(sj6, "OSCILLATOR");
    CHECK(pg.r[cur_page()][1].c == SJ6_WAVE2);
    knob_turn(1, 3);
    visit(sj6, "PULSE WIDTH");
    CHECK(pg.r[cur_page()][1].c == SJ6_PWM1);
    knob_turn(1, 28);
    const uint8_t *sj = proj.sound.v[T_303][sj6];
    CHECK(original_sound_edit_mask(sj) & S1_EXTRA_MARK); /* This is SJ6 PWM1's bit. */
    CHECK(!engine_bass_sound_validate(&proj.sound));
    CHECK(original_sound_s1_extra_valid(sj, opaque)); /* Red on raw-bit recognition. */

    CHECK(pg.r[cur_page()][2].c == SJ6_PWM2_ON);
    knob_turn(2, 1);
    visit(sj6, "LFO2 PWM");
    knob_turn(0, 41);
    knob_turn(1, 37);
    knob_turn(2, 1);
    if (pref_get(pg.r[cur_page()][2]) == 1) knob_turn(2, 1);
    CHECK(pref_get(pg.r[cur_page()][2]) == 2);
    knob_turn(3, 32);
    CHECK(original_sound_edit_mask(sj) & ORIGINAL_SJ6_LFO2_PWM2_BIT);
    if (!s1) shot("join-superj6-pwm2");
    visit(sj6, "LFO MODE");
    knob_turn(1, -1);
    if (!s1) shot("join-superj6-sync");
    CHECK(pref_get(pg.r[cur_page()][1]) == 0);
    set_arp(s1);
    set_arp(sj6);
    CHECK(!project_save());
    copy_object(out, legacy, EXPORT_OBJECT);

    sound_t old = proj.sound;
    begin_pair();
    visit(s1, "OSCILLATOR");
    knob_turn(2, 0);
    record(lead);
    begin_pair();
    visit(s1, "OSCILLATOR");
    knob_turn(2, -127);
    record(tail);
    expect_owned_pcm(s1);
    CHECK(!memcmp(old.v[T_303][sj6], proj.sound.v[T_303][sj6], NPARAMS_MAX));
    CHECK(!memcmp(old.v[T_303][ORIGINAL_SJ6_EXTRA_ROW+sj6],
                  proj.sound.v[T_303][ORIGINAL_SJ6_EXTRA_ROW+sj6], NPARAMS_MAX));
    CHECK(!memcmp(opaque, proj.sound.v[T_303][S1_EXTRA_ROW+sj6], sizeof opaque));
    CHECK(!memcmp(old.v[T_303][ARP_BLOCK_VOICE0], proj.sound.v[T_303][ARP_BLOCK_VOICE0], 2*NPARAMS_MAX));

    sound_t osc = proj.sound;
    begin_pair();
    visit(sj6, "LFO2 PWM");
    knob_turn(1, 0);
    record(lead);
    begin_pair();
    visit(sj6, "LFO2 PWM");
    knob_turn(1, 19);
    record(tail);
    expect_owned_pcm(sj6);
    CHECK(!memcmp(osc.v[T_303][S1_EXTRA_ROW+s1],
                  proj.sound.v[T_303][S1_EXTRA_ROW+s1], S1_EXTRA_BYTES));
    stabilize();
    capture_locked(s1, sj6);
    add_motion();
    project_capture_stage();
    uint8_t header[32];
    project_file_header(header);
    CHECK(!project_file_validate(header));
    for (unsigned v = 0; v < NBASS; ++v) {
        lane_t *l = &project_stage.arr.lane[NLANE-1];
        l->t = T_303; l->v = S1_EXTRA_ROW+v; l->part = PART_303A+v;
        project_file_header(header);
        CHECK(project_file_validate(header));
    }

    sound_t saved = proj.sound;
    arrange_t before_arrange = proj.arr;
    CHECK(!project_save() && !project_load());
    CHECK(!memcmp(&saved, &proj.sound, sizeof saved));
    CHECK(!memcmp(&before_arrange, &proj.arr, sizeof before_arrange));
    copy_object(out, mixed, EXPORT_OBJECT);
    stabilize();
    prove_playback(s1, sj6);
    proj.sound = saved;
    stabilize();
    memset(proj.arr.lane, 0, sizeof proj.arr.lane);
    engine_note_lock_refresh();
    choose(s1, ORIGINAL_SOUND_JUNO);
    static const uint8_t empty[S1_EXTRA_BYTES];
    CHECK(!memcmp(empty, proj.sound.v[T_303][S1_EXTRA_ROW+s1], S1_EXTRA_BYTES));
    CHECK(!memcmp(saved.v[T_303][sj6], proj.sound.v[T_303][sj6], NPARAMS_MAX));
    CHECK(!memcmp(saved.v[T_303][ORIGINAL_SJ6_EXTRA_ROW+sj6],
                  proj.sound.v[T_303][ORIGINAL_SJ6_EXTRA_ROW+sj6], NPARAMS_MAX));
    choose(sj6, ORIGINAL_SOUND_SYNTHONE);
    CHECK(!memcmp(opaque, proj.sound.v[T_303][S1_EXTRA_ROW+sj6], sizeof opaque));
    s1_patch_value value;
    CHECK(engine_s1_patch_read(sj6, 0, &value));
    CHECK(engine_s1_patch_set(sj6, value.revision, 0, 0) == S1_EDIT_REJECTED);
    CHECK(!memcmp(opaque, proj.sound.v[T_303][S1_EXTRA_ROW+sj6], sizeof opaque));
    memset(proj.sound.v[T_303][S1_EXTRA_ROW+sj6], 0, sizeof opaque);
    printf("PASS mixed owner %u: typed bit 22, shared ARP, locks, motion, project and cross-bank ownership\n", s1);
}
#endif

int main(void)
{
    const char *out = getenv("SMC_UI_SHOTS");
    const char *objects = getenv("S1_JOIN_OBJECTS");
#if X0X_S1_EXTRA
    CHECK(out && !objects);
    journey(0, out);
    journey(1, out);
#else
    CHECK(objects);
    setup(V_PART, PART_303A);
    proj.set.bt |= PROJ_BT_CLK_IN;
    CHECK(!project_save());
    for (unsigned owner = 0; owner < 2; ++owner) {
        char name[24];
        snprintf(name, sizeof name, "legacy-%u.obj", owner);
        copy_object(objects, name, MOUNT_OBJECT);
        CHECK(!project_load() && !project_storage_unsupported());
        const uint8_t opaque[] = {0x80u, 0x80u, 0x91u};
        CHECK(!memcmp(opaque, proj.sound.v[T_303][6+(1-owner)], sizeof opaque));
        snprintf(name, sizeof name, "mixed-%u.obj", owner);
        copy_object(objects, name, MOUNT_OBJECT);
        CHECK(project_load() == -9 && project_storage_unsupported());
        CHECK(project_save() == -9);
        copy_object(objects, name, VERIFY_OBJECT);
    }
    puts("PASS exact old checkpoint loads unmarked mixed sounds and rejects edited records without overwriting them");
#endif
    printf("PASS Synth One and SuperJ6 storage seam %u checks\n", checks);
    return 0;
}
