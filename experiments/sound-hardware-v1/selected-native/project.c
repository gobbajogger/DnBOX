/* SPDX-License-Identifier: GPL-3.0-only */
/* Create regular project_save() payloads using the shipping selector and sequencer format. */
#define main simulator_main
#include "../../../host/x0x_host.c"
#undef main
#include <assert.h>
#ifdef NDEBUG
#error "This fixture requires assertions"
#endif

int main(int argc, char **argv)
{
    assert(argc >= 5 && argc <= 7);
    int sub = 0, filter_position = 0;
    for (int i = 5; i < argc; ++i) {
        if (!strcmp(argv[i], "--synthone-sub")) sub = 1;
        else if (!strcmp(argv[i], "--synthone-filter=bandpass")) filter_position = 64;
        else if (!strcmp(argv[i], "--synthone-filter=highpass")) filter_position = 127;
        else assert(0 && "unknown scene argument");
    }
#if !X0X_SYNTHONE_PATCH
    assert(!sub && !filter_position);
#endif
    const unsigned kind[2] = {(unsigned)atoi(argv[1]), (unsigned)atoi(argv[2])};
    const int kit = atoi(argv[3]); /* -1 bass/BREAK, 0/1 isolated kit, 2 all parts. */
    assert(kit >= -1 && kit <= 2);
    assert(!(sub || filter_position) || (kit == 2 && (kind[0] == ORIGINAL_SOUND_SYNTHONE ||
                                kind[1] == ORIGINAL_SOUND_SYNTHONE)));
    const int notes = kit < 0 || kit == 2;
    boot();
    pattern_t *pat = &proj.pat[0];
    for (int voice = 0; voice < 2; ++voice) {
        int32_t buffer[512];
        assert(engine_bass_select_preset(voice, kind[voice], 0, &proj.sound) == 0);
        for (int i = 0; i < 1000 && engine_bass_preset_pending(voice); ++i)
            engine_render(buffer, 256);
        assert(!engine_bass_preset_pending(voice));
        assert(engine_bass_bank(voice) == (int)kind[voice]);
#if X0X_SYNTHONE_PATCH
        const unsigned slots[] = {S1_PATCH_SUB_LEVEL, S1_PATCH_FILTER_TYPE};
        const unsigned positions[] = {sub ? S1_PATCH_STEPS : 0, (unsigned)filter_position};
        if (kind[voice] == ORIGINAL_SOUND_SYNTHONE) for (unsigned edit = 0; edit < 2; ++edit) {
            if (!positions[edit]) continue;
            unsigned id = s1_patch_at(slots[edit])->factory_id;
            s1_patch_value value;
            assert(engine_s1_patch_read(voice, id, &value));
            assert(engine_s1_patch_set(voice, value.revision, id, positions[edit]) == S1_EDIT_QUEUED);
            for (int i = 0; i < 16 && engine_s1_patch_pending(); ++i)
                engine_render(buffer, 256);
            assert(!engine_s1_patch_pending());
            assert(engine_s1_patch_read(voice, id, &value) &&
                   value.position == positions[edit] && value.physical > 0.f);
        }
#endif

        bpart_t *part = &pat->bass[voice];
        part->len = 16;
        for (int i = 0; i < part->len; ++i) {
            part->step[i] = (bstep_t){
                .note = (uint8_t)(48 + 12 * voice + (i % 3) * 3),
                .flags = notes ? G_NOTE : G_REST, .velocity = 100,
            };
        }
        memset(pat->drum[voice].hit, 0, sizeof pat->drum[voice].hit);
    }
    for (int k = 0; k < 2; ++k) {
        if (k != kit && kit != 2) continue;
        dpart_t *drum = &pat->drum[k];
        drum->len = 16;
        for (int step = 0; step < drum->len; ++step) {
            sm_set(drum->hit[step % NDRUM], step);
            sm_set(drum->hit[(step + 5) % NDRUM], step);
        }
    }
    pat->brk.steps = notes ? 0xffff : 0;
    pat->brk.slot_a = pat->brk.slot_b = 0;
    pat->brk.set[BRK_COMPLEX] = 0;
    assert(project_save() == 0);

    FILE *f = fopen(argv[4], "wbx");
    assert(f);
    for (int i = 0; i < OBJ_NOBJ; ++i) {
        uint32_t size = store_len[i] < 0 ? 0 : (uint32_t)store_len[i];
        const uint8_t length_le[4] = {
            (uint8_t)size, (uint8_t)(size >> 8), (uint8_t)(size >> 16), (uint8_t)(size >> 24)
        };
        assert(fwrite(length_le, 1, sizeof length_le, f) == sizeof length_le);
        assert(fwrite(store[i], 1, size, f) == size);
    }
    assert(fclose(f) == 0);
    return 0;
}
