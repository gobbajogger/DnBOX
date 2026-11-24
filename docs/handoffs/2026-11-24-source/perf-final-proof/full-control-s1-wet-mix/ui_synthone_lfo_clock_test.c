/* SPDX-License-Identifier: GPL-3.0-only */
#include "lfo-test-helpers.h"
#include "../../firmware/src/app/s1c_part.h"
#include "../../firmware/src/dsp/s1c/s1c_front.h"
#include "../../firmware/src/dsp/s1pack/s1pack.h"

static const unsigned clock_ids[] = {15, 47, 36, 43, 139, 64};
static float expected[2][sizeof clock_ids / sizeof clock_ids[0]];
static unsigned observed[2], watch_owner;
static int watching, forced_wet_words;
static uint8_t pack[0xf000];
static float raw[150];

void host_lfo_capture(unsigned owner, const float *left, const float *right, unsigned n)
{ (void)owner; (void)left; (void)right; (void)n; }

void host_s1_clock_peek(unsigned owner, float target[6]);

/* Separate from the authentic-factory pass. Introduce controlled off-grid
 * words after decoding so the known rollback bug cannot be masked. */
void host_clock_witness_words(s1c *engine, unsigned program)
{
    if (forced_wet_words && program != S1_PART_INIT && engine->t[139] == 0.f) {
        CHECK(s1c_front_set(engine, 64, 120.f));
        CHECK(s1c_front_set(engine, 36, .73f));
        CHECK(s1c_front_set(engine, 43, .11f));
    }
}

/* Called at the original compact voice render boundary, before the first
 * sample of every slice. Once enabled, neither state may contain a mixture. */
void host_s1_clock_watch(unsigned owner, const float *targets, unsigned locked)
{
    if (!watching || owner != watch_owner) return;
    CHECK(locked <= 1);
    for (unsigned i = 0; i < sizeof clock_ids / sizeof clock_ids[0]; ++i) {
        if (targets[clock_ids[i]] != expected[locked][i])
            fprintf(stderr, "clock owner %u lock %u factory ID %u got %a expected %a\n",
                    owner, locked, clock_ids[i], targets[clock_ids[i]], expected[locked][i]);
        CHECK(targets[clock_ids[i]] == expected[locked][i]);
    }
    ++observed[locked];
}

static void word(void *context, unsigned id, float value)
{ (void)context; raw[id] = value; }

static unsigned scene(void)
{
    for (unsigned i = 0; i < 694; ++i) {
        char name[S1PACK_NAME_MAX];
        memset(raw, 0, sizeof raw);
        CHECK(s1pack_read(pack, i, name, word, NULL) >= 0);
        if (raw[139] == 0.f && raw[35] > 0.f && raw[41] > 0.f) return i+1;
    }
    CHECK(0 && "free-running factory with pan and delay enabled");
    return 0;
}

static void choose_clock_preset(unsigned owner, unsigned preset)
{
    CHECK(!engine_bass_select_preset(owner, ORIGINAL_SOUND_SYNTHONE, preset, &proj.sound));
    for (unsigned n = 0; n < 1024 && engine_bass_preset_pending(owner); ++n) render(1);
    CHECK(!engine_bass_preset_pending(owner) && engine_bass_preset_index(owner) == (int)preset);
    build_pages();
}

static void expected_transition(const float prior[6], float desired, float next[6])
{
    static s1c oracle;
    s1c_init(&oracle, 48000.f, NULL);
    for (unsigned i = 0; i < 6; ++i) s1c_set(&oracle, clock_ids[i], prior[i]);
    CHECK(s1c_front_set(&oracle, 139, desired));
    for (unsigned i = 0; i < 6; ++i) next[i] = oracle.t[clock_ids[i]];
}

