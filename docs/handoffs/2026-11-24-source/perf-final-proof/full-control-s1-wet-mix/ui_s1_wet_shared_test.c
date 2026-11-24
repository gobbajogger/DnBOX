/* SPDX-License-Identifier: GPL-3.0-only */
#include "vibrato-mixed-helpers.h"
#include "lfo-objects-helpers.h"

#ifndef S1_WET_MIXED_LEGACY
static void compare_owner(unsigned owner, const uint8_t *cell, const uint8_t *extra)
{
    CHECK(!memcmp(cell, proj.sound.v[T_303][owner], NPARAMS_MAX));
    CHECK(!memcmp(extra, shared(owner), NPARAMS_MAX));
}

static void compare_clock(unsigned owner, const float *prior)
{
    const unsigned ids[] = {15, 47, 13, 45, 139, 44, 40};
    for (unsigned i = 0; i < sizeof ids / sizeof ids[0]; ++i) {
        s1_patch_value out;
        CHECK(engine_s1_patch_read(owner, ids[i], &out));
        CHECK(out.physical == prior[i]);
    }
}

static void mixed_wet_history(unsigned j)
{
    const unsigned s = j ^ 1u;
    fresh_setup(j);
    proj.set.bt |= PROJ_BT_CLK_IN;
    select_bank(j, ORIGINAL_SOUND_JUNO);
    select_bank(s, ORIGINAL_SOUND_SYNTHONE);
    shared(j)[15] = 0xb1;
    shared(j)[NPARAMS_MAX - 1] = 0xcc;
    shared(s)[15] = 0x51;
    shared(s)[NPARAMS_MAX - 1] = 0xdd;

    memset(proj.sound.v[T_303][ARP_BLOCK_VOICE0 + j], 0, ARP_BLOCK_BYTES);
    engine_arp_cfg arp;
    engine_arp_cfg_default(&arp);
    arp.on_mask = 0x56fdu;
    engine_arp_block_encode(&arp, proj.sound.v[T_303][ARP_BLOCK_VOICE0 + s]);
    engine_apply_sound(&proj.sound);
    sj6_factory_row row = authored();
    sj6_factory_add_word(&row, 164, 0x80000000u);
    sj6_factory_add_word(&row, 178, 0u);
    sj6_factory_add_word(&row, SJ6_A_REFERENCE_ID, 0x3eeeeeefu);
    CHECK(engine_superj6_factory_load(j, &row));
    render(1);
    CHECK(engine_superj6_knob_position(j, SJ6_VIBRATO_DEPTH) == 64);
    CHECK(engine_superj6_knob_position(j, SJ6_VIBRATO_RATE) == 95);
    engine_arp_steps j_steps, s_steps;
    CHECK(engine_arp_steps_read(j, &j_steps) && j_steps.on_mask == 0x7ffdu);
    CHECK(engine_arp_steps_read(s, &s_steps) && s_steps.on_mask == 0x56fdu);

    uint8_t peer_cell[NPARAMS_MAX], peer_row[NPARAMS_MAX], arp_rows[2][NPARAMS_MAX];
    memcpy(peer_cell, proj.sound.v[T_303][j], sizeof peer_cell);
    memcpy(peer_row, shared(j), sizeof peer_row);
    memcpy(arp_rows, proj.sound.v[T_303][ARP_BLOCK_VOICE0], sizeof arp_rows);
    CHECK(original_sound_bass_shared_size(peer_cell, peer_row) == 12);
    s1_patch_value foreign;
    CHECK(!engine_s1_patch_read(j, 15, &foreign));
    CHECK(engine_s1_patch_set(j, 0, 15, 1) == S1_EDIT_REJECTED);
    CHECK(set_s1(s, S1_OSC2_LEVEL, 75) == S1_EDIT_QUEUED);
    CHECK(set_s1(s, S1_FILTER_TYPE, 1) == S1_EDIT_QUEUED);
    CHECK(original_sound_bass_shared_size(proj.sound.v[T_303][s], shared(s)) == 13);
    CHECK(!project_save());
    stored_project("mixed-wet-control", s, PROJECT_EXPORT);
    CHECK(set_s1(s, S1_LFO1_RATE, 3) == S1_EDIT_QUEUED);
    CHECK(set_s1(s, S1_LFO2_RATE, 8) == S1_EDIT_QUEUED);
    CHECK(set_s1(s, S1_LFO1_SHAPE, 2) == S1_EDIT_QUEUED);
    CHECK(set_s1(s, S1_LFO2_SHAPE, 3) == S1_EDIT_QUEUED);
    s1_patch_value mode;
    CHECK(engine_s1_patch_read(s, 139, &mode));
    CHECK(set_s1(s, S1_LFO_SYNC, !mode.position) == S1_EDIT_QUEUED);
    CHECK(original_sound_bass_shared_size(proj.sound.v[T_303][s], shared(s)) == 15);
    CHECK(set_s1(s, S1_DELAY_MIX, 59) == S1_EDIT_QUEUED);
    CHECK(set_s1(s, S1_REVERB_MIX, 93) == S1_EDIT_QUEUED);
    CHECK(set_s1(s, S1_LFO1_SHAPE, 1) == S1_EDIT_QUEUED);
    CHECK(original_sound_bass_shared_size(proj.sound.v[T_303][s], shared(s)) == 15);
    CHECK(!shared(s)[0] && !shared(s)[1]);
    CHECK(shared(s)[15] == 0x51 && shared(s)[NPARAMS_MAX - 1] == 0xdd);
    compare_owner(j, peer_cell, peer_row);
    CHECK(!memcmp(arp_rows, proj.sound.v[T_303][ARP_BLOCK_VOICE0], sizeof arp_rows));

    uint8_t s_cell[NPARAMS_MAX], s_row[NPARAMS_MAX];
    memcpy(s_cell, proj.sound.v[T_303][s], sizeof s_cell);
    memcpy(s_row, shared(s), sizeof s_row);
    CHECK(!project_save());
    stored_project("mixed-wet-sound", s, PROJECT_EXPORT);
    float values[7];
    const unsigned ids[] = {15, 47, 13, 45, 139, 44, 40};
    for (unsigned i = 0; i < sizeof ids / sizeof ids[0]; ++i) {
        s1_patch_value out;
        CHECK(engine_s1_patch_read(s, ids[i], &out));
        values[i] = out.physical;
    }
    engine_note_lock_capture(1);
    CHECK(!engine_bass_note_capture(proj.arr.lane, 0, s, 0, &proj.sound));
    engine_note_lock_capture(0);
    CHECK(!engine_bass_note_validate(&proj.arr.lane[0]));
    CHECK(!memcmp(proj.arr.lane[0].val+BASS_NOTE_USED, s_row, 15));
    const lane_t lfo_lock = proj.arr.lane[0];
    lane_t broken = lfo_lock;
    broken.val[BASS_NOTE_USED+14] |= 0x80u;
    CHECK(engine_bass_note_validate(&broken));
    reopen();
    compare_owner(s, s_cell, s_row);
    compare_owner(j, peer_cell, peer_row);
    compare_clock(s, values);

    memset(proj.arr.lane, 0, sizeof proj.arr.lane);
    engine_note_lock_refresh();
    undo_commit_stable();
    select_bank(s, ORIGINAL_SOUND_JUNO);
    for (unsigned i = 0; i < 15; ++i) CHECK(!shared(s)[i]);
    CHECK(shared(s)[15] == 0x51 && shared(s)[NPARAMS_MAX - 1] == 0xdd);
    CHECK(engine_superj6_factory_load(s, &row));
    render(1);
    CHECK(original_sound_bass_shared_size(proj.sound.v[T_303][s], shared(s)) == 12);
    uint8_t j_cell[NPARAMS_MAX], j_row[NPARAMS_MAX];
    memcpy(j_cell, proj.sound.v[T_303][s], sizeof j_cell);
    memcpy(j_row, shared(s), sizeof j_row);
    uint8_t j_arp[NPARAMS_MAX];
    memcpy(j_arp, proj.sound.v[T_303][ARP_BLOCK_VOICE0 + s], sizeof j_arp);
    CHECK(engine_arp_steps_read(s, &s_steps) && s_steps.on_mask == 0x56fdu);
    undo_commit_stable();
    do_undo(0);
    CHECK(engine_bass_bank(s) == ORIGINAL_SOUND_SYNTHONE);
    compare_owner(s, s_cell, s_row);
    compare_clock(s, values);
    compare_owner(j, peer_cell, peer_row);
    CHECK(!memcmp(arp_rows, proj.sound.v[T_303][ARP_BLOCK_VOICE0], sizeof arp_rows));
    do_undo(1);
    CHECK(engine_bass_bank(s) == ORIGINAL_SOUND_JUNO);
    CHECK(engine_superj6_knob_position(s, SJ6_VIBRATO_RATE) == 95);
    compare_owner(s, j_cell, j_row);
    compare_owner(j, peer_cell, peer_row);
    /* SuperJ6 authoring owns its AMP2 data in the ARP block. Redo must
     * restore that exact snapshot without replacing the saved Saga grid. */
    CHECK(!memcmp(arp_rows[j], proj.sound.v[T_303][ARP_BLOCK_VOICE0 + j], NPARAMS_MAX));
    CHECK(!memcmp(j_arp, proj.sound.v[T_303][ARP_BLOCK_VOICE0 + s], NPARAMS_MAX));
    CHECK(engine_arp_steps_read(s, &s_steps) && s_steps.on_mask == 0x56fdu);
    do_undo(0);
    compare_owner(s, s_cell, s_row);

    /* The typed header cannot claim reserved high bits. Both edit and preset
     * admission must decline and preserve the entire damaged row. */
    shared(s)[14] |= 0x80u;
    uint8_t malformed[NPARAMS_MAX];
    memcpy(malformed, shared(s), sizeof malformed);
    CHECK(original_sound_bass_shared_size(s_cell, malformed) < 0);
    CHECK(engine_bass_sound_bank(&proj.sound, s) < 0);
    CHECK(set_s1(s, S1_LFO1_RATE, 7) == S1_EDIT_REJECTED);
    CHECK(engine_bass_select_preset(s, ORIGINAL_SOUND_JUNO, 0, &proj.sound) < 0);
    compare_owner(s, s_cell, malformed);
    compare_owner(j, peer_cell, peer_row);
    memcpy(shared(s), s_row, sizeof s_row);

    /* An unmarked row can carry opaque suffix bytes. They cannot be repurposed
     * by an LFO edit, either at admission or before the queued write commits. */
    select_bank(s, ORIGINAL_SOUND_SYNTHONE);
    shared(s)[13] = 0x71;
    CHECK(original_sound_bass_shared_size(proj.sound.v[T_303][s], shared(s)) == 0);
    CHECK(set_s1(s, S1_LFO1_SHAPE, 3) == S1_EDIT_REJECTED);
    CHECK(shared(s)[13] == 0x71);
    CHECK(set_s1(s, S1_OSC2_LEVEL, 77) == S1_EDIT_QUEUED);
    CHECK(set_s1(s, S1_FILTER_TYPE, 2) == S1_EDIT_QUEUED);
    CHECK(set_s1(s, S1_LFO1_SHAPE, 3) == S1_EDIT_REJECTED);
    CHECK(shared(s)[13] == 0x71 && shared(s)[15] == 0x51);
    shared(s)[13] = 0;
    CHECK(engine_s1_patch_read(s, 45, &mode));
    CHECK(engine_s1_patch_set(s, mode.revision, 45, 3) == S1_EDIT_QUEUED);
    shared(s)[14] = 0x74;
    memcpy(s_cell, proj.sound.v[T_303][s], sizeof s_cell);
    memcpy(s_row, shared(s), sizeof s_row);
    render(1);
    compare_owner(s, s_cell, s_row);
    compare_owner(j, peer_cell, peer_row);
    CHECK(original_sound_bass_shared_size(s_cell, s_row) == 13);
    select_bank(s, ORIGINAL_SOUND_SYNTHONE);
    CHECK(original_sound_bass_shared_size(proj.sound.v[T_303][s], shared(s)) == 0);
    CHECK(shared(s)[14] == 0x74 && shared(s)[15] == 0x51);
    proj.arr.lane[0] = lfo_lock;
    CHECK(!engine_bass_note_validate(&proj.arr.lane[0]));
    CHECK(!project_save());
    stored_project("mixed-wet-lock", s, PROJECT_EXPORT);
    compare_owner(j, peer_cell, peer_row);
    printf("PASS shared-row MIX, LFO, authored vibrato and A REF, refusal and cross-engine history with Synth One owner %u\n", s);
}

