/* Matched filter-only wrapper around frozen C; no candidate DSP edits.
 * The include is resolved only from the pinned frozen C src directory. */
#include "s1_dry_voice.c"
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

enum { PREWARM = 4096, RETAINED = 32768, STEADY_START = 16384 };

typedef struct { double sum_squares, peak_abs; uint32_t count; } diagnostic_level;

static void observe(diagnostic_level *level, double x)
{
    double magnitude = x < 0.0 ? -x : x;
    level->sum_squares += x * x;
    if (magnitude > level->peak_abs) level->peak_abs = magnitude;
    ++level->count;
}

static float substep_and_reconstruct(s1_dry_voice *voice, float input, double *estimate)
{
    /* These read-only copies are outside frozen DSP. Reconstruct the centered
     * argument from the committed output and old lags in host double precision.
     * This is NOT the exact six-step solver d and never feeds the DSP. */
    double old1 = (double)voice->filter_history[3];
    double old2 = (double)voice->filter_history[4];
    double old3 = (double)voice->feedback_lag3;
    double old4 = (double)voice->feedback_lag4;
    float y = filter_substep(voice, input);
    *estimate = ((double)y + 4.0 * old1 + 6.0 * old2 + 4.0 * old3 + old4) / 16.0;
    return y;
}

static float filter_output(s1_dry_voice *voice, float input,
                          double *first_estimate, double *second_estimate)
{
    float first = substep_and_reconstruct(voice, input, first_estimate);
    float second = substep_and_reconstruct(voice, input, second_estimate);
    return 0.5f * (first + second);
}

int main(int argc, char **argv)
{
    if (argc != 4) {
        fprintf(stderr, "usage: %s INPUT OUTPUT RESONANCE_F32_HEX\n", argv[0]);
        return 2;
    }
    uint32_t little_endian = 1;
    if (*(unsigned char *)&little_endian != 1) return 2;
    char *end = 0;
    errno = 0;
    unsigned long parsed = strtoul(argv[3], &end, 16);
    if (errno || !end || end == argv[3] || *end ||
        (parsed != 0ul && parsed != 0x3f666666ul)) return 2;
    uint32_t resonance_word = (uint32_t)parsed;
    float resonance;
    memcpy(&resonance, &resonance_word, sizeof resonance);
    FILE *input = fopen(argv[1], "rb");
    if (!input) { perror("input"); return 2; }
    if (fseek(input, 0, SEEK_END) != 0 || ftell(input) != RETAINED * 4L ||
        fseek(input, 0, SEEK_SET) != 0) { fclose(input); return 2; }

    s1_dry_voice voice;
    s1_dry_controls controls;
    s1_dry_reset(&voice);
    s1_dry_frozen_controls(&controls);
    controls.cutoff_hz = 2000.0f;
    controls.resonance = resonance;
    controls.filter_depth = 0.0f;
    if (s1_dry_configure(&voice, &controls) != S1_DRY_OK) { fclose(input); return 2; }
    const s1_dry_voice initial = voice;
    unsigned prewarm_nonfinite = 0, nonfinite = 0;
    float prewarm_peak = 0.0f;
    for (unsigned frame = 0; frame < PREWARM; ++frame) {
        double estimate1, estimate2;
        float sample = filter_output(&voice, 0.0f, &estimate1, &estimate2);
        prewarm_nonfinite += !finite_float(sample);
        float magnitude = fm_fabsf(sample);
        if (magnitude > prewarm_peak) prewarm_peak = magnitude;
    }
    FILE *output = fopen(argv[2], "wbx");
    if (!output) { perror("new output"); fclose(input); return 2; }
    diagnostic_level all = {0}, steady = {0};
    float block[64], rendered[64];
    for (unsigned start = 0; start < RETAINED; start += 64) {
        if (fread(block, sizeof(float), 64, input) != 64) return 2;
        for (unsigned i = 0; i < 64; ++i) {
            if (!finite_float(block[i])) return 2;
            double estimate1, estimate2;
            rendered[i] = filter_output(&voice, block[i], &estimate1, &estimate2);
            nonfinite += !finite_float(rendered[i]);
            observe(&all, estimate1); observe(&all, estimate2);
            if (start + i >= STEADY_START) {
                observe(&steady, estimate1); observe(&steady, estimate2);
            }
        }
        if (fwrite(rendered, sizeof(float), 64, output) != 64) return 2;
    }
    if (fclose(input) != 0 || fclose(output) != 0) return 2;
    int other_state_unchanged =
        memcmp(&initial.amp, &voice.amp, sizeof voice.amp) == 0 &&
        memcmp(&initial.filter, &voice.filter, sizeof voice.filter) == 0 &&
        memcmp(&initial.prepared, &voice.prepared, sizeof voice.prepared) == 0 &&
        voice.phase == initial.phase && voice.phase_increment == initial.phase_increment &&
        voice.velocity_squared == initial.velocity_squared && voice.gate == initial.gate &&
        voice.started == initial.started;
    printf("{\"scope\":\"frozen C filter only\",\"sample_rate_hz\":44100,"
           "\"cutoff_hz\":2000,\"resonance_f32_hex\":\"%08" PRIx32 "\","
           "\"prewarm_frames\":4096,\"retained_frames\":32768,"
           "\"filter_substeps\":73728,\"steady_window\":[16384,32768],"
           "\"prewarm_nonfinite\":%u,\"output_nonfinite\":%u,\"prewarm_peak\":%.9g,"
           "\"oscillator_envelopes_prepared_controls_unchanged\":%s,"
           "\"feedback_argument_classification\":\"reconstructed from current output and old lags; NOT exact traced solver d\","
           "\"reconstructed_all\":{\"count\":%u,\"sum_squares\":%.17g,\"peak_abs\":%.17g},"
           "\"reconstructed_steady\":{\"count\":%u,\"sum_squares\":%.17g,\"peak_abs\":%.17g}}\n",
           resonance_word, prewarm_nonfinite, nonfinite, (double)prewarm_peak,
           other_state_unchanged ? "true" : "false",
           all.count, all.sum_squares, all.peak_abs,
           steady.count, steady.sum_squares, steady.peak_abs);
    return prewarm_nonfinite || nonfinite || !other_state_unchanged ? 1 : 0;
}
