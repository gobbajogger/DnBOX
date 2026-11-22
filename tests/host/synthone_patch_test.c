/* SPDX-License-Identifier: GPL-3.0-only */
#define main simulator_main
#include "../../host/x0x_host.c"
#undef main
#include <assert.h>

static void block(void)
{
    int32_t samples[512];
    engine_render(samples, 256);
}

static void select_patch(unsigned part, unsigned bank)
{
    assert(engine_bass_select_preset(part, bank, 0, &proj.sound) == 0);
    for (unsigned i = 0; i < 1000 && engine_bass_preset_pending(part); ++i) block();
    assert(!engine_bass_preset_pending(part) && engine_bass_bank(part) == (int)bank);
}

static s1_patch_value read_patch(unsigned part, unsigned id)
{
    s1_patch_value value;
    assert(engine_s1_patch_read(part, id, &value));
    assert(value.preset == ORIGINAL_SYNTHONE_INIT_FACTORY);
    return value;
}

static void note_lock(void)
{
    unsigned pat = 4;
    bpart_t *line = &proj.pat[pat].bass[0];
    for (unsigned s = 0; s < NSTEPS; ++s) line->step[s].flags = G_REST;
    line->len = 16; line->rate = RATE_16; line->dir = DIR_FWD; line->transpose = 24;
    line->step[0].flags = line->step[4].flags = G_NOTE;
    line->step[0].note = 36; line->step[4].note = 43;
    line->step[0].velocity = line->step[4].velocity = 100;
    engine_note_lock_capture(1);
    assert(!engine_bass_note_capture(proj.arr.lane,pat,0,0,&proj.sound));
    engine_note_lock_refresh();
    s1_patch_value before = read_patch(0,16);
    assert(engine_s1_patch_set(0,before.revision,16,9) == S1_EDIT_QUEUED);
    engine_bass_edit_hold(1);
    block();
    assert(read_patch(0,16).physical == before.physical);
    engine_bass_edit_hold(0);
    block();
    assert(read_patch(0,16).position == 9);
    seq.ppat[TRK_BASS0] = pat;
    seq_start(&seq);
    for (unsigned i=0; i<2000 && !read_patch(0,16).note_locked; ++i) block();
    s1_patch_value locked = read_patch(0,16);
    assert(locked.note_locked && locked.physical == before.physical);
    assert(engine_s1_patch_set(0,before.revision,16,0) == S1_EDIT_BUSY);
    for (unsigned i=0; i<2000 && read_patch(0,16).note_locked; ++i) block();
    assert(!read_patch(0,16).note_locked && read_patch(0,16).position == 9);
    seq_stop(&seq);
    block();
}

