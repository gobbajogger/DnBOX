/* SPDX-License-Identifier: GPL-3.0-only */
#include "lfo-test-helpers.h"
#include "../../firmware/src/app/s1c_part.h"
#include "../../firmware/src/dsp/s1pack/s1pack.h"

static const unsigned ids[] = {15, 13, 47, 45};
static uint8_t pack[0xf000];
static float raw[150];
static float capture[2][NBASS][FRAMES][2];
static unsigned counts[NBASS];
static int active_take = -1;

void host_lfo_capture(unsigned owner, const float *left, const float *right, unsigned n)
{
    if (active_take < 0) return;
    for (unsigned i = 0; i < n && counts[owner] < FRAMES; ++i) {
        float *sample = capture[active_take][owner][counts[owner]++];
        sample[0] = left[i];
        sample[1] = right[i];
    }
}

static void lfo_take(unsigned take)
{
    memset(counts, 0, sizeof counts);
    active_take = (int)take;
    record(take ? changed : reference);
    active_take = -1;
    for (unsigned owner = 0; owner < NBASS; ++owner) CHECK(counts[owner] == FRAMES);
}

static void check_lfo_audio(unsigned owner)
{
    unsigned distinct = 0;
    for (unsigned n = 0; n < FRAMES; ++n)
        distinct += memcmp(capture[0][owner][n], capture[1][owner][n], sizeof capture[0][owner][n]) != 0;
    CHECK(distinct > FRAMES/2);
    CHECK(!memcmp(capture[0][1-owner], capture[1][1-owner], sizeof capture[0][1-owner]));
}

static void keep_word(void *context, unsigned id, float value)
{
    (void)context;
    raw[id] = value;
}

static unsigned find_factory(unsigned sync)
{
    for (unsigned index = 0; index < 694; ++index) {
        char name[S1PACK_NAME_MAX];
        memset(raw, 0, sizeof raw);
        CHECK(s1pack_read(pack, index, name, keep_word, NULL) >= 0);
        if (raw[139] != (float)sync || raw[14] < .1f || raw[46] < .1f || raw[20] != 0.f)
            continue;
        if ((raw[48] == 1.f && raw[57] == 2.f) || (raw[48] == 2.f && raw[57] == 1.f))
            return index+1;
    }
    CHECK(0 && "authenticated factory with both modulation routes");
    return 0;
}

static void choose_lfo_preset(unsigned owner, unsigned preset)
{
    CHECK(!engine_bass_select_preset(owner, ORIGINAL_SOUND_SYNTHONE, preset, &proj.sound));
    for (unsigned n = 0; n < 1024 && engine_bass_preset_pending(owner); ++n) render(1);
    CHECK(!engine_bass_preset_pending(owner) && engine_bass_preset_index(owner) == (int)preset);
    build_pages();
}

static void setup_levels(void)
{
    proj.sound.v[T_MIX][PART_303A][MX_PAN] = 0;
    proj.sound.v[T_MIX][PART_303B][MX_PAN] = 127;
    for (unsigned v = 0; v < NBASS; ++v) {
        proj.sound.v[T_MIX][PART_303A+v][MX_REV] = 0;
        proj.sound.v[T_MIX][PART_303A+v][MX_DLY] = 0;
    }
    proj.sound.v[T_MST][0][MST_RATIO] = proj.sound.v[T_MST][0][MST_PUMP] = 0;
    proj.sound.v[T_MST][0][MST_LIMIT] = proj.sound.v[T_MST][0][MST_MODE] = 0;
}

static void verify_targets(unsigned owner, const float expected[4], int locked)
{
    for (unsigned i = 0; i < 4; ++i) {
        s1_patch_value got = read_slot(owner, ids[i]);
        CHECK(got.physical == expected[i] && got.note_locked == locked);
    }
}

static void play_lfo_lock(unsigned owner, const float edited[4], const float base[4])
{
    bpart_t *line = &proj.pat[0].bass[owner];
    for (unsigned n = 0; n < NSTEPS; ++n) line->step[n].flags = G_REST;
    line->len = 16; line->rate = RATE_16; line->dir = DIR_FWD; line->transpose = 24;
    line->step[0].flags = line->step[4].flags = G_NOTE;
    line->step[0].note = 36; line->step[4].note = 43;
    line->step[0].velocity = line->step[4].velocity = 100;
    seq.ppat[TRK_BASS0+owner] = 0;
    engine_note_lock_refresh();
    seq_start(&seq);
    for (unsigned n = 0; n < 2000 && !read_slot(owner, 15).note_locked; ++n) render(1);
    verify_targets(owner, edited, 1);
    CHECK(engine_s1_patch_set(owner, read_slot(owner, 15).revision, 15, 0) == S1_EDIT_BUSY);
    for (unsigned n = 0; n < 2000 && read_slot(owner, 15).note_locked; ++n) render(1);
    verify_targets(owner, base, 0);
    seq_stop(&seq);
    render(4);
}

