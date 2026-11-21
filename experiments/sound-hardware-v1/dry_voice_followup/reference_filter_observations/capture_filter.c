/* New comparison-only public-API and file-I/O wrapper. No filter algorithm. */
#include <errno.h>
#include <fenv.h>
#include <float.h>
#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "soundpipe.h"

enum { SAMPLE_RATE = 44100, PREWARM_FRAMES = 4096, RETAINED_FRAMES = 32768 };
_Static_assert(sizeof(float) == 4, "four-byte float required");
_Static_assert(sizeof(SPFLOAT) == 4, "four-byte SPFLOAT required");
_Static_assert(FLT_RADIX == 2 && FLT_MANT_DIG == 24 && FLT_MAX_EXP == 128,
               "IEEE754 binary32 float required");

static float float_from_bits(uint32_t bits) {
    float value;
    memcpy(&value, &bits, sizeof(value));
    return value;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "usage: capture_filter INPUT.f32le NEW_OUTPUT.f32le RESONANCE_BITS\n");
        return 2;
    }
    uint32_t resonance_bits;
    if (strcmp(argv[3], "00000000") == 0) resonance_bits = UINT32_C(0x00000000);
    else if (strcmp(argv[3], "3f666666") == 0) resonance_bits = UINT32_C(0x3f666666);
    else {
        fprintf(stderr, "resonance is outside the authorized two-value contract\n");
        return 2;
    }
    const uint32_t endian_check = 1;
    if (*(const unsigned char *)&endian_check != 1 || fegetround() != FE_TONEAREST) {
        fprintf(stderr, "little-endian host and round-to-nearest mode required\n");
        return 2;
    }

    FILE *input = fopen(argv[1], "rb");
    if (!input) { perror("open input"); return 1; }
    float samples[RETAINED_FRAMES];
    const size_t count = fread(samples, sizeof(float), RETAINED_FRAMES, input);
    const int trailing = fgetc(input);
    const int read_error = ferror(input);
    const int close_error = fclose(input);
    if (count != RETAINED_FRAMES || trailing != EOF || read_error || close_error) {
        fprintf(stderr, "input must contain exactly 32768 binary32 samples\n");
        return 1;
    }
    for (unsigned i = 0; i < RETAINED_FRAMES; ++i) {
        if (!isfinite(samples[i])) {
            fprintf(stderr, "nonfinite input at frame %u\n", i);
            return 1;
        }
    }

    FILE *output = fopen(argv[2], "wbx");
    if (!output) { perror("create new output"); return 1; }
    sp_data context = {0};
    context.sr = SAMPLE_RATE;
    sp_moogladder *filter = NULL;
    unsigned prewarm_completed = 0, retained_completed = 0;
    unsigned prewarm_nonfinite = 0, output_nonfinite = 0;
    int status = 1;
    if (sp_moogladder_create(&filter) != SP_OK || filter == NULL) {
        fprintf(stderr, "original public create call failed\n");
        goto finish;
    }
    if (sp_moogladder_init(&context, filter) != SP_OK) {
        fprintf(stderr, "original public init call failed\n");
        goto finish;
    }
    filter->freq = float_from_bits(UINT32_C(0x44fa0000));
    filter->res = float_from_bits(resonance_bits);

    for (unsigned i = 0; i < PREWARM_FRAMES; ++i) {
        float zero = 0.0f, value = 0.0f;
        if (sp_moogladder_compute(&context, filter, &zero, &value) != SP_OK) {
            fprintf(stderr, "original public compute failed in prewarm frame %u\n", i);
            goto finish;
        }
        ++prewarm_completed;
        if (!isfinite(value)) ++prewarm_nonfinite;
    }
    for (unsigned i = 0; i < RETAINED_FRAMES; ++i) {
        float sample = samples[i], value = 0.0f;
        if (sp_moogladder_compute(&context, filter, &sample, &value) != SP_OK) {
            fprintf(stderr, "original public compute failed in retained frame %u\n", i);
            goto finish;
        }
        ++retained_completed;
        if (!isfinite(value)) ++output_nonfinite;
        if (fwrite(&value, sizeof(value), 1, output) != 1) {
            perror("write output");
            goto finish;
        }
    }
    status = (prewarm_nonfinite == 0 && output_nonfinite == 0) ? 0 : 1;

finish:
    if (fclose(output) != 0) { perror("close output"); status = 1; }
    if (filter != NULL && sp_moogladder_destroy(&filter) != SP_OK) {
        fprintf(stderr, "original public destroy call failed\n");
        status = 1;
    }
    printf("{\"sample_rate_hz\":%u,\"cutoff_f32_hex\":\"44fa0000\","
           "\"resonance_f32_hex\":\"%08" PRIx32 "\","
           "\"initial_state_fresh\":true,\"prewarm_frames\":%u,"
           "\"retained_frames\":%u,\"filter_compute_calls\":%u,"
           "\"input_nonfinite_count\":0,\"prewarm_nonfinite_count\":%u,"
           "\"output_nonfinite_count\":%u,\"status\":%d}\n",
           (unsigned)SAMPLE_RATE, resonance_bits, prewarm_completed, retained_completed,
           prewarm_completed + retained_completed, prewarm_nonfinite,
           output_nonfinite, status);
    return status;
}
