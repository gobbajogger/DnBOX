/* Host-only diagnostic wrapper for frozen D. No DSP implementation change.
 * Static mode replays physical-gain/100_2000_.9. Active mode changes only
 * filter ADSR to .2/.3/.4/.5 and depth to 1.2, from the source-free contract.
 * Tuning, reset, warmup, note events and sample-writing loop are copied from
 * frozen D's host.c. No original-engine source, header or object is linked. */
#include "s1_dry_voice.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

enum { WARMUP_FRAMES = 176448, FRAMES = 88192, OFF_FRAME = 66112 };

static uint32_t bits(float x)
{ uint32_t value; memcpy(&value, &x, sizeof value); return value; }
static float from_bits(uint32_t x)
{ float value; memcpy(&value, &x, sizeof value); return value; }
static int finite_float(float x)
{ return (bits(x) & 0x7f800000u) != 0x7f800000u; }

int main(int argc, char **argv)
{
    if (argc != 3 || (strcmp(argv[1], "static") != 0 &&
                      strcmp(argv[1], "active") != 0)) {
        fprintf(stderr, "usage: %s static|active NEW_OUTPUT.f32le\n", argv[0]);
        return 2;
    }
    int active = strcmp(argv[1], "active") == 0;
    uint32_t endian = 1;
    if (*(unsigned char *)&endian != 1) { fputs("little endian host required\n", stderr); return 2; }
    unsigned long velocity = 100;
    s1_dry_controls p;
    s1_dry_frozen_controls(&p);
    if (active) {
        p.filter = (s1_dry_adsr){0.2f, 0.3f, 0.4f, 0.5f};
        p.filter_depth = 1.2f;
    }
    /* Fail explicitly if a different frozen core/default contract is linked. */
    if (bits(p.cutoff_hz) != 0x44fa0000u || bits(p.resonance) != 0x3f666666u ||
        bits(p.amp.attack_s) != 0x3d4ccccdu || bits(p.amp.decay_s) != 0x3ba3d70au ||
        bits(p.amp.sustain) != 0x3f4ccccdu || bits(p.amp.release_s) != 0x3d4ccccdu ||
        bits(p.oscillator1_level) != 0x3f800000u || bits(p.balance) != 0x3f000000u ||
        bits(p.filter.attack_s) != (active ? 0x3e4ccccdu : 0x3d4ccccdu) ||
        bits(p.filter.decay_s) != (active ? 0x3e99999au : 0x3d4ccccdu) ||
        bits(p.filter.sustain) != (active ? 0x3ecccccdu : 0x3f800000u) ||
        bits(p.filter.release_s) != 0x3f000000u ||
        bits(p.filter_depth) != (active ? 0x3f99999au : 0x00000000u)) {
        fputs("frozen physical contract mismatch\n", stderr);
        return 2;
    }
    s1_dry_voice voice;
    s1_dry_reset(&voice);
    if (s1_dry_configure(&voice, &p) != 0) return 2;
    FILE *output = fopen(argv[2], "wbx");
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
    printf("{\"mode\":\"%s\",\"frames_per_note\":88192,\"notes\":[36,60,84],"
           "\"warmup_frames\":176448,\"off_frame\":66112,\"sample_rate_hz\":44100,"
           "\"nonfinite\":%u,\"phase_at_note_on\":0,\"comparison_gain\":1,"
           "\"velocity\":100,\"cutoff_hex\":\"%08" PRIx32 "\","
           "\"resonance_hex\":\"%08" PRIx32 "\",\"depth_hex\":\"%08" PRIx32 "\","
           "\"filter_ADSR_hex\":[\"%08" PRIx32 "\",\"%08" PRIx32
           "\",\"%08" PRIx32 "\",\"%08" PRIx32 "\"]}\n",
           active ? "active" : "static", nonfinite, bits(p.cutoff_hz),
           bits(p.resonance), bits(p.filter_depth), bits(p.filter.attack_s),
           bits(p.filter.decay_s), bits(p.filter.sustain), bits(p.filter.release_s));
    return nonfinite ? 1 : 0;
}
