/* SPDX-License-Identifier: GPL-3.0-only */
#include "lfo-test-helpers.h"
#include "../../firmware/src/app/s1c_part.h"

enum { DELAY_MIX = 44, REVERB_MIX = 40 };
static const unsigned wet_ids[] = {DELAY_MIX, REVERB_MIX};
static uint8_t pack[0xf000];
static float takes[2][NBASS][FRAMES][2];
static unsigned take_frames[NBASS];
static int record_take = -1;

void host_lfo_capture(unsigned owner, const float *left, const float *right, unsigned frames)
{
    if (record_take < 0) return;
    for (unsigned i = 0; i < frames && take_frames[owner] < FRAMES; ++i) {
        float *sample = takes[record_take][owner][take_frames[owner]++];
        sample[0] = left[i];
        sample[1] = right[i];
    }
}

/* Test-only reads from the actual runtime owner. */
void host_s1_wet_peek(unsigned owner, float enables[2], unsigned layout[5]);

static void take(unsigned n)
{
    memset(take_frames, 0, sizeof take_frames);
    record_take = (int)n;
    record(n ? changed : reference);
    record_take = -1;
    for (unsigned owner = 0; owner < NBASS; ++owner) CHECK(take_frames[owner] == FRAMES);
}

static void choose(unsigned owner, unsigned preset)
{
    CHECK(!engine_bass_select_preset(owner, ORIGINAL_SOUND_SYNTHONE, preset, &proj.sound));
    for (unsigned n = 0; n < 1024 && engine_bass_preset_pending(owner); ++n) render(1);
    CHECK(!engine_bass_preset_pending(owner));
    CHECK(engine_bass_bank(owner) == ORIGINAL_SOUND_SYNTHONE);
    CHECK(engine_bass_preset_index(owner) == (int)preset);
    build_pages();
}

static unsigned program_for(unsigned enabled)
{
    static s1_dry_part part;
    for (unsigned index = 0; index < 694; ++index) {
        CHECK(s1_part_program(&part, index));
        float delay = part.e.t[S1C_P_DLON], verb = part.e.t[S1C_P_RVON];
        if (delay == (float)enabled && verb == (float)enabled &&
            (!enabled || part.e.t[S1C_P_DLT] < .15f)) return index+1;
    }
    CHECK(0 && "authentic factory case with matching wet enables");
    return 0;
}

static void edits_on_held(unsigned owner, unsigned enabled)
{
    const float flag[] = {(float)enabled, (float)enabled};
    for (unsigned effect = 0; effect < 2; ++effect) {
        const unsigned knob = 1+effect;
        const unsigned id = wet_ids[effect];
        const float before = read_slot(owner, id).physical;
        const float peer = read_slot(1-owner, id).physical;
        const float other = read_slot(owner, wet_ids[1-effect]).physical;
        float enables[2];
        unsigned allocation[5], edited[5];
        begin_pair(owner, 57);
        render(48);
        detent(knob, 0);
        host_s1_wet_peek(owner, enables, allocation);
        CHECK(!memcmp(enables, flag, sizeof flag));
        take(0);
        begin_pair(owner, 57);
        render(48);
        detent(knob, before < .5f ? 127 : -127);
        host_s1_wet_peek(owner, enables, edited);
        CHECK(!memcmp(enables, flag, sizeof flag));
        CHECK(!memcmp(edited, allocation, sizeof edited));
        take(1);
        CHECK(read_slot(owner, id).physical == (before < .5f ? 1.f : 0.f));
        CHECK(read_slot(owner, wet_ids[1-effect]).physical == other);
        CHECK(read_slot(1-owner, id).physical == peer);
        CHECK(!memcmp(takes[0][1-owner], takes[1][1-owner], sizeof takes[0][0]));
        unsigned differences = 0;
        for (unsigned n = 0; n < FRAMES; ++n)
            differences += memcmp(takes[0][owner][n], takes[1][owner][n], sizeof takes[0][owner][n]) != 0;
        printf("owner %u MIX %u enable %u: %u/%u owner frames changed, peer exact\n",
               owner, id, enabled, differences, FRAMES);
        CHECK(enabled ? differences > FRAMES/2 : !differences);
    }
}

static void set_existing(unsigned owner, unsigned id, unsigned position)
{
    CHECK(engine_s1_patch_set(owner, read_slot(owner, id).revision, id, position) == S1_EDIT_QUEUED);
    render(1);
}

static void verify_slot_set(unsigned owner, const unsigned *ids, const float *values, unsigned count, int locked)
{
    for (unsigned i = 0; i < count; ++i) {
        s1_patch_value value = read_slot(owner, ids[i]);
        CHECK(value.physical == values[i] && value.note_locked == locked);
    }
}

