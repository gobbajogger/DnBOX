/* SPDX-License-Identifier: GPL-3.0-only */
#define ARP_FIXTURE_MAIN retained_arp_main
#include "ui_arp_engine_test.c"

enum { FILTER_TYPE = 115, ADSR_TRACK = 149, FRAMES = 4096 };
static int32_t reference[FRAMES*2], changed[FRAMES*2];

enum { PROJECT_EXPORT, PROJECT_MOUNT, PROJECT_VERIFY };

static void stored_project(const char *stage, unsigned owner, unsigned action)
{
    const char *path = getenv("S1_FILTER_PROJECTS");
    if (!path) path = getenv("SMC_UI_SHOTS");
    CHECK(path);
    char name[1024];
    snprintf(name, sizeof name, "%s/%s-%u.bin", path, stage, owner);
    FILE *f = fopen(name, action == PROJECT_EXPORT ? "wb" : "rb");
    CHECK(f);
    const unsigned objects[] = {OBJ_SET, OBJ_SOUND, OBJ_PAT0, OBJ_PAT1, OBJ_SONG, OBJ_MOTION};
    uint8_t wire[PLAT_STORE_MAX], saved[PLAT_STORE_MAX];
    for (unsigned i = 0; i < sizeof objects / sizeof objects[0]; ++i) {
        if (action == PROJECT_EXPORT) {
            int bytes = plat_store_load(objects[i], wire, sizeof wire);
            CHECK(bytes >= 0 && fwrite(&bytes, sizeof bytes, 1, f) == 1);
            CHECK(fwrite(wire, 1, (unsigned)bytes, f) == (unsigned)bytes);
        } else {
            int bytes;
            CHECK(fread(&bytes, sizeof bytes, 1, f) == 1 && bytes >= 0 && (unsigned)bytes <= sizeof wire);
            CHECK(fread(wire, 1, (unsigned)bytes, f) == (unsigned)bytes);
            if (action == PROJECT_MOUNT) {
                CHECK(!plat_store_save(objects[i], wire, (unsigned)bytes));
            } else {
                CHECK(plat_store_load(objects[i], saved, sizeof saved) == bytes);
                CHECK(!memcmp(wire, saved, (unsigned)bytes));
            }
        }
    }
    if (action != PROJECT_EXPORT) CHECK(fgetc(f) == EOF);
    CHECK(!fclose(f));
}

static s1_patch_value read_slot(unsigned owner, unsigned id)
{
    s1_patch_value value;
    CHECK(engine_s1_patch_read(owner, id, &value));
    return value;
}

static void visit(const char *title)
{
    for (int i = 0; i < pg.n && strcmp(title, pg.title[cur_page()]); ++i) {
        now_ms += 180;
        enc_acc[EN_SELECT] = 1;
        input();
    }
    CHECK(!strcmp(title, pg.title[cur_page()]));
}

static void detent(unsigned col, int n)
{
    now_ms += 180;
    turn((int)col, n);
}

static void stabilize(void)
{
    engine_init(proj.pat, &proj.arr.song, proj.arr.lane, &proj.sound);
    engine_apply_sound(&proj.sound);
    render(24);
    engine_fx_mute(3u);
}

static void begin_pair(unsigned owner, unsigned note)
{
    stabilize();
    engine_bass_on(owner, note, 0, 0);
    engine_bass_on(1-owner, 55, 0, 0);
    render(4);
}

static void record(int32_t *pcm)
{
    for (unsigned i = 0; i < FRAMES; i += 256) engine_render(pcm + 2*i, 256);
}

static void own_pcm(unsigned owner)
{
    unsigned changed_frames = 0;
    for (unsigned i = 0; i < FRAMES; ++i) {
        changed_frames += reference[2*i+owner] != changed[2*i+owner];
        CHECK(reference[2*i+1-owner] == changed[2*i+1-owner]);
    }
    CHECK(changed_frames > FRAMES/2);
}

static double energy(const int32_t *pcm, unsigned owner)
{
    double total = 0;
    for (unsigned i = 0; i < FRAMES; ++i) {
        double sample = pcm[2*i+owner];
        total += sample * sample;
    }
    CHECK(total > 0.);
    return total;
}

static void select_patch(unsigned owner)
{
    CHECK(!engine_bass_select_preset(owner, ORIGINAL_SOUND_SYNTHONE, 0, &proj.sound));
    for (unsigned i = 0; i < 1024 && engine_bass_preset_pending(owner); ++i) render(1);
    CHECK(engine_bass_bank(owner) == ORIGINAL_SOUND_SYNTHONE);
}

static void assert_link_preserved(unsigned owner, const s1_patch_snapshot *base)
{
    s1_patch_snapshot after;
    CHECK(engine_s1_patch_snapshot(owner, &after));
    CHECK(after.filter_link == base->filter_link);
    CHECK(!memcmp(after.physical, base->physical, sizeof after.physical));
}