static void lfo_journey(unsigned owner, unsigned sync, unsigned preset)
{
    setup(V_PART, PART_303A+owner);
    memset(proj.arr.lane, 0, sizeof proj.arr.lane);
    proj.set.bt |= PROJ_BT_CLK_IN;
    choose_lfo_preset(owner, preset);
    choose_lfo_preset(1-owner, 0);
    setup_levels();
    float initial[4], edited[4], base[4], peer[4];
    for (unsigned i = 0; i < 4; ++i) {
        initial[i] = read_slot(owner, ids[i]).physical;
        peer[i] = read_slot(1-owner, ids[i]).physical;
    }
    visit("LFO");
    static const char *const names[] = {"LFO 1 RATE", "LFO 1 SHAPE", "LFO 2 RATE", "LFO 2 SHAPE"};
    for (unsigned i = 0; i < 4; ++i) {
        const x0x_param_t *d = pref_desc(pg.r[cur_page()][i]);
        CHECK(d && !strcmp(d->name, names[i]));
    }
    if (!sync) {
        char number[32] = {0}, unit[12] = {0};
        unsigned sub_hz = 0;
        for (unsigned i = 0; i < 4; i += 2) if (initial[i] < 1.f) {
            pref_t ref = pg.r[cur_page()][i];
            pref_value_of(ref, pref_get(ref), number, unit);
            CHECK(!strcmp(unit, "mHz") && strcmp(number, "0"));
            ++sub_hz;
        }
        CHECK(sub_hz);
    }
    ui_frame();
    if (!owner) shot("s1-lfo-free");
    if (sync) shot("s1-lfo-synced");
    for (unsigned i = 0; i < 4; ++i) {
        const unsigned max = i & 1u ? 3 : sync ? 18 : 127;
        unsigned position = read_slot(owner, ids[i]).position;
        CHECK(position <= max);
        CHECK(engine_s1_patch_set(owner, read_slot(owner, ids[i]).revision, ids[i], max+1) == S1_EDIT_REJECTED);
        begin_pair(owner, 60);
        detent(i, 0);
        lfo_take(0);
        begin_pair(owner, 60);
        detent(i, position > max/2 ? -(int)max : (int)max);
        lfo_take(1);
        edited[i] = read_slot(owner, ids[i]).physical;
        CHECK(edited[i] != initial[i]);
        check_lfo_audio(owner);
        verify_targets(1-owner, peer, 0);
    }
    if (!owner) shot("s1-lfo-edited");
    stabilize();
    CHECK(!project_save() && !project_load());
    verify_targets(owner, edited, 0);
    stored_project("lfo-base", owner, PROJECT_EXPORT);
    engine_note_lock_capture(1);
    CHECK(!engine_bass_note_capture(proj.arr.lane, 0, owner, 0, &proj.sound));
    engine_note_lock_capture(0);
    CHECK(!engine_bass_note_validate(&proj.arr.lane[0]));
    lane_t invalid = proj.arr.lane[0];
    invalid.val[BASS_NOTE_USED+13] = invalid.val[BASS_NOTE_USED+14] = 0;
    CHECK(engine_bass_note_validate(&invalid));

    visit("LFO");
    for (unsigned i = 0; i < 4; ++i) {
        s1_patch_value prior = read_slot(owner, ids[i]);
        detent(i, prior.position ? -127 : 127);
        base[i] = read_slot(owner, ids[i]).physical;
    }
    CHECK(!project_save() && !project_load());
    stabilize();
    play_lfo_lock(owner, edited, base);
    stabilize();
    choose_lfo_preset(owner, preset);
    verify_targets(owner, initial, 0);
    verify_targets(1-owner, peer, 0);
    CHECK(!project_save() && !project_load());
    stored_project("lfo-lock", owner, PROJECT_EXPORT);
    puts("PASS physical LFO owner edits, held PCM isolation, save and note-lock recovery");
}

int main(void)
{
    const char *path = getenv("S1_EDITOR_TEST_PACK");
    CHECK(path);
    FILE *f = fopen(path, "rb");
    CHECK(f);
    unsigned bytes = (unsigned)fread(pack, 1, sizeof pack, f);
    CHECK(fgetc(f) == EOF && !fclose(f) && s1_part_set_pack(pack, bytes) == 694);
#ifdef S1_LFO_LEGACY
    setup(V_PART, PART_303A);
    for (unsigned owner = 0; owner < NBASS; ++owner) {
        const char *const cases[] = {"lfo-base", "lfo-lock"};
        for (unsigned i = 0; i < 2; ++i) {
            stored_project(cases[i], owner, PROJECT_MOUNT);
            const uint32_t writes = store_writes;
            CHECK(project_load() == -9 && project_storage_unsupported());
            CHECK(project_save() == -9);
            CHECK(store_writes == writes);
            stored_project(cases[i], owner, PROJECT_VERIFY);
        }
    }
    printf("PASS previous reader refuses LFO Sound and lock records without storage changes, %u checks\n", checks);
#else
    for (unsigned owner = 0; owner < NBASS; ++owner)
        lfo_journey(owner, owner, find_factory(owner));
    printf("PASS independent Synth One LFO rates and shapes %u checks\n", checks);
#endif
    return 0;
}