static void play_lock(unsigned owner, const float *locked, const float *base)
{
    bpart_t *line = &proj.pat[0].bass[owner];
    for (unsigned i = 0; i < NSTEPS; ++i) line->step[i].flags = G_REST;
    line->len = 16; line->rate = RATE_16; line->dir = DIR_FWD; line->transpose = 24;
    line->step[0].flags = line->step[4].flags = G_NOTE;
    line->step[0].note = 36; line->step[4].note = 43;
    line->step[0].velocity = line->step[4].velocity = 100;
    seq.ppat[TRK_BASS0+owner] = 0;
    engine_note_lock_refresh();
    seq_start(&seq);
    for (unsigned n = 0; n < 2000 && !read_slot(owner, DELAY_MIX).note_locked; ++n) render(1);
    verify_slot_set(owner, wet_ids, locked, 2, 1);
    CHECK(engine_s1_patch_set(owner, read_slot(owner, REVERB_MIX).revision, REVERB_MIX, 31) == S1_EDIT_BUSY);
    for (unsigned n = 0; n < 2000 && read_slot(owner, DELAY_MIX).note_locked; ++n) render(1);
    verify_slot_set(owner, wet_ids, base, 2, 0);
    seq_stop(&seq);
    render(4);
    seq_start(&seq);
    for (unsigned n = 0; n < 2000 && !read_slot(owner, DELAY_MIX).note_locked; ++n) render(1);
    verify_slot_set(owner, wet_ids, locked, 2, 1);
    seq_stop(&seq);
    render(4);
    verify_slot_set(owner, wet_ids, base, 2, 0);
}


#if !defined(S1_WET_LEGACY) && !defined(S1_WET_RED)
static void codec_golden(void)
{
    uint8_t cell[ORIGINAL_SOUND_BYTES], data[S1_EXTRA_BYTES] = {0};
    CHECK(original_sound_init(cell, ORIGINAL_SOUND_SYNTHONE, 0));
    for (unsigned slot = S1_EXTRA_FIRST; slot < S1_LFO_FIRST; ++slot)
        CHECK(original_sound_s1_extra_set(cell, data, slot, s1_patch_steps(slot)));
    for (unsigned slot = S1_LFO_FIRST; slot < S1_LFO_SYNC; ++slot)
        CHECK(original_sound_s1_extra_set(cell, data, slot,
                                         s1_lfo_rate_slot(slot) ? S1_LFO_RATE_CODES-1u : 4u));
    CHECK(original_sound_s1_sync_set(cell, data, 2, (const uint8_t[]){0, 0}));
    CHECK(data[0] || data[1]);
    CHECK(original_sound_s1_extra_set(cell, data, S1_DELAY_MIX, 128));
    CHECK(original_sound_s1_extra_set(cell, data, S1_REVERB_MIX, 128));
    const uint8_t golden[] = {0,0,0,0x41,0x80,0x20,0x40,0x10,0x20,0xa8,0xbf,0x44,0x64,0x80,0x20};
    CHECK(!memcmp(data, golden, sizeof golden));
    CHECK(original_sound_s1_extra_valid(cell, data));
    s1_extra_state state, saved;
    CHECK(s1_extra_read(data, &state));
    for (unsigned slot = S1_EXTRA_FIRST; slot < S1_LFO_FIRST; ++slot)
        CHECK(state.panel[slot-S1_EXTRA_FIRST] == s1_patch_steps(slot)+1u);
    CHECK(state.lfo[0] == 147 && state.lfo[1] == 147 && state.lfo[2] == 4 &&
          state.lfo[3] == 4 && state.lfo[4] == 2 && state.wet[0] == 128 && state.wet[1] == 128);
    saved = state;
    data[sizeof data-1] |= 0x80u;
    CHECK(!s1_extra_read(data, &state) && !memcmp(&state, &saved, sizeof saved));
}

static void opaque_suffix(unsigned owner)
{
    uint8_t *extra = proj.sound.v[T_303][6+owner];
    uint8_t saved[S1_EXTRA_BYTES];
    set_existing(owner, FILTER_TYPE, 1);
    CHECK(s1_extra_size(extra) == S1_FILTER_EXTRA_BYTES);
    float factory = read_slot(owner, DELAY_MIX).physical;
    extra[14] = 93;
    memcpy(saved, extra, sizeof saved);
    CHECK(engine_s1_patch_set(owner, read_slot(owner, DELAY_MIX).revision, DELAY_MIX, 37) == S1_EDIT_REJECTED);
    CHECK(!memcmp(saved, extra, sizeof saved));
    extra[14] = 0;
    CHECK(engine_s1_patch_set(owner, read_slot(owner, DELAY_MIX).revision, DELAY_MIX, 37) == S1_EDIT_QUEUED);
    extra[14] = 93;
    render(1);
    CHECK(!memcmp(saved, extra, sizeof saved));
    CHECK(read_slot(owner, DELAY_MIX).physical == factory);
    extra[14] = 0;
}
#endif

