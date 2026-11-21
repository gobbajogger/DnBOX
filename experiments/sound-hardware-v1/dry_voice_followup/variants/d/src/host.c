/* Host-only bounded renderer and ownership checks. No original engine link. */
#include "s1_dry_voice.h"
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { WARMUP_FRAMES = 176448, FRAMES = 88192, OFF_FRAME = 66112 };

static uint32_t bits(float x)
{ uint32_t value; memcpy(&value, &x, sizeof value); return value; }
static float from_bits(uint32_t x)
{ float value; memcpy(&value, &x, sizeof value); return value; }
static int finite_float(float x)
{ return (bits(x) & 0x7f800000u) != 0x7f800000u; }

static void describe(void)
{
    s1_dry_voice voice;
    s1_dry_controls p;
    s1_dry_reset(&voice);
    s1_dry_frozen_controls(&p);
    (void)s1_dry_configure(&voice, &p);
    printf("{\"voice_bytes\":%zu,\"prepared_bytes\":%zu,"
           "\"history_bytes\":%zu,\"sample_rate_hz\":44100,"
           "\"coefficient_hex\":[\"%08" PRIx32 "\",\"%08" PRIx32
           "\",\"%08" PRIx32 "\",\"%08" PRIx32 "\",\"%08" PRIx32
           "\",\"%08" PRIx32 "\"]}\n",
           sizeof voice, sizeof voice.prepared, sizeof voice - sizeof voice.prepared,
           bits(voice.prepared.amp.attack_retention),
           bits(voice.prepared.amp.decay_retention),
           bits(voice.prepared.amp.release_retention),
           bits(voice.prepared.filter.attack_retention),
           bits(voice.prepared.filter.decay_retention),
           bits(voice.prepared.filter.release_retention));
}

#define CHECK(test) do { if (!(test)) { \
    fprintf(stderr, "selftest failed at line %d: %s\n", __LINE__, #test); \
    return 1; } } while (0)

static int selftest(void)
{
    s1_dry_voice a, b, copy;
    s1_dry_controls p, bad;
    float out[64];
    s1_dry_reset(&a);
    s1_dry_frozen_controls(&p);
    CHECK(s1_dry_configure(&a, &p) == 0);
    copy = a;
    bad = p; bad.cutoff_hz = from_bits(0x7fc00000u);
    CHECK(s1_dry_configure(&a, &bad) == S1_DRY_RANGE);
    CHECK(memcmp(&a, &copy, sizeof a) == 0);
    bad = p; bad.resonance = from_bits(0x7f800000u);
    CHECK(s1_dry_configure(&a, &bad) == S1_DRY_RANGE);
    CHECK(memcmp(&a, &copy, sizeof a) == 0);
    bad = p; bad.filter_depth = 1.2001f;
    CHECK(s1_dry_configure(&a, &bad) == S1_DRY_RANGE);
    CHECK(memcmp(&a, &copy, sizeof a) == 0);
    /* Full physical endpoints, including valid depth 1.2 and cutoff 22050. */
    bad = p; bad.cutoff_hz = 22050.0f; bad.resonance = 0.98f;
    bad.filter_depth = 1.2f;
    CHECK(s1_dry_configure(&a, &bad) == 0);
    CHECK(a.prepared.controls.cutoff_hz == 22050.0f);
    CHECK(a.prepared.controls.filter_depth == 1.2f);
    CHECK(s1_dry_cutoff(2000.0f, 1.2f, 0.0f) == 64.0f);
    float mapped = s1_dry_cutoff(2000.0f, 1.2f, 0.4f);
    CHECK(mapped > 559.999f && mapped < 560.001f);
    CHECK(s1_dry_cutoff(22050.0f, 0.0f, 0.0f) == 22050.0f);
    CHECK(s1_dry_configure(&a, &p) == 0);
    CHECK(s1_dry_start(&a, from_bits(0x4382d013u), 100, 0.0f) == 0);
    CHECK(bits(a.velocity_squared) == 0x3f1eb866u);
    copy = a;
    CHECK(s1_dry_start(&a, 440.0f, 100, 0.0f) == S1_DRY_UNSUPPORTED);
    CHECK(memcmp(&a, &copy, sizeof a) == 0);
    CHECK(s1_dry_render(&a, out, 65) == S1_DRY_RANGE);
    CHECK(memcmp(&a, &copy, sizeof a) == 0);
    /* Pre-render selftest: configuration/gate/ownership only. No note audio
     * is generated. Scalar feedback checks are a separate test executable. */
    b = a;
    CHECK(s1_dry_note_off(&a) == 0);
    CHECK(memcmp(&b, &copy, sizeof b) == 0);
    CHECK(b.gate == 1 && a.gate == 0);
    CHECK(s1_dry_start(&a, 440.0f, 0, 0.0f) == 0);
    CHECK(a.gate == 0);
    a.feedback_lag3 = 0.25f;
    a.feedback_lag4 = -0.5f;
    s1_dry_reset(&a);
    CHECK(bits(a.feedback_lag3) == 0 && bits(a.feedback_lag4) == 0);
    CHECK(sizeof a == 160 && sizeof a.prepared == 92);
    puts("{\"status\":\"PASS\",\"checks\":[\"finite physical input validation\","
         "\"invalid changes leave state untouched\",\"full cutoff/depth endpoints\","
         "\"squared velocity\",\"independent owners\",\"zero initialized extra histories\","
         "\"160 byte object\",\"explicit unsupported retrigger\"],\"note_audio_rendered\":false}");
    return 0;
}