static void clock_journey(unsigned owner, unsigned preset)
{
    setup(V_PART, PART_303A+owner);
    memset(proj.arr.lane, 0, sizeof proj.arr.lane);
    proj.set.bt |= PROJ_BT_CLK_IN;
    choose_clock_preset(owner, preset);
    choose_clock_preset(1-owner, 0);
    CHECK(read_slot(owner, 139).position == 0); /* Rates-only candidate red. */
    visit("LFO");
    detent(0, 17);
    detent(2, 29);
    host_s1_clock_peek(owner, expected[0]);
    float peer[6], current[6], off[6];
    host_s1_clock_peek(1-owner, peer);
    expected_transition(expected[0], 1.f, expected[1]);
    visit("TEMPO SYNC");
    CHECK(!strcmp(pref_desc(pg.r[cur_page()][0])->name, "SYNC TO ARP"));
    detent(0, 1);
    host_s1_clock_peek(owner, current);
    CHECK(!memcmp(current, expected[1], sizeof current));
    if (forced_wet_words) CHECK(current[2] != expected[0][2] && current[3] != expected[0][3]);
    if (!forced_wet_words) {
        CHECK(!project_save());
        stored_project("sync-base", owner, PROJECT_EXPORT);
    }
    engine_note_lock_capture(1);
    CHECK(!engine_bass_note_capture(proj.arr.lane, 0, owner, 0, &proj.sound));
    engine_note_lock_capture(0);
    CHECK(!engine_bass_note_validate(&proj.arr.lane[0]));

    expected_transition(current, 0.f, off);
    detent(0, -1);
    host_s1_clock_peek(owner, current);
    CHECK(!memcmp(current, off, sizeof current));
    CHECK(!project_save() && !project_load());
    host_s1_clock_peek(owner, current);
    CHECK(!memcmp(current, off, sizeof current));

    choose_clock_preset(owner, preset);
    visit("LFO");
    detent(0, 17);
    detent(2, 29);
    host_s1_clock_peek(owner, current);
    CHECK(!memcmp(current, expected[0], sizeof current));
    CHECK(!project_save() && !project_load());
    if (!forced_wet_words) stored_project("sync-lock", owner, PROJECT_EXPORT);
    stabilize();

    bpart_t *line = &proj.pat[0].bass[owner];
    for (unsigned n = 0; n < NSTEPS; ++n) line->step[n].flags = G_REST;
    line->len = 16; line->rate = RATE_16; line->dir = DIR_FWD; line->transpose = 24;
    line->step[0].flags = line->step[4].flags = G_NOTE;
    line->step[0].note = 36; line->step[4].note = 43;
    line->step[0].velocity = line->step[4].velocity = 100;
    seq.ppat[TRK_BASS0+owner] = 0;
    engine_note_lock_refresh();
    watch_owner = owner;
    memset(observed, 0, sizeof observed);
    watching = 1;
    seq_start(&seq);
    for (unsigned n = 0; n < 2000 && !read_slot(owner, 139).note_locked; ++n) render(1);
    CHECK(read_slot(owner, 139).note_locked && read_slot(owner, 139).position == 1);
    CHECK(engine_s1_patch_set(owner, read_slot(owner, 139).revision, 139, 0) == S1_EDIT_BUSY);
    for (unsigned n = 0; n < 2000 && read_slot(owner, 139).note_locked; ++n) render(1);
    CHECK(!read_slot(owner, 139).note_locked && read_slot(owner, 139).position == 0);
    CHECK(observed[0] && observed[1]);
    seq_stop(&seq);
    render(4);
    seq_start(&seq);
    for (unsigned n = 0; n < 2000 && !read_slot(owner, 139).note_locked; ++n) render(1);
    CHECK(read_slot(owner, 139).note_locked);
    seq_stop(&seq);
    render(4);
    CHECK(!read_slot(owner, 139).note_locked);
    host_s1_clock_peek(owner, current);
    CHECK(!memcmp(current, expected[0], sizeof current));
    watching = 0;
    host_s1_clock_peek(1-owner, current);
    CHECK(!memcmp(current, peer, sizeof current));
    printf("PASS clock owner %u, %u base and %u locked render boundaries checked\n",
           owner, observed[0], observed[1]);
}

int main(void)
{
    const char *path = getenv("S1_EDITOR_TEST_PACK");
    CHECK(path);
    FILE *f = fopen(path, "rb");
    CHECK(f);
    unsigned bytes = (unsigned)fread(pack, 1, sizeof pack, f);
    CHECK(fgetc(f) == EOF && !fclose(f) && s1_part_set_pack(pack, bytes) == 694);
#if defined(S1_CLOCK_LEGACY) || defined(S1_CLOCK_IMPORT_RATE)
    setup(V_PART, PART_303A);
    for (unsigned owner = 0; owner < NBASS; ++owner) {
#ifdef S1_CLOCK_LEGACY
        const char *const entries[] = {"sync-base", "sync-lock"};
#else
        const char *const entries[] = {"lfo-base", "lfo-lock"};
#endif
        for (unsigned i = 0; i < 2; ++i) {
            stored_project(entries[i], owner, PROJECT_MOUNT);
#ifdef S1_CLOCK_LEGACY
            const uint32_t writes = store_writes;
            CHECK(project_load() == -9 && project_storage_unsupported());
            CHECK(project_save() == -9);
            CHECK(store_writes == writes);
#else
            CHECK(!project_load() && !project_storage_unsupported());
            CHECK(!project_save());
#endif
            stored_project(entries[i], owner, PROJECT_VERIFY);
        }
    }
#ifdef S1_CLOCK_LEGACY
    printf("PASS previous reader preserves unsupported global sync objects, %u checks\n", checks);
#else
    printf("PASS sync reader accepts rates-only Sound and lock objects byte-for-byte, %u checks\n", checks);
#endif
#else
    forced_wet_words = getenv("S1_LFO_ROLLBACK_WITNESS") != NULL;
    unsigned preset = scene();
    for (unsigned owner = 0; owner < NBASS; ++owner) clock_journey(owner, preset);
    if (getenv("S1_LFO_CAPTURE_INIT")) {
        setup(V_PART, PART_303A);
        choose_clock_preset(0, 0);
        visit("TEMPO SYNC");
        if (!read_slot(0, 139).position) detent(0, 1);
        CHECK(read_slot(0, 139).position);
        ui_frame();
        shot("s1-global-sync-on");
        detent(0, -1);
        CHECK(!read_slot(0, 139).position);
        ui_frame();
        shot("s1-global-sync-off");
    }
    printf("PASS original Synth One tempo sync transaction %u checks\n", checks);
#endif
    return 0;
}