static void playback(unsigned owner)
{
    engine_note_lock_capture(1);
    CHECK(!engine_bass_note_capture(proj.arr.lane, 0, owner, 0, &proj.sound));
    engine_note_lock_capture(0);
    lane_t *lock = &proj.arr.lane[0];
    CHECK(lock->t == BASS_NOTE_TARGET && !engine_bass_note_validate(lock));
    lane_t invalid = *lock;
    invalid.val[BASS_NOTE_USED+11] = 3;
    CHECK(engine_bass_note_validate(&invalid));
    invalid = *lock;
    invalid.val[BASS_NOTE_USED+13] = 1;
    CHECK(engine_bass_note_validate(&invalid));

    visit("FILTER MODE");
    detent(0, -127);
    detent(1, -127);
    CHECK(read_slot(owner, FILTER_TYPE).position == 0 && read_slot(owner, ADSR_TRACK).position == 0);
    CHECK(!project_save() && !project_load());
    stabilize();
    bpart_t *line = &proj.pat[0].bass[owner];
    for (unsigned n = 0; n < NSTEPS; ++n) line->step[n].flags = G_REST;
    line->len = 16; line->rate = RATE_16; line->dir = DIR_FWD; line->transpose = 24;
    line->step[0].flags = line->step[4].flags = G_NOTE;
    line->step[0].note = 36; line->step[4].note = 43;
    line->step[0].velocity = line->step[4].velocity = 100;
    seq.ppat[TRK_BASS0+owner] = 0;
    engine_note_lock_refresh();
    seq_start(&seq);
    for (unsigned n = 0; n < 2000 && !read_slot(owner, FILTER_TYPE).note_locked; ++n) render(1);
    CHECK(read_slot(owner, FILTER_TYPE).note_locked && read_slot(owner, FILTER_TYPE).position == 2);
    CHECK(read_slot(owner, ADSR_TRACK).note_locked && read_slot(owner, ADSR_TRACK).position == 127);
    CHECK(engine_s1_patch_set(owner, read_slot(owner, ADSR_TRACK).revision, ADSR_TRACK, 0) == S1_EDIT_BUSY);
    for (unsigned n = 0; n < 2000 && read_slot(owner, FILTER_TYPE).note_locked; ++n) render(1);
    CHECK(!read_slot(owner, FILTER_TYPE).note_locked && read_slot(owner, FILTER_TYPE).position == 0);
    CHECK(!read_slot(owner, ADSR_TRACK).note_locked && read_slot(owner, ADSR_TRACK).position == 0);
    seq_stop(&seq);
    render(4);
}