static int parse_float(const char *text, float *value)
{
    char *end = 0;
    errno = 0;
    *value = strtof(text, &end);
    return !errno && end && end != text && !*end;
}

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "--describe") == 0) { describe(); return 0; }
    if (argc == 2 && strcmp(argv[1], "--selftest") == 0) return selftest();
    if (argc != 6) {
        fprintf(stderr, "usage: %s OUTPUT VELOCITY CUTOFF_HZ RESONANCE DEPTH\n", argv[0]);
        return 2;
    }
    uint32_t endian = 1;
    if (*(unsigned char *)&endian != 1) { fputs("little endian host required\n", stderr); return 2; }
    char *end = 0;
    errno = 0;
    unsigned long velocity = strtoul(argv[2], &end, 10);
    if (errno || !end || end == argv[2] || *end || velocity > 127) return 2;
    s1_dry_controls p;
    s1_dry_frozen_controls(&p);
    if (!parse_float(argv[3], &p.cutoff_hz) || !parse_float(argv[4], &p.resonance) ||
        !parse_float(argv[5], &p.filter_depth)) return 2;
    s1_dry_voice voice;
    s1_dry_reset(&voice);
    if (s1_dry_configure(&voice, &p) != 0) return 2;
    FILE *output = fopen(argv[1], "wbx");
    if (!output) { perror("new output"); return 2; }
    const uint32_t frequencies[3] = {0x4282d013u, 0x4382d013u, 0x4482d013u};
    float mono[64];
    unsigned nonfinite = 0;
    for (unsigned note = 0; note < 3; ++note) {
        s1_dry_reset(&voice);
        if (s1_dry_configure(&voice, &p) != 0) return 2;
        for (unsigned i = 0; i < WARMUP_FRAMES; i += 64) {
            if (s1_dry_render(&voice, mono, 64) != 0) return 2;
            for (unsigned j = 0; j < 64; ++j) if (mono[j] != 0.0f) return 2;
        }
        if (s1_dry_start(&voice, from_bits(frequencies[note]), (unsigned)velocity, 0.0f) != 0)
            return 2;
        for (unsigned frame = 0; frame < FRAMES; frame += 64) {
            if (frame == OFF_FRAME && s1_dry_note_off(&voice) != 0) return 2;
            if (s1_dry_render(&voice, mono, 64) != 0) return 2;
            for (unsigned j = 0; j < 64; ++j) nonfinite += !finite_float(mono[j]);
            if (fwrite(mono, sizeof(float), 64, output) != 64) return 2;
        }
    }
    if (fclose(output) != 0) return 2;
    printf("{\"frames_per_note\":88192,\"notes\":[36,60,84],"
           "\"warmup_frames\":176448,\"off_frame\":66112,"
           "\"nonfinite\":%u,\"phase_at_note_on\":0,\"comparison_gain\":1}\n", nonfinite);
    return nonfinite ? 1 : 0;
}
