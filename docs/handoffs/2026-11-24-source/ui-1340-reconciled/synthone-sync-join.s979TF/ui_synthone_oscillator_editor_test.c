/* SPDX-License-Identifier: GPL-3.0-only */
#define ARP_FIXTURE_MAIN retained_arp_main
#include "ui_arp_engine_test.c"
#include "../../firmware/src/dsp/s1c/s1c_front.h"
#include "../../firmware/src/app/s1c_part.h"

enum { SAMPLE_FRAMES = 4096 };
static int32_t before[SAMPLE_FRAMES * 2], edited[SAMPLE_FRAMES * 2], reloaded[SAMPLE_FRAMES * 2];

static void visit_page(const char *name)
{
    for (int i = 0; i < pg.n; ++i) {
        if (!strcmp(pg.title[cur_page()], name)) return;
        now_ms += 160;
        enc_acc[EN_SELECT] = 1;
        input();
    }
    CHECK(0 && "expected Synth One page");
}

static void select_s1(unsigned owner, unsigned preset)
{
    CHECK(!engine_bass_select_preset(owner, ORIGINAL_SOUND_SYNTHONE, preset, &proj.sound));
    for (unsigned n = 0; n < 1024 && engine_bass_preset_pending(owner); ++n) render(1);
    CHECK(engine_bass_bank(owner) == ORIGINAL_SOUND_SYNTHONE);
    CHECK(engine_bass_preset_index(owner) == (int)preset);
    build_pages();
}

static void start_held(unsigned owner)
{
    engine_init(proj.pat, &proj.arr.song, proj.arr.lane, &proj.sound);
    engine_apply_sound(&proj.sound);
    render(4);
    engine_bass_on(owner, 57, 0, 0);
    render(8);
}

static void capture_held(unsigned owner, int32_t *out)
{
    for (unsigned i = 0; i < SAMPLE_FRAMES; i += 256) engine_render(out + i * 2, 256);
    engine_bass_off(owner);
    render(8);
}

static void pcm(unsigned owner, int32_t *out)
{
    start_held(owner);
    capture_held(owner, out);
}

static unsigned differences(const int32_t *a, const int32_t *b)
{
    unsigned n = 0;
    for (unsigned i = 0; i < SAMPLE_FRAMES * 2; ++i) n += a[i] != b[i];
    return n;
}

static void detent(unsigned knob, int delta)
{
    now_ms += 160;
    turn((int)knob, delta);
}

static void layout(unsigned owner, const char *page, const char *const labels[4])
{
    visit_page(page);
    for (unsigned i = 0; i < 4; ++i) {
        pref_t ref = pg.r[cur_page()][i];
        if (!labels[i]) { CHECK(ref.kind == R_NONE); continue; }
        CHECK(ref.b == owner && pref_desc(ref));
        CHECK(!strcmp(pref_desc(ref)->name, labels[i]));
    }
}

#if X0X_S1_EXTRA && X0X_BASS_NOTE_SOUND
static unsigned capture_lock(unsigned owner)
{
    engine_note_lock_capture(1);
    CHECK(!engine_bass_note_capture(proj.arr.lane, 0, owner, 3, &proj.sound));
    engine_note_lock_capture(0);
    for (unsigned i = 0; i < NLANE; ++i) {
        lane_t *lock = &proj.arr.lane[i];
        if (lock->used && lock->t == BASS_NOTE_TARGET && lock->part == PART_303A + owner) {
            CHECK(!engine_bass_note_validate(lock));
            CHECK(!memcmp(lock->val+BASS_NOTE_USED, proj.sound.v[T_303][S1_EXTRA_ROW+owner], S1_EXTRA_BYTES));
            lane_t invalid = *lock;
            invalid.val[BASS_NOTE_USED+2+S1_SUB_WAVE-S1_EXTRA_FIRST] = 2;
            CHECK(engine_bass_note_validate(&invalid));
            return i;
        }
    }
    CHECK(0);
    return 0;
}
#endif

