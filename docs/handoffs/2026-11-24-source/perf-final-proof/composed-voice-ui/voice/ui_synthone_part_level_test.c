/* SPDX-License-Identifier: GPL-3.0-only */
#include "lfo-test-helpers.h"
#include "../../firmware/src/app/s1c_part.h"

void host_s1_retrigger(const s1c *s) { (void)s; }
enum { PART_LEVEL = 32, REVERB_MIX = 40 };
static const unsigned wet_ids[] = {PART_LEVEL};
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
            (!enabled || (part.e.t[S1C_P_RVMIX] > .25f))) return index+1;
    }
    CHECK(0 && "authentic factory case with matching wet enables");
    return 0;
}

static void edits_on_held(unsigned owner, unsigned enabled)
{
    const float flag[] = {(float)enabled, (float)enabled};
    for (unsigned effect = 0; effect < 1; ++effect) {
        const unsigned knob = 1;
        const unsigned id = wet_ids[effect];
        const float before = read_slot(owner, id).physical;
        sound_t prior = proj.sound;
        const float peer = read_slot(1-owner, id).physical;
        const float other = read_slot(owner, 44u).physical;
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
        detent(knob, before < 1.f ? 127 : -127);
        host_s1_wet_peek(owner, enables, edited);
        CHECK(!memcmp(enables, flag, sizeof flag));
        CHECK(!memcmp(edited, allocation, sizeof edited));
        take(1);
        CHECK(read_slot(owner, id).physical == (before < 1.f ? 2.f : 0.f));
        CHECK(read_slot(owner, 44u).physical == other);
        CHECK(read_slot(1-owner, id).physical == peer);
        CHECK(!memcmp(prior.v[T_MIX], proj.sound.v[T_MIX], sizeof prior.v[T_MIX]));
        CHECK(!memcmp(takes[0][1-owner], takes[1][1-owner], sizeof takes[0][0]));
        unsigned differences = 0;
        for (unsigned n = 0; n < FRAMES; ++n)
            differences += memcmp(takes[0][owner][n], takes[1][owner][n], sizeof takes[0][owner][n]) != 0;
        printf("owner %u PART LEVEL %u enable %u: %u/%u owner frames changed, peer exact\n",
               owner, id, enabled, differences, FRAMES);
        CHECK(differences > 100);
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
    for (unsigned n = 0; n < 2000 && !read_slot(owner, PART_LEVEL).note_locked; ++n) render(1);
    verify_slot_set(owner, wet_ids, locked, 1, 1);
    const sound_t sound = proj.sound;
    const lane_t lock = proj.arr.lane[0];
    CHECK(engine_s1_patch_set(owner, read_slot(owner, PART_LEVEL).revision, PART_LEVEL, 31) == S1_EDIT_BUSY);
    CHECK(!memcmp(&sound, &proj.sound, sizeof sound));
    CHECK(!memcmp(&lock, &proj.arr.lane[0], sizeof lock));
    for (unsigned n = 0; n < 2000 && read_slot(owner, PART_LEVEL).note_locked; ++n) render(1);
    verify_slot_set(owner, wet_ids, base, 1, 0);
    seq_stop(&seq);
    render(4);
    seq_start(&seq);
    for (unsigned n = 0; n < 2000 && !read_slot(owner, PART_LEVEL).note_locked; ++n) render(1);
    verify_slot_set(owner, wet_ids, locked, 1, 1);
    seq_stop(&seq);
    render(4);
    verify_slot_set(owner, wet_ids, base, 1, 0);
}


#if !defined(S1_WET_LEGACY) && !defined(S1_WET_RED)
static void codec_golden(void)
{
    uint8_t cell[ORIGINAL_SOUND_BYTES], data[S1_EXTRA_BYTES] = {0};
    CHECK(original_sound_init(cell,ORIGINAL_SOUND_SYNTHONE,0));
    CHECK(original_sound_s1_extra_set(cell,data,S1_DELAY_MIX,8));
    CHECK(original_sound_s1_extra_set(cell,data,S1_REVERB_MIX,73));
    CHECK(original_sound_s1_extra_set(cell,data,S1_REVERB_HP,97));
    CHECK(original_sound_s1_extra_set(cell,data,S1_WIDEN,2));
    uint8_t saved[ORIGINAL_SOUND_BYTES], extra[S1_EXTRA_BYTES];
    memcpy(extra,data,sizeof extra);
    memcpy(saved,cell,sizeof saved);
    for(unsigned i=0;i<=127u;++i) {
        CHECK(original_sound_s1_set(cell,S1_PART_LEVEL,i));
        CHECK(original_sound_valid(cell));
        CHECK(cell[3]==i && (original_sound_edit_mask(cell)&8u));
        CHECK(!memcmp(data,extra,sizeof extra));
        for(unsigned k=0;k<sizeof cell;++k) if(k!=3 && k!=22) CHECK(cell[k]==saved[k]);
    }
    memcpy(saved,cell,sizeof saved);
    CHECK(!original_sound_s1_set(cell,S1_PART_LEVEL,128));
    CHECK(!memcmp(saved,cell,sizeof saved));
    cell[3]=128;
    CHECK(!original_sound_valid(cell));
}
#endif

static void hp_journey(unsigned owner, unsigned enabled, unsigned preset)
{
    setup(V_PART, PART_303A+owner);
    memset(proj.arr.lane, 0, sizeof proj.arr.lane);
    proj.set.bt |= PROJ_BT_CLK_IN;
    choose(owner, preset);
    choose(1-owner, 0);
    float factory[] = {read_slot(owner, PART_LEVEL).physical, read_slot(owner, REVERB_MIX).physical};
    CHECK(engine_s1_patch_set(owner, read_slot(owner, REVERB_MIX).revision, REVERB_MIX, 128) == S1_EDIT_REJECTED);
    for (unsigned v = 0; v < NBASS; ++v) {
        proj.sound.v[T_MIX][PART_303A+v][MX_REV] = 0;
        proj.sound.v[T_MIX][PART_303A+v][MX_DLY] = 0;
    }
    visit("OUTPUT");
    CHECK(!strcmp(pref_desc(pg.r[cur_page()][1])->name, "PART LEVEL"));
    CHECK(!strcmp(pref_desc(pg.r[cur_page()][0])->name, "WIDEN"));
#if !defined(S1_WET_LEGACY) && !defined(S1_WET_RED)

#endif
    const s1_patch_value init = read_slot(owner, PART_LEVEL);
    CHECK(s1_patch_at((unsigned)s1_patch_slot(PART_LEVEL))->unit == X0X_U_PCT);
    int direction = init.position < 127u ? 1 : -1;
    begin_pair(owner, 57);
    detent(1, direction);
    render(1);
    float adjacent = read_slot(owner, PART_LEVEL).physical;
    CHECK(fabsf(adjacent - init.physical) < .024f);
    CHECK(direction > 0 ? adjacent > init.physical : adjacent < init.physical);
    printf("owner %u PART LEVEL factory %.6f -> adjacent %.6f original gain unit\n", owner, init.physical, adjacent);
    engine_bass_off(owner); engine_bass_off(1-owner);
    choose(owner, preset);
    CHECK(read_slot(owner, PART_LEVEL).physical == init.physical);
    visit("OUTPUT");
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
        stored_project("level-v1-control", owner, PROJECT_EXPORT);
    }
    edits_on_held(owner, enabled);
    verify_slot_set(owner, ids, expected, sizeof ids / sizeof ids[0], 0);
    float authored[] = {read_slot(owner, PART_LEVEL).physical, read_slot(owner, REVERB_MIX).physical};
    stabilize();
    CHECK(!project_save() && !project_load());
    verify_slot_set(owner, wet_ids, authored, 1, 0);
    verify_slot_set(owner, ids, expected, sizeof ids / sizeof ids[0], 0);
    if (!enabled) return;
    stored_project("level-v2-sound", owner, PROJECT_EXPORT);
    engine_note_lock_capture(1);
    CHECK(!engine_bass_note_capture(proj.arr.lane, 0, owner, 0, &proj.sound));
    engine_note_lock_capture(0);
    CHECK(!engine_bass_note_validate(&proj.arr.lane[0]));
    lane_t invalid = proj.arr.lane[0];
    invalid.val[3] = 128u;
    CHECK(engine_bass_note_validate(&invalid));
    choose(owner, preset);
    verify_slot_set(owner, wet_ids, factory, 1, 0);
    CHECK(!project_save() && !project_load());
    stored_project("level-v2-lock", owner, PROJECT_EXPORT);
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
        stored_project("level-v1-control", owner, PROJECT_MOUNT);
        CHECK(!project_load() && !project_storage_unsupported());
        const char *const records[] = {"level-v2-sound", "level-v2-lock"};
        for (unsigned i = 0; i < 2; ++i) {
            stored_project(records[i], owner, PROJECT_MOUNT);
            const uint32_t writes = store_writes;
            CHECK(project_load() == -9 && project_storage_unsupported());
            CHECK(project_save() == -9 && store_writes == writes);
            stored_project(records[i], owner, PROJECT_VERIFY);
        }
    }
    printf("PASS exact previous reader accepts the LFO control and refuses Part Level Sound/lock without writes, %u checks\n", checks);
#elif defined(S1_WET_RED)
    setup(V_PART, PART_303A);
    choose(0, 0);
    visit("OUTPUT");
    s1_patch_value absent;
    CHECK(engine_s1_patch_read(0, PART_LEVEL, &absent));
#else
    codec_golden();
    for (unsigned enabled = 0; enabled <= 1; ++enabled) {
        unsigned preset = program_for(enabled);
        for (unsigned owner = 0; owner < NBASS; ++owner) hp_journey(owner, enabled, preset);
    }
    printf("PASS real Synth One PART LEVEL, both owners and enables, %u checks\n", checks);
#endif
    return 0;
}