static void journey(unsigned owner)
{
    setup(V_PART, PART_303A+owner);
    memset(proj.arr.lane, 0, sizeof proj.arr.lane);
    proj.set.bt |= PROJ_BT_CLK_IN;
    select_patch(owner);
    select_patch(1-owner);
    build_pages();
    CHECK(read_slot(owner, FILTER_TYPE).position == 0); /* Baseline lacks the editor. */
    CHECK(read_slot(owner, ADSR_TRACK).position == 0);
    CHECK(engine_s1_patch_set(owner, read_slot(owner, ADSR_TRACK).revision, ADSR_TRACK, 128) == S1_EDIT_REJECTED);
    CHECK(engine_s1_patch_set(owner, read_slot(owner, FILTER_TYPE).revision, FILTER_TYPE, 3) == S1_EDIT_REJECTED);
    proj.sound.v[T_MIX][PART_303A][MX_PAN] = 0;
    proj.sound.v[T_MIX][PART_303B][MX_PAN] = 127;
    for (unsigned v = 0; v < NBASS; ++v) {
        proj.sound.v[T_MIX][PART_303A+v][MX_REV] = 0;
        proj.sound.v[T_MIX][PART_303A+v][MX_DLY] = 0;
    }
    proj.sound.v[T_MST][0][MST_RATIO] = proj.sound.v[T_MST][0][MST_PUMP] = 0;
    proj.sound.v[T_MST][0][MST_LIMIT] = proj.sound.v[T_MST][0][MST_MODE] = 0;
    visit("FILTER ADSR");
    detent(0, 31);
    visit("FILTER");
    detent(3, 1);
    s1_patch_snapshot base;
    CHECK(engine_s1_patch_snapshot(owner, &base) && base.filter_link);
    visit("FILTER MODE");
    CHECK(!strcmp(pref_desc(pg.r[cur_page()][0])->name, "FILTER TYPE"));
    CHECK(!strcmp(pref_desc(pg.r[cur_page()][1])->name, "ADSR PITCH TRACK"));
    CHECK(pg.r[cur_page()][2].kind == R_NONE && pg.r[cur_page()][3].kind == R_NONE);
    if (!owner) { ui_frame(); shot("filter-low"); }

    uint8_t *extra = proj.sound.v[T_303][6+owner];
    uint8_t *peer = proj.sound.v[T_303][6+1-owner];
    extra[11] = 91; extra[12] = 92;
    CHECK(!engine_bass_sound_validate(&proj.sound));
    CHECK(engine_s1_patch_set(owner, read_slot(owner, FILTER_TYPE).revision, FILTER_TYPE, 1) == S1_EDIT_REJECTED);
    visit("OSCILLATOR");
    detent(2, -127);
    CHECK(extra[11] == 91 && extra[12] == 92);
    CHECK(!engine_bass_sound_validate(&proj.sound));
    engine_note_lock_capture(1);
    CHECK(!engine_bass_note_capture(proj.arr.lane, 0, owner, 7, &proj.sound));
    engine_note_lock_capture(0);
    CHECK(!proj.arr.lane[0].val[BASS_NOTE_USED+11] && !proj.arr.lane[0].val[BASS_NOTE_USED+12]);
    CHECK(!project_save());
    stored_project("oscillator", owner, PROJECT_EXPORT);
    memset(proj.arr.lane, 0, sizeof proj.arr.lane);
    stabilize();
    select_patch(owner);
    static const uint8_t empty[11];
    CHECK(!memcmp(extra, empty, sizeof empty));
    CHECK(extra[11] == 91 && extra[12] == 92);
    extra[11] = extra[12] = 0;
    peer[12] = 73;
    visit("FILTER ADSR");
    detent(0, 31);
    visit("FILTER");
    detent(3, 1);
    assert_link_preserved(owner, &base);

    visit("FILTER MODE");
    begin_pair(owner, 60);
    detent(0, 0);
    record(reference);
    begin_pair(owner, 60);
    detent(0, 1);
    record(changed);
    CHECK(read_slot(owner, FILTER_TYPE).position == 1);
    own_pcm(owner);
    if (!owner) shot("filter-band");
    begin_pair(owner, 60);
    detent(0, 0);
    record(reference);
    begin_pair(owner, 60);
    detent(0, 1);
    record(changed);
    CHECK(read_slot(owner, FILTER_TYPE).position == 2);
    own_pcm(owner);
    if (!owner) shot("filter-high");
    CHECK(peer[12] == 73);
    assert_link_preserved(owner, &base);

    const unsigned notes[] = {48, 84};
    double fraction[2];
    for (unsigned i = 0; i < 2; ++i) {
        detent(1, -127);
        begin_pair(owner, notes[i]);
        detent(1, 0);
        record(reference);
        begin_pair(owner, notes[i]);
        detent(1, 127);
        record(changed);
        CHECK(read_slot(owner, ADSR_TRACK).position == 127);
        own_pcm(owner);
        fraction[i] = energy(changed, owner) / energy(reference, owner);
    }
    CHECK(fraction[1] < fraction[0] && fraction[0] < 1.);
    if (!owner) shot("filter-track");
    assert_link_preserved(owner, &base);
    stabilize();
    CHECK(!project_save() && !project_load());
    stored_project("filter-base", owner, PROJECT_EXPORT);
    CHECK(read_slot(owner, FILTER_TYPE).position == 2 && read_slot(owner, ADSR_TRACK).position == 127);
    assert_link_preserved(owner, &base);
    visit("FILTER");
    detent(3, -1);
    s1_patch_snapshot independent;
    CHECK(engine_s1_patch_snapshot(owner, &independent) && !independent.filter_link);
    CHECK(!memcmp(independent.physical, base.physical, sizeof base.physical));
    playback(owner);
    stabilize();
    select_patch(owner);
    CHECK(read_slot(owner, FILTER_TYPE).position == 0 && read_slot(owner, ADSR_TRACK).position == 0);
    CHECK(peer[12] == 73);
    CHECK(!engine_bass_note_validate(&proj.arr.lane[0]));
    CHECK(!project_save() && !project_load());
    stored_project("filter-lock", owner, PROJECT_EXPORT);
    printf("PASS filter owner %u: all modes, tracking energy (%.6f / %.6f), linked ADSR, note locks and reload\n", owner, fraction[0], fraction[1]);
}

int main(void)
{
#ifdef S1_FILTER_LEGACY
    setup(V_PART, PART_303A);
    for (unsigned owner = 0; owner < NBASS; ++owner) {
        stored_project("oscillator", owner, PROJECT_MOUNT);
        CHECK(!project_load() && !project_storage_unsupported());
        const uint8_t *extra = proj.sound.v[T_303][6+owner];
        CHECK(extra[11] == 91 && extra[12] == 92);
        CHECK(!engine_bass_note_validate(&proj.arr.lane[0]));
        const char *const unsupported[] = {"filter-base", "filter-lock"};
        for (unsigned i = 0; i < sizeof unsupported / sizeof unsupported[0]; ++i) {
            stored_project(unsupported[i], owner, PROJECT_MOUNT);
            CHECK(project_load() == -9 && project_storage_unsupported());
            CHECK(project_save() == -9);
            stored_project(unsupported[i], owner, PROJECT_VERIFY);
        }
    }
    printf("PASS previous editor reads oscillator projects and preserves unsupported filter Sound/lock projects, %u checks\n", checks);
#else
    journey(0);
    journey(1);
    printf("PASS Synth One filter mode and ADSR pitch tracking %u checks\n", checks);
#endif
    return 0;
}