static void journey(unsigned owner)
{
    static const unsigned ids[] = {0, 1, 6, 7, 8, 9, 10, 11, 12};
    static const unsigned group[] = {0,0,1,1,1,1,2,2,2};
    static const unsigned knob[] = {2,3,0,1,2,3,0,1,2};
    static const char *const page[] = {"OSCILLATOR", "OSC MIX", "FM / NOISE"};
    static const char *const labels[3][4] = {
        {"OSC BALANCE", "OSC 1 LEVEL", "OSC 1 MORPH", "OSC 2 MORPH"},
        {"OSC 2 LEVEL", "SUB LEVEL", "SUB OCTAVE", "SUB WAVE"},
        {"FM LEVEL", "FM INDEX", "NOISE LEVEL", NULL}
    };
    setup(V_PART, PART_303A + owner);
    proj.set.bt |= PROJ_BT_CLK_IN;
    select_s1(owner, 0);
    s1_patch_value a;
    CHECK(engine_s1_patch_read(owner, 0, &a)); /* Red: compact oscillator morph has no editor. */
    CHECK(a.physical == s1c_front_init(0));
    uint8_t old_owner[ORIGINAL_SOUND_BYTES];
    memcpy(old_owner, proj.sound.v[T_303][owner], sizeof old_owner);
    uint8_t former_peer[ORIGINAL_SOUND_BYTES];
    memcpy(former_peer, proj.sound.v[T_303][1-owner], sizeof former_peer);
    for (unsigned i = 0; i < 3; ++i) {
        layout(owner, page[i], labels[i]);
        ui_frame();
        const char *const capture[] = {"compact-oscillator", "compact-osc-mix", "compact-fm-noise"};
        if (owner == 0) shot(capture[i]);
    }
#if X0X_S1_EXTRA
    uint8_t peer_extra[S1_EXTRA_BYTES];
    memcpy(peer_extra, proj.sound.v[T_303][S1_EXTRA_ROW+1-owner], sizeof peer_extra);
#endif
    pcm(owner, before);
    for (unsigned i = 0; i < sizeof ids / sizeof ids[0]; ++i) {
        visit_page(page[group[i]]);
        CHECK(engine_s1_patch_read(owner, ids[i], &a));
        CHECK(a.physical == s1c_front_init(ids[i]));
        start_held(owner);
        detent(knob[i], 0);
        capture_held(owner, edited);
        start_held(owner);
        unsigned max = ids[i] == 8 || ids[i] == 9 ? 1 : 127;
        detent(knob[i], a.position > max / 2 ? -127 : 127);
        capture_held(owner, reloaded);
        unsigned audible = differences(edited, reloaded);
        CHECK(audible > 0);
        printf("owner=%u source-id=%u held edit changed %u/%u samples\n", owner, ids[i], audible, SAMPLE_FRAMES*2);
        s1_patch_value changed;
        CHECK(engine_s1_patch_read(owner, ids[i], &changed));
        CHECK(changed.physical != a.physical);
    }
    CHECK(!memcmp(former_peer, proj.sound.v[T_303][1-owner], sizeof former_peer));
#if X0X_S1_EXTRA
    CHECK(!memcmp(peer_extra, proj.sound.v[T_303][S1_EXTRA_ROW+1-owner], sizeof peer_extra));
    sound_t bad = proj.sound;
    bad.v[T_303][S1_EXTRA_ROW+owner][1] |= 0x80u;
    CHECK(engine_bass_sound_validate(&bad));
#endif
    if (owner == 0) {
        visit_page("OSCILLATOR"); ui_frame(); shot("compact-oscillator-edited");
        visit_page("OSC MIX"); ui_frame(); shot("compact-osc-mix-edited");
        visit_page("FM / NOISE"); ui_frame(); shot("compact-fm-noise-edited");
    }
    visit_page("FILTER ADSR");
    detent(0, 42);
    s1_patch_snapshot link_off, link_on;
    CHECK(engine_s1_patch_snapshot(owner, &link_off));
    visit_page("FILTER");
    detent(3, 1);
    CHECK(engine_s1_patch_snapshot(owner, &link_on) && link_on.filter_link);
    CHECK(!memcmp(link_off.physical, link_on.physical, sizeof link_off.physical));
    pcm(owner, edited);
    unsigned changed = differences(before, edited);
    CHECK(changed > SAMPLE_FRAMES);
#if X0X_S1_EXTRA && X0X_BASS_NOTE_SOUND
    unsigned locked = capture_lock(owner);
#endif
    sound_t saved = proj.sound;
    CHECK(!project_save() && !project_load());
    CHECK(!memcmp(&saved, &proj.sound, sizeof saved));
    pcm(owner, reloaded);
    CHECK(!differences(edited, reloaded));
    CHECK(engine_s1_patch_read(owner, 0, &a));
    CHECK(engine_s1_patch_snapshot(owner, &link_on) && link_on.filter_link);
    CHECK(!memcmp(link_off.physical, link_on.physical, sizeof link_off.physical));
#if X0X_S1_EXTRA && X0X_BASS_NOTE_SOUND
    CHECK(!engine_bass_note_validate(&proj.arr.lane[locked]));
    memset(&proj.arr.lane[locked], 0, sizeof proj.arr.lane[locked]);
    engine_note_lock_refresh();
#endif
    select_s1(owner, 0);
    CHECK(engine_s1_patch_set(owner, a.revision, 0, 0) == S1_EDIT_REJECTED);
    CHECK(!memcmp(old_owner, proj.sound.v[T_303][owner], sizeof old_owner));
    pcm(owner, reloaded);
    CHECK(!differences(before, reloaded));
    printf("owner=%u held scene changed %u/%u samples; reload and INIT replacement bit-exact\n",
           owner, changed, SAMPLE_FRAMES * 2);
}

