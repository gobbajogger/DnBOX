/* SPDX-License-Identifier: GPL-3.0-only */
/* Each mode renders the same wet mix in a fresh host process. */
#define main host_simulator_main
#include "../../host/x0x_host.c"
#undef main
#include <assert.h>
#include <pthread.h>

enum worker_mode { SERIAL, DEFERRED, FAILED, CONCURRENT, INTERRUPTED };
static enum worker_mode mode;
static pthread_t thread;
static _Thread_local int in_worker;
static void (*pending)(uint32_t);
static uint32_t argument;
static unsigned jobs, drops;
extern int mix_test_kits_complete(void);

void plat_cpu2_sync(void)
{
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
    /* Both kits have published, while later worker jobs remain. */
    if (in_worker && mode == INTERRUPTED && mix_test_kits_complete()) {
        ++drops;
        pthread_exit(NULL);
    }
}

static void *worker(void *unused)
{
    (void)unused;
    in_worker = 1;
    pending(argument);
    return NULL;
}

int mix_test_submit(void (*fn)(uint32_t), uint32_t arg)
{
    assert(!pending);
    if (mode == SERIAL) return 0;
    ++jobs;
    pending = fn;
    argument = arg;
    if (mode == CONCURRENT || mode == INTERRUPTED)
        assert(!pthread_create(&thread, NULL, worker, NULL));
    return 1;
}

int mix_test_join(void)
{
    assert(pending);
    if (mode == DEFERRED)
        pending(argument);
    else if (mode == CONCURRENT || mode == INTERRUPTED)
        assert(!pthread_join(thread, NULL));
    pending = NULL;
    return mode == FAILED || mode == INTERRUPTED ? -1 : 0;
}

static void render(FILE *stream, unsigned n)
{
    struct { int32_t pcm[512]; uint32_t guard; } out;
    memset(&out, 0x5a, sizeof out);
    engine_render(out.pcm, n);
    unsigned frames = n > 256 ? 256 : n;
    assert(out.guard == 0x5a5a5a5a && !pending);
    for (unsigned i = frames * 2; i < 512; ++i) assert(out.pcm[i] == 0x5a5a5a5a);
    assert(fwrite(out.pcm, 2 * sizeof *out.pcm, frames, stream) == frames);
}

static void select_bank(FILE *stream, int part, int bank)
{
    assert(!engine_bass_select_preset(part, bank, 0, &proj.sound));
    for (unsigned i = 0; engine_bass_preset_pending(part) && i < 2000; ++i)
        render(stream, 256);
    assert(!engine_bass_preset_pending(part) && engine_bass_bank(part) == bank);
}

int main(int argc, char **argv)
{
    assert(argc == 4);
    mode = (enum worker_mode)atoi(argv[1]);
    const int play_break = atoi(argv[2]);
    FILE *stream = fopen(argv[3], "wb");
    assert(stream);
    boot();
    engine_guard_enable(0);
    seq.playing = 0;
    select_bank(stream, 0, ORIGINAL_SOUND_SYNTHONE);
    select_bank(stream, 1, ORIGINAL_SOUND_JUNO);

    static uint8_t sample[4097];
    for (unsigned i = 0; i < sizeof sample; ++i) sample[i] = (uint8_t)(19*i + 1);
    slot_nz[0] = 1;
    slot_data[0][0] = sample;
    slot_n[0][0] = 8193;
    slot_rate[0][0] = 22050;
    engine_brk_loops();
    proj.pat[seq.ppat[TRK_BRK]].brk.slot_a = engine_brk_nslots() - 1;
    engine_brk_loops();
    render(stream, 256);
    engine_brk_flush();
    seq.mute = 0;

    static const unsigned sizes[] = {256, 1, 255, 3, 129, 0, 17, 300, 64, 251};
    for (unsigned b = 0; b < 100; ++b) {
        if (b == 52) {
            engine_request_notes_off();
            render(stream, 256);
            select_bank(stream, 1, ORIGINAL_SOUND_CHICAGO);
        }
        if (b % 11 == 0) {
            engine_drum(0, b % 11, .8f);
            engine_drum(1, b % 11, .7f);
            if (play_break) engine_brk_live(32 + b % 8, 1);
            engine_bass_on_velocity(0, 48 + b % 24, 1, 0, 110);
            engine_bass_on_velocity(1, 60 + b % 12, 0, 0, 90);
        } else if (b % 11 == 1) {
            engine_bass_on_velocity(1, 54 + b % 12, 0, 1, 91);
        } else if (b % 11 == 7) {
            engine_bass_off(0);
            engine_bass_off(1);
            engine_brk_live(32 + (b - 7) % 8, 0);
        }
        if (b == 12 || b == 44 || b == 71) {
            int value = b == 44 ? 0 : 96;
            engine_set(T_303, 0, ORIGINAL_SYNTHONE_CRUSH, value);
            for (int part = PART_303A; part <= PART_303B; ++part) {
                engine_set(T_MIX, part, MX_REV, value);
                engine_set(T_MIX, part, MX_DLY, value);
            }
        }
        render(stream, sizes[b % (sizeof sizes / sizeof sizes[0])]);
    }
    assert(mode == SERIAL || jobs);
    assert(mode != INTERRUPTED || drops);
    assert(!fclose(stream));
    printf("worker mode %d, requests %u, between-job interruptions %u\n", mode, jobs, drops);
}