#endif

int main(void)
{
#ifdef S1_WET_MIXED_LEGACY
    for (unsigned s = 0; s < NBASS; ++s) {
        unsigned j = s ^ 1u;
        fresh_setup(s);
        stored_project("mixed-wet-control", s, PROJECT_MOUNT);
        CHECK(!project_load() && !project_storage_unsupported());
        engine_apply_sound(&proj.sound);
        render(4);
        CHECK(engine_bass_bank(j) == ORIGINAL_SOUND_JUNO);
        CHECK(engine_bass_bank(s) == ORIGINAL_SOUND_SYNTHONE);
        CHECK(engine_superj6_knob_position(j, SJ6_VIBRATO_RATE) == 95);
        engine_arp_steps steps;
        CHECK(engine_arp_steps_read(j, &steps) && steps.on_mask == 0x7ffdu);
        const char *const cases[] = {"mixed-wet-sound", "mixed-wet-lock"};
        for (unsigned i = 0; i < sizeof cases / sizeof cases[0]; ++i) {
            stored_project(cases[i], s, PROJECT_MOUNT);
            const uint32_t writes = store_writes;
            CHECK(project_load() == -9 && project_storage_unsupported());
            CHECK(project_save() == -9);
            CHECK(store_writes == writes);
            stored_project(cases[i], s, PROJECT_VERIFY);
        }
    }
    printf("PASS previous product reads the mixed control and preserves both unsupported MIX cases, %u checks\n", checks);
#else
    for (unsigned j = 0; j < NBASS; ++j) mixed_wet_history(j);
    printf("PASS joined MIX/shared-row ownership %u checks\n", checks);
#endif
    return 0;
}