static void factory_journey(const char *pack_path)
{
    static uint8_t pack[0xf000];
    FILE *input_file = fopen(pack_path, "rb");
    CHECK(input_file);
    unsigned n = (unsigned)fread(pack, 1, sizeof pack, input_file);
    CHECK(fgetc(input_file) == EOF && !fclose(input_file) && s1_part_set_pack(pack, n) == 694);
    for (unsigned owner = 0; owner < 2; ++owner) {
        setup(V_PART, PART_303A+owner);
        proj.set.bt |= PROJ_BT_CLK_IN;
        unsigned preset = owner ? 694 : 129;
        select_s1(owner, preset);
        s1_patch_value original, current;
        CHECK(engine_s1_patch_read(owner, 0, &original));
        uint8_t original_cell[ORIGINAL_SOUND_BYTES];
        memcpy(original_cell, proj.sound.v[T_303][owner], sizeof original_cell);
        visit_page("OSCILLATOR");
        detent(2, original.position > 64 ? -127 : 127);
        CHECK(engine_s1_patch_read(owner, 0, &current) && current.physical != original.physical);
        unsigned applied_position = current.position;
        CHECK(!project_save() && !project_load());
        pcm(owner, edited);
        CHECK(engine_s1_patch_read(owner, 0, &current) && current.position == applied_position);
        CHECK(engine_bass_preset_index(owner) == (int)preset);
        select_s1(owner, preset);
        CHECK(!memcmp(original_cell, proj.sound.v[T_303][owner], sizeof original_cell));
        CHECK(engine_s1_patch_read(owner, 0, &current) && current.physical == original.physical);
    }
    printf("PASS factory editor owners A/B: program 129/694 identity and exact replacement, %u checks\n", checks);
}

int main(void)
{
    const char *pack_path = getenv("S1_EDITOR_TEST_PACK");
    if (pack_path) { factory_journey(pack_path); return 0; }
    journey(0);
    journey(1);
    printf("PASS Synth One compact oscillator panel %u checks\n", checks);
    return 0;
}
