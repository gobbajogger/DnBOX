/* Compare two independently linked compact renderers using complete state. */
#include "s1c_part.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

void reference_s1c_render(s1c *, float *, float *, unsigned);

static ptrdiff_t relative(const void *pointer, const void *base, size_t bytes)
{
    if (!pointer) return -1;
    uintptr_t at = (uintptr_t)pointer, first = (uintptr_t)base;
    assert(at >= first && at - first < bytes);
    return (ptrdiff_t)(at - first);
}

static void same_state(const s1_dry_part *test, const s1_dry_part *reference)
{
    s1_dry_part a, b;
    memcpy(&a, test, sizeof a);
    memcpy(&b, reference, sizeof b);
    assert(memcmp(a.arena, b.arena, S1C_ARENA_BYTES) == 0);
#define SAME_PTR(ptr, base_a, base_b, bytes) do { \
    assert(relative(a.ptr, base_a, bytes) == relative(b.ptr, base_b, bytes)); \
    a.ptr = NULL; b.ptr = NULL; \
} while (0)
#define SAME_ARENA(ptr) SAME_PTR(ptr, test->arena, reference->arena, S1C_ARENA_BYTES)
    SAME_ARENA(e.pool);
    SAME_ARENA(e.rv.m);
    SAME_ARENA(e.rv.e);
    SAME_ARENA(e.rv.st);
    assert(memcmp(a.e.rv.off, b.e.rv.off, 5 * sizeof(uint16_t)) == 0);
    a.e.rv.off = b.e.rv.off = NULL;
    SAME_PTR(e.wid.buf, test->wbuf, reference->wbuf, sizeof test->wbuf);
    assert(memcmp(a.e.wid.len, b.e.wid.len, 3 * sizeof(int16_t)) == 0);
    a.e.wid.len = b.e.wid.len = NULL;
    for (unsigned i = 0; i < 4; ++i) {
        SAME_ARENA(e.dl[i].buf);
        SAME_ARENA(e.dl[i].m);
        SAME_ARENA(e.dl[i].ex);
    }
    a.arena = b.arena = NULL;
    assert(memcmp(&a, &b, sizeof a) == 0);
#undef SAME_ARENA
#undef SAME_PTR
}

int main(int argc, char **argv)
{
    assert(argc == 2);
    static uint8_t pack[65536];
    FILE *input = fopen(argv[1], "rb");
    assert(input);
    size_t size = fread(pack, 1, sizeof pack, input);
    assert(!ferror(input) && !fclose(input));
    assert(s1_part_set_pack(pack, (uint32_t)size) == 694);
    static _Alignas(8) uint8_t test_arena[S1C_ARENA_BYTES], ref_arena[S1C_ARENA_BYTES];
    uint64_t frames = 0;
    unsigned spans = 0;
    for (unsigned program = 0; program <= 694; ++program) {
        unsigned id = program < 694 ? program : S1_PART_INIT;
        s1_dry_part a, b;
        memset(test_arena, 0, sizeof test_arena);
        memset(ref_arena, 0, sizeof ref_arena);
        s1_part_init(&a);
        s1_part_init(&b);
        s1_part_bind(&a, test_arena);
        s1_part_bind(&b, ref_arena);
        assert(s1_part_program(&a, id) && s1_part_program(&b, id));
        enum { MONO_PARAM = 20 }; /* Compact core mono parameter ID. */
        for (unsigned span = 0; span < 40; ++span) {
            static const unsigned lengths[] = {0, 1, 7, 19, 64, 127, 128, 255, 256, 256};
            unsigned n = lengths[span % (sizeof lengths / sizeof *lengths)];
            if (span == 16 || span == 24) {
                float mono = span == 16 ? 1.0f : 0.0f;
                s1c_set(&a.e, MONO_PARAM, mono);
                s1c_set(&b.e, MONO_PARAM, mono);
            }
            if (span == 0 || span == 17 || span == 21 || span == 27) {
                for (unsigned key = 48; key <= 63; key += 3) {
                    unsigned vel = span == 21 ? 0 : 104;
                    assert(s1_part_note(&a, key, vel) && s1_part_note(&b, key, vel));
                }
            }
            if (span == 18 || span == 31) {
                float rv = span == 18 ? 0.97f : 0.0f;
                s1c_set(&a.e, S1C_P_RVFB, rv);
                s1c_set(&b.e, S1C_P_RVFB, rv);
            }
            float l[256] = {0}, r[256] = {0}, ref_l[256] = {0}, ref_r[256] = {0};
            s1c_render(&a.e, l, r, n);
            reference_s1c_render(&b.e, ref_l, ref_r, n);
            assert(memcmp(l, ref_l, sizeof l) == 0 && memcmp(r, ref_r, sizeof r) == 0);
            same_state(&a, &b);
            spans++;
            frames += n;
        }
    }
    printf("PASS: identical stereo PCM, complete normalized part state and effects arena across "
           "%u programs including INIT, %u spans, %llu frames\n",
           695u, spans, (unsigned long long)frames);
}