static void sub_extension_cells(void)
{
    uint8_t cell[32], ext[S1_MASTER_EXT_SIZE] = {0};
    assert(original_sound_init(cell, ORIGINAL_SOUND_SYNTHONE, 0));
    for (unsigned id = ORIGINAL_SYNTHONE_MASTER_VOLUME; id <= ORIGINAL_SYNTHONE_MASTER_MAKEUP; ++id)
        assert(original_sound_set_extended(cell, ext, id, 20 + id));
    assert(original_sound_set_extended(cell, ext, ORIGINAL_SYNTHONE_LFO2_LEVEL, 93));
    const uint8_t expected[S1_MASTER_EXT_SIZE] = {1, 37, 38, 39, 40, 41, 42};
    assert(!memcmp(ext, expected, sizeof ext));
    assert(original_master_valid(cell, ext) && original_master_edited(cell, ext));
    assert(!original_sound_s1_sub_position(cell, ext, S1_PATCH_SUB_LEVEL));
    assert(original_sound_s1_set_extended(cell, ext, S1_PATCH_SUB_LEVEL, 86));
    assert(original_sound_s1_set_extended(cell, ext, S1_PATCH_SUB_OCTAVE, 127));
    assert(original_sound_s1_set_extended(cell, ext, S1_PATCH_SUB_SQUARE, 127));
    assert(original_master_valid(cell, ext) && original_master_edited(cell, ext));
    for (unsigned id = ORIGINAL_SYNTHONE_MASTER_VOLUME; id <= ORIGINAL_SYNTHONE_MASTER_MAKEUP; ++id)
        assert(original_master_display(cell, ext, id) == 20 + id);
    assert(original_sound_s1_display(cell, ORIGINAL_SYNTHONE_LFO2_LEVEL) == 93);
    assert(original_sound_set_extended(cell, ext, ORIGINAL_SYNTHONE_MASTER_RATIO, 72));
    assert(original_sound_s1_sub_position(cell, ext, S1_PATCH_SUB_LEVEL) == 86);
    assert(original_sound_s1_sub_position(cell, ext, S1_PATCH_SUB_OCTAVE) == 127);
    assert(original_sound_s1_sub_position(cell, ext, S1_PATCH_SUB_SQUARE) == 127);
    assert(original_master_display(cell, ext, ORIGINAL_SYNTHONE_MASTER_RATIO) == 72);
    uint8_t corrupt[S1_MASTER_EXT_SIZE];
    memcpy(corrupt, ext, sizeof corrupt);
    corrupt[5] |= 128u;
    assert(!original_master_valid(cell, corrupt));
    assert(!original_sound_s1_set_extended(cell, corrupt, S1_PATCH_SUB_LEVEL, 1));
    assert(original_sound_s1_sub_position(cell, ext, S1_PATCH_SUB_LEVEL) == 86);

    /* An opted-in sub-only cell leaves the ordinary master bypassed. A later
     * first master edit starts from the legacy defaults, keeping the sub. */
    assert(original_sound_init(cell, ORIGINAL_SOUND_SYNTHONE, 0));
    memset(ext, 255, sizeof ext);
    assert(original_sound_s1_set_extended(cell, ext, S1_PATCH_SUB_SQUARE, 127));
    assert(original_master_valid(cell, ext) && !original_master_edited(cell, ext));
    assert(original_master_display(cell, ext, ORIGINAL_SYNTHONE_MASTER_VOLUME) == 32);
    assert(original_sound_set_extended(cell, ext, ORIGINAL_SYNTHONE_MASTER_VOLUME, 64));
    assert(original_master_valid(cell, ext) && original_master_edited(cell, ext));
    assert(original_master_display(cell, ext, ORIGINAL_SYNTHONE_MASTER_VOLUME) == 64);
    assert(original_sound_s1_sub_position(cell, ext, S1_PATCH_SUB_SQUARE) == 127);
}