static void wet_journey(unsigned owner, unsigned enabled, unsigned preset)
{
    setup(V_PART, PART_303A+owner);
    memset(proj.arr.lane, 0, sizeof proj.arr.lane);
    proj.set.bt |= PROJ_BT_CLK_IN;
    choose(owner, preset);
    choose(1-owner, 0);
    float factory[] = {read_slot(owner, DELAY_MIX).physical, read_slot(owner, REVERB_MIX).physical};
    CHECK(engine_s1_patch_set(owner, read_slot(owner, REVERB_MIX).revision, REVERB_MIX, 128) == S1_EDIT_REJECTED);
    for (unsigned v = 0; v < NBASS; ++v) {
        proj.sound.v[T_MIX][PART_303A+v][MX_REV] = 0;
        proj.sound.v[T_MIX][PART_303A+v][MX_DLY] = 0;
    }
    visit("EFFECT");
    CHECK(!strcmp(pref_desc(pg.r[cur_page()][1])->name, "DELAY MIX"));
    CHECK(!strcmp(pref_desc(pg.r[cur_page()][2])->name, "REVERB MIX"));
#if !defined(S1_WET_LEGACY) && !defined(S1_WET_RED)
    opaque_suffix(owner);
#endif
    /* Exercise the exact preimage LFO wire record before upgrading it. */
    const unsigned ids[] = {0, 1, 6, 7, 8, 9, 10, 11, 12, 115, 149, 15, 47, 13, 45, 139};
    float expected[sizeof ids / sizeof ids[0]];
    const unsigned positions[] = {19, 23, 42, 51, 1, 1, 37, 43, 69, 1, 46, 2, 5, 2, 3};
    for (unsigned i = 0; i < sizeof positions / sizeof positions[0]; ++i)
        set_existing(owner, ids[i], positions[i]);
    set_existing(owner, 139, !read_slot(owner, 139).position);
    for (unsigned i = 0; i < sizeof ids / sizeof ids[0]; ++i)
        expected[i] = read_slot(owner, ids[i]).physical;
    if (enabled) {
        stabilize();
        CHECK(!project_save());
        stored_project("wet-v1-control", owner, PROJECT_EXPORT);
    }
    edits_on_held(owner, enabled);
    verify_slot_set(owner, ids, expected, sizeof ids / sizeof ids[0], 0);
    float authored[] = {read_slot(owner, DELAY_MIX).physical, read_slot(owner, REVERB_MIX).physical};
    stabilize();
    CHECK(!project_save() && !project_load());
    verify_slot_set(owner, wet_ids, authored, 2, 0);
    verify_slot_set(owner, ids, expected, sizeof ids / sizeof ids[0], 0);
    if (!enabled) return;
    stored_project("wet-v2-sound", owner, PROJECT_EXPORT);
    engine_note_lock_capture(1);
    CHECK(!engine_bass_note_capture(proj.arr.lane, 0, owner, 0, &proj.sound));
    engine_note_lock_capture(0);
    CHECK(!engine_bass_note_validate(&proj.arr.lane[0]));
    lane_t invalid = proj.arr.lane[0];
    invalid.val[BASS_NOTE_USED+14] |= 0x80u;
    CHECK(engine_bass_note_validate(&invalid));
    choose(owner, preset);
    verify_slot_set(owner, wet_ids, factory, 2, 0);
    CHECK(!project_save() && !project_load());
    stored_project("wet-v2-lock", owner, PROJECT_EXPORT);
    stabilize();
    play_lock(owner, authored, factory);
}

int main(void)
{
    const char *path = getenv("S1_EDITOR_TEST_PACK");
    CHECK(path);
    FILE *file = fopen(path, "rb");
    CHECK(file);
    unsigned bytes = (unsigned)fread(pack, 1, sizeof pack, file);
    CHECK(fgetc(file) == EOF && !fclose(file) && s1_part_set_pack(pack, bytes) == 694);
#ifdef S1_WET_LEGACY
    for (unsigned owner = 0; owner < NBASS; ++owner) {
        setup(V_PART, PART_303A+owner);
        stored_project("wet-v1-control", owner, PROJECT_MOUNT);
        CHECK(!project_load() && !project_storage_unsupported());
        const char *const records[] = {"wet-v2-sound", "wet-v2-lock"};
        for (unsigned i = 0; i < 2; ++i) {
            stored_project(records[i], owner, PROJECT_MOUNT);
            const uint32_t writes = store_writes;
            CHECK(project_load() == -9 && project_storage_unsupported());
            CHECK(project_save() == -9 && store_writes == writes);
            stored_project(records[i], owner, PROJECT_VERIFY);
        }
    }
    printf("PASS exact previous reader accepts the LFO control and refuses wet Sound/lock without writes, %u checks\n", checks);
#elif defined(S1_WET_RED)
    setup(V_PART, PART_303A);
    choose(0, 0);
    visit("EFFECT");
    s1_patch_value absent;
    CHECK(engine_s1_patch_read(0, DELAY_MIX, &absent));
#else
    codec_golden();
    for (unsigned enabled = 0; enabled <= 1; ++enabled) {
        unsigned preset = program_for(enabled);
        for (unsigned owner = 0; owner < NBASS; ++owner) wet_journey(owner, enabled, preset);
    }
    printf("PASS real Synth One delay and reverb MIX, both owners and enables, %u checks\n", checks);
#endif
    return 0;
}
