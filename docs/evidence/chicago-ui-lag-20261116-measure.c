/* Scratch host measurement for the Chicago AMP/ARP restore fix.
 * Not product firmware. Compiled against one tree at a time.
 * Counts original_sound_restore_chicago / program_init per ui_frame
 * and records host CLOCK_MONOTONIC time. This is not device latency
 * and not an emu360 result. */
#define _POSIX_C_SOURCE 200809L
#define main simulator_main
/* Compile with -DX0X_HOST_C='"/absolute/path/host/x0x_host.c"' so the
 * included app is the tree under measurement, not this file's directory. */
#ifndef X0X_HOST_C
#error "pass -DX0X_HOST_C=\\\"/path/to/host/x0x_host.c\\\""
#endif
#include X0X_HOST_C
#undef main

static uint32_t g_restore_chi, g_chi_init;

int __real_original_sound_restore_chicago(void *out, const uint8_t *cell);
int __wrap_original_sound_restore_chicago(void *out, const uint8_t *cell)
{
    g_restore_chi++;
    return __real_original_sound_restore_chicago(out, cell);
}

int __real_original_chicago_program_init(void *p, unsigned factory);
int __wrap_original_chicago_program_init(void *p, unsigned factory)
{
    g_chi_init++;
    return __real_original_chicago_program_init(p, factory);
}

static int adopt(int voice, int bank)
{
    if (engine_bass_select_preset(voice, bank, 0, &proj.sound)) return -1;
    for (int i = 0; i < 8000 && engine_bass_preset_pending(voice); i++) {
        int32_t a[512];
        engine_render(a, 256);
    }
    return (engine_bass_sound_bank(&proj.sound, voice) == bank &&
            engine_bass_bank(voice) == bank) ? 0 : -1;
}

static void settle(int part, int page)
{
    ui.view = V_PART;
    ui.part = (uint8_t)part;
    ui.overlay = O_NONE;
    ui.help = 0;
    ui.touched = -1;
    ui.touch_until = ui.msg_until = 0;
    ui.msg[0] = 0;
    ui.held_step = ui.hold_step = -1;
    ui.btn = ui.keys = ui.rec = 0;
    ui.page[V_PART][part] = (uint8_t)page;
    set_view(V_PART);
    ui.page[V_PART][part] = (uint8_t)page;
    build_pages();
    *page_of(V_PART) = (uint8_t)page;
}

static uint64_t nsec(void)
{
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (uint64_t)t.tv_sec * 1000000000ull + (uint64_t)t.tv_nsec;
}

static void one_frame(int part, int page, const char *tag, int index)
{
    uint64_t t0, t1;
    g_restore_chi = g_chi_init = 0;
    t0 = nsec();
    now_ms += 16;
    ui_frame();
    t1 = nsec();
    printf("FRAME label=%s part=%d page=%d %s=%d restore=%u init=%u ns=%llu\n",
           MEASURE_LABEL, part, page, tag, index,
           g_restore_chi, g_chi_init,
           (unsigned long long)(t1 - t0));
}

int main(void)
{
    load_force = 0;
    boot();
    now_ms = 1000;
    proj.set.autosave_off = 1;
    printf("MEASURE_BEGIN label=%s\n", MEASURE_LABEL);
    if (adopt(0, ORIGINAL_SOUND_MIAMI) || adopt(1, ORIGINAL_SOUND_CHICAGO)) {
        printf("ADOPT_FAIL\n");
        return 1;
    }
    {
        pref_t r = PR(R_ENG, T_303, 1, 11);
        char a[16], u[8];
        original_controls.valid = 0;
        g_restore_chi = g_chi_init = 0;
        pref_value_of(r, 0, a, u);
        printf("WRAP_PROBE restore=%u init=%u\n", g_restore_chi, g_chi_init);
        if (!g_restore_chi || !g_chi_init) {
            puts("WRAP_INACTIVE");
            return 2;
        }
    }
    for (int p = 0; p < 8; p++) {
        settle(PART_303B, p);
        original_controls.valid = 0;
        one_frame(PART_303B, p, "cold", 0);
        for (int rep = 0; rep < 3; rep++) {
            for (int f = 0; f < 8; f++)
                one_frame(PART_303B, p, "steady", rep * 8 + f);
        }
    }
    for (int p = 0; p < 6; p++) {
        settle(PART_303A, p);
        original_controls.valid = 0;
        one_frame(PART_303A, p, "cold", 0);
        one_frame(PART_303A, p, "steady", 0);
    }
    printf("MEASURE_END label=%s\n", MEASURE_LABEL);
    return 0;
}