int main(void)
{
    sub_extension_cells();
    boot();
    select_patch(0, ORIGINAL_SOUND_SYNTHONE);
    select_patch(1, ORIGINAL_SOUND_SYNTHONE);
    s1_patch_value a = read_patch(0, 16), b = read_patch(1, 16);
    assert(a.physical == 2000.f && b.physical == 2000.f && !a.note_locked);
    assert(engine_s1_patch_set(1, a.revision, 16, 0) == S1_EDIT_REJECTED);
    assert(engine_s1_patch_set(0, a.revision, 0, 0) == S1_EDIT_REJECTED);
    assert(engine_s1_patch_set(0, a.revision, 1, 0) == S1_EDIT_REJECTED);
    assert(engine_s1_patch_set(0, a.revision, 16, 128) == S1_EDIT_REJECTED);
    assert(!engine_s1_patch_read(2, 16, &a));
    assert(!s1_patch_at(S1_PATCH_COUNT));
    assert(!engine_bass_preset_edited(0) && !engine_bass_preset_edited(1));

    unsigned ids[] = {2,5,16,17,19,22,23,24,25,26,27,28,29,7,8,9};
    assert(sizeof ids / sizeof ids[0] == S1_PATCH_COUNT);
    for (unsigned i = 0; i < S1_PATCH_COUNT; ++i) {
        const s1_patch_descriptor *d = s1_patch_at(i);
        assert(d && *d->label && d->factory_id == ids[i] && d->minimum < d->maximum);
        assert(d->unit == X0X_U_HZ || d->unit == X0X_U_S || d->unit == X0X_U_PCT);
        assert(s1_patch_physical(d, 0) == d->minimum && s1_patch_physical(d, 127) == d->maximum);
        /* Sweep every position on one part, reading the other owner's target
         * and comparing its entire sound cell and identity after each edit. */
        const s1_patch_value other = read_patch(1, ids[i]);
        const sound_t original = proj.sound;
        for (unsigned q = 0; q <= S1_PATCH_STEPS; ++q) {
            float before = read_patch(0, ids[i]).physical;
            assert(engine_s1_patch_set(0, a.revision, ids[i], q) == S1_EDIT_QUEUED);
            assert(read_patch(0, ids[i]).physical == before);
            assert(engine_s1_patch_set(0, a.revision, ids[i], q) == S1_EDIT_BUSY);
            block();
            s1_patch_value actual = read_patch(0, ids[i]);
            unsigned position = (ids[i] == 8u || ids[i] == 9u) ? (q >= 64u ? 127u : 0u) : q;
            assert(actual.physical == s1_patch_physical(d, q) && actual.position == position);
            assert(read_patch(1, ids[i]).physical == other.physical);
            assert(!memcmp(original.v[T_303][1], proj.sound.v[T_303][1], ORIGINAL_SOUND_BYTES));
            assert(!memcmp(&original.bass[1], &proj.sound.bass[1], sizeof original.bass[1]));
        }
    }
    assert(engine_bass_preset_edited(0) && !engine_bass_preset_edited(1));
    assert(engine_bass_sound_validate(&proj.sound) == 0);
    sound_t saved = proj.sound;
    assert(project_save() == 0);
    assert(engine_s1_patch_set(0, a.revision, 16, 0) == S1_EDIT_QUEUED);
    assert(project_save() == -8);
    select_patch(0, ORIGINAL_SOUND_SYNTHONE);
    assert(read_patch(0, 16).physical == 2000.f && !engine_bass_preset_edited(0));
    assert(engine_s1_patch_set(0, a.revision, 16, 0) == S1_EDIT_REJECTED);
    s1_patch_value fresh = read_patch(0, 16);
    assert(fresh.revision != a.revision);
    assert(engine_s1_patch_set(0, fresh.revision, 16, 0) == S1_EDIT_QUEUED);
    assert(project_load() == 0);
    engine_apply_sound(&proj.sound);
    block();
    assert(!memcmp(&saved, &proj.sound, sizeof saved));
    assert(engine_s1_patch_set(0, fresh.revision, 16, 0) == S1_EDIT_REJECTED);
    for (unsigned i = 0; i < S1_PATCH_COUNT; ++i)
        assert(read_patch(0, ids[i]).physical == s1_patch_at(i)->maximum);
    s1_patch_snapshot snapshot;
    assert(engine_s1_patch_snapshot(0, &snapshot) && !snapshot.note_locked);
    for (unsigned i=0; i<S1_PATCH_COUNT; ++i) assert(snapshot.physical[i] == s1_patch_at(i)->maximum);
    assert(!engine_s1_patch_snapshot(2, &snapshot) && !engine_s1_patch_snapshot(0, NULL));
    assert(engine_s1_patch_set(1, read_patch(1,16).revision, 16, 1) == S1_EDIT_QUEUED);
    block();
    assert(read_patch(1,16).position == 1 && read_patch(0,16).position == 127);
    select_patch(1, ORIGINAL_SOUND_MIAMI);
    assert(!engine_s1_patch_read(1, 16, &b));
    assert(engine_s1_patch_set(1, b.revision, 16, 127) == S1_EDIT_REJECTED);
    note_lock();
    puts("Synth One INIT patch: 16 factory IDs x 128 positions, separate owners, stale/refused writes, queued readback, project save/load, preset replacement, coherent snapshot and note locks PASS");
}
