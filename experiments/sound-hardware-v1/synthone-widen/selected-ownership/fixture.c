/* SPDX-License-Identifier: GPL-3.0-only */
#define main simulator_main
#include "host/x0x_host.c"
#undef main

#define CHECK(x) do { if (!(x)) { fprintf(stderr, "failed at line %d: %s\n", __LINE__, #x); exit(2); } } while (0)
enum { BLOCK = 256, DELAY = 2205 };
static FILE *audio;
static unsigned blocks;
static int audition;
#ifdef X0X_WIDEN_PROBE
extern int widen_probe_set(int v, float wet);
static struct {
    uint32_t raw[DELAY]; /* Independent 32-bit floating-point-word history. */
    unsigned pos, elapsed, resets, frames, wet_frames, dry_frames, taps, audible;
    uint32_t input_hash;
} oracle[2];

static uint32_t bits(float x)
{
    uint32_t word;
    memcpy(&word, &x, sizeof word);
    return word;
}
void widen_probe_reset(int v)
{
    memset(oracle[v].raw, 0, sizeof oracle[v].raw);
    oracle[v].pos = oracle[v].elapsed = 0;
    ++oracle[v].resets;
}

void widen_probe_frame(int v, float input, float delayed, float output, float wet)
{
    CHECK(oracle[v].resets > 0);
    unsigned pos = oracle[v].pos;
    uint32_t expected = oracle[v].raw[pos] & UINT32_C(0xffffff00);
    CHECK(expected == bits(delayed));
    if (oracle[v].elapsed < DELAY) CHECK(expected == 0);
    float t;
    memcpy(&t, &expected, sizeof t);
    float y = wet * t + (1.f - wet) * input;
    if (wet == 0.f) {
        CHECK(bits(output) == bits(input));
        ++oracle[v].dry_frames;
    } else {
        CHECK(bits(y) == bits(output));
        ++oracle[v].wet_frames;
        if (bits(t) != bits(input) && fabsf(t) > .00001f) ++oracle[v].audible;
    }
    if (fabsf(t) > .00001f) ++oracle[v].taps;
    oracle[v].raw[pos] = bits(input);
    oracle[v].input_hash = oracle[v].input_hash * UINT32_C(16777619) ^ bits(input);
    oracle[v].pos = (pos + 1) % DELAY;
    ++oracle[v].elapsed;
    ++oracle[v].frames;
}
static void mix(int v, float x) { CHECK(widen_probe_set(v, x) == 1); }
static void denied(int v) { CHECK(widen_probe_set(v, 1.f) == 0); }
#else
static void mix(int v, float x) { (void)v; (void)x; }
static void denied(int v) { (void)v; }
#endif

static void render(unsigned n)
{
    int32_t pcm[BLOCK*2];
    for (unsigned j = 0; j < n; ++j) {
        engine_render(pcm, BLOCK);
        CHECK(fwrite(pcm, sizeof pcm, 1, audio) == 1);
        ++blocks;
    }
}

static void finish_select(int v, int bank)
{
    for (unsigned n = 0; n < 700 && engine_bass_preset_pending(v); ++n) render(1);
    CHECK(engine_bass_bank(v) == bank && !engine_bass_preset_pending(v));
}
static void select_bank(int v, int bank)
{
    CHECK(engine_bass_select_preset(v, bank, 0, &proj.sound) == 0);
    finish_select(v, bank);
}
static void note(int v, int key, int velocity)
{
    uint32_t generation = 0, context = 0;
    int arp = 0;
    engine_live_key_state(v, &generation, &arp, &context);
    CHECK(engine_live_key(v, generation, 2, key, velocity, 0, 0));
}

int main(int argc, char **argv)
{
    CHECK(argc == 4);
    audition = atoi(argv[1]);
    audio = fopen(argv[2], "wb");
    CHECK(audio);
    boot();
    run_ms(50);
    denied(-1); denied(2);
    for (int v = 0; v < 2; ++v) {
        select_bank(v, ORIGINAL_SOUND_CHICAGO);
        denied(v);
        CHECK(engine_bass_select_preset(v, ORIGINAL_SOUND_SYNTHONE, 255, &proj.sound) < 0);
        denied(v);
        CHECK(engine_bass_select_preset(v, ORIGINAL_SOUND_SYNTHONE, 0, &proj.sound) == 0);
        denied(v); /* request alone cannot transfer arena ownership */
        finish_select(v, ORIGINAL_SOUND_SYNTHONE);
    }
#ifdef X0X_WIDEN_PROBE
    CHECK(!widen_probe_set(0, -1.f) && !widen_probe_set(1, 2.f));
    unsigned cold_b = oracle[1].resets;
#endif
    note(0, 47, 99); note(1, 62, 83);
    render(17); /* warm both parts while fully dry */
#ifdef X0X_WIDEN_PROBE
    CHECK(oracle[0].taps > 512 && oracle[1].taps > 512);
    CHECK(oracle[0].input_hash != oracle[1].input_hash);
    unsigned a0 = oracle[0].audible, b0 = oracle[1].audible;
#endif
    mix(0, audition ? 1.f : 0.f);
    render(1);
#ifdef X0X_WIDEN_PROBE
    if (audition) CHECK(oracle[0].audible - a0 > 200 && oracle[1].audible == b0);
#endif
    mix(1, audition ? .5f : 0.f);
    render(12);
#ifdef X0X_WIDEN_PROBE
    if (audition) CHECK(oracle[1].audible - b0 > 2500);
#endif
    note(0, 47, 0);
    select_bank(0, ORIGINAL_SOUND_CHICAGO);
    denied(0);
#ifdef X0X_WIDEN_PROBE
    unsigned a_frames = oracle[0].frames, b_frames = oracle[1].frames;
#endif
    note(0, 53, 92); render(12); note(0, 53, 0); render(30);
    select_bank(0, ORIGINAL_SOUND_MIAMI); denied(0); render(4);
    select_bank(0, ORIGINAL_SOUND_JUNO); denied(0); render(4);
#ifdef X0X_WIDEN_PROBE
    CHECK(oracle[0].frames == a_frames);
    CHECK(oracle[1].frames >= b_frames + 50*BLOCK && oracle[1].resets == cold_b);
    unsigned cold_a = oracle[0].resets;
#endif
    select_bank(0, ORIGINAL_SOUND_SYNTHONE);
#ifdef X0X_WIDEN_PROBE
    CHECK(oracle[0].resets == cold_a+1 && oracle[0].elapsed <= BLOCK);
    CHECK(oracle[1].resets == cold_b);
#endif
    mix(0, audition ? 1.f : 0.f); note(0, 75, 105); render(16);
    note(0, 75, 0); note(1, 62, 0); render(36);
    CHECK(fclose(audio) == 0);
    FILE *project = fopen(argv[3], "wb");
    CHECK(project && fwrite(&proj, sizeof proj, 1, project) == 1 && fclose(project) == 0);
    printf("blocks=%u project_bytes=%zu\n", blocks, sizeof proj);
#ifdef X0X_WIDEN_PROBE
    for (int v = 0; v < 2; ++v) {
        printf("part=%d checked=%u dry=%u wet=%u nonzero_taps=%u audible_wet=%u resets=%u input_hash=%08x\n",
               v, oracle[v].frames, oracle[v].dry_frames, oracle[v].wet_frames,
               oracle[v].taps, oracle[v].audible, oracle[v].resets, oracle[v].input_hash);
    }
#endif
    return 0;
}
