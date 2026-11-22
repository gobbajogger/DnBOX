/* Bounded pre-render scalar/state tests. No oscillator/envelope audio render.
 * Include the actual candidate translation unit so its private solver and
 * substep are tested without adding a public/testing API to the DSP. */
#include "../src/s1_dry_voice.c"
#include "scalar_cases.h"
#include <inttypes.h>
#include <stdio.h>

static uint32_t word(float x)
{ uint32_t value; memcpy(&value, &x, sizeof value); return value; }
static float from_word(uint32_t x)
{ float value; memcpy(&value, &x, sizeof value); return value; }

#define CHECK(test) do { if (!(test)) { \
    fprintf(stderr, "scalar selftest failed at line %d: %s\n", __LINE__, #test); \
    return 1; } } while (0)

/* Frozen B's complete filter substep arithmetic, for the r=0 invariant only.
 * This helper is an independent test comparator; it is absent from the
 * candidate and renderer builds. It does not read reference PCM or an engine. */
static float frozen_b_zero_substep(float history[5], float A, float R, float input)
{
    float tmp = history[3];
    float nonlinear = chicago_nonlinear(history[4]);
    float v = input - R * nonlinear;
    float z1 = v + A * (history[0] - v);
    float z2 = z1 + A * (history[1] - z1);
    float z3 = z2 + A * (history[2] - z2);
    float z4 = z3 + A * (history[3] - z3);
    history[0] = z1; history[1] = z2;
    history[2] = z3; history[3] = z4; history[4] = tmp;
    return z4;
}

static void initial_histories(s1_dry_voice *voice, unsigned index)
{
    if (index == 1) {
        const float fixed[5] = {0.25f, -0.5f, 0.75f, -0.125f, 0.375f};
        memcpy(voice->filter_history, fixed, sizeof fixed);
        voice->feedback_lag3 = -0.625f;
        voice->feedback_lag4 = 0.875f;
    } else if (index == 2) {
        voice->filter_history[1] = from_word(0x80000000u);
        voice->filter_history[3] = from_word(0x80000000u);
        voice->feedback_lag3 = from_word(0x80000000u);
    }
}

int main(void)
{
    unsigned solver_checked = 0, zero_checked = 0, history_checked = 0;
    for (unsigned i = 0; i < SCALAR_CASE_COUNT; ++i) {
        float w = from_word(scalar_cases[i].w), c = from_word(scalar_cases[i].c);
        float result = feedback_solution(w, c);
        if (word(result) != scalar_cases[i].expected) {
            fprintf(stderr, "scalar point %u: actual=%08" PRIx32
                    " expected=%08" PRIx32 "\n", i, word(result), scalar_cases[i].expected);
            return 1;
        }
        CHECK(finite_float(result));
        CHECK(result >= w - c && result <= w + c);
        ++solver_checked;
    }

    const float cutoffs[5] = {64.0f, 500.0f, 2000.0f, 8000.0f, 22050.0f};
    const float inputs[12] = {0.0f, -0.0f, 1.0f, -1.0f, 0.125f, -0.125f,
                             1e-8f, -1e-8f, 0.9f, -0.9f, 0.0f, 0.0f};
    s1_dry_voice untouched, untouched_copy;
    s1_dry_reset(&untouched);
    untouched_copy = untouched;
    for (unsigned fc = 0; fc < 5; ++fc) {
        for (unsigned sign = 0; sign < 2; ++sign) {
            for (unsigned initial = 0; initial < 3; ++initial) {
                s1_dry_voice voice;
                s1_dry_controls p;
                s1_dry_reset(&voice);
                s1_dry_frozen_controls(&p);
                p.cutoff_hz = cutoffs[fc];
                p.resonance = from_word(sign ? 0x80000000u : 0u);
                CHECK(s1_dry_configure(&voice, &p) == S1_DRY_OK);
                initial_histories(&voice, initial);
                float legacy[5];
                memcpy(legacy, voice.filter_history, sizeof legacy);
                for (unsigned i = 0; i < 12; ++i) {
                    float old_h4 = voice.filter_history[4];
                    float old_lag3 = voice.feedback_lag3;
                    float expected = frozen_b_zero_substep(legacy,
                        voice.prepared.pole_retention, p.resonance * 0.8f, inputs[i]);
                    float actual = filter_substep(&voice, inputs[i]);
                    CHECK(word(actual) == word(expected));
                    for (unsigned h = 0; h < 5; ++h)
                        CHECK(word(voice.filter_history[h]) == word(legacy[h]));
                    CHECK(word(voice.feedback_lag3) == word(old_h4));
                    CHECK(word(voice.feedback_lag4) == word(old_lag3));
                    ++zero_checked;
                }
            }
        }
    }

    const float resonances[4] = {0.25f, 0.5f, 0.9f, 0.98f};
    float largest_contraction = 0.0f;
    for (unsigned fc = 0; fc < 5; ++fc) {
        for (unsigned r = 0; r < 4; ++r) {
            for (unsigned initial = 0; initial < 3; ++initial) {
                s1_dry_voice voice;
                s1_dry_controls p;
                s1_dry_reset(&voice);
                s1_dry_frozen_controls(&p);
                p.cutoff_hz = cutoffs[fc]; p.resonance = resonances[r];
                CHECK(s1_dry_configure(&voice, &p) == S1_DRY_OK);
                initial_histories(&voice, initial);
                float b = 1.0f - voice.prepared.pole_retention;
                float b2 = b * b, b4 = b2 * b2;
                float c = (0.0625f * (p.resonance * 0.8f)) * b4;
                float contraction = 5.0f * c;
                CHECK(contraction >= 0.0f && contraction < 0.097f);
                if (contraction > largest_contraction) largest_contraction = contraction;
                for (unsigned i = 0; i < 12; ++i) {
                    float old_h3 = voice.filter_history[3];
                    float old_h4 = voice.filter_history[4];
                    float old_lag3 = voice.feedback_lag3;
                    float actual = filter_substep(&voice, inputs[i]);
                    CHECK(finite_float(actual));
                    CHECK(fm_fabsf(actual) <= 1.7841f);
                    CHECK(word(voice.filter_history[3]) == word(actual));
                    CHECK(word(voice.filter_history[4]) == word(old_h3));
                    CHECK(word(voice.feedback_lag3) == word(old_h4));
                    CHECK(word(voice.feedback_lag4) == word(old_lag3));
                    for (unsigned h = 0; h < 5; ++h)
                        CHECK(finite_float(voice.filter_history[h]));
                    CHECK(memcmp(&untouched, &untouched_copy, sizeof untouched) == 0);
                    ++history_checked;
                }
            }
        }
    }
    printf("{\"status\":\"PASS\",\"frozen_scalar_points_exact\":%u,"
           "\"zero_resonance_B_substeps_exact\":%u,\"history_shift_checks\":%u,"
           "\"maximum_selected_float_contraction\":%.9g,\"voice_bytes\":%zu,"
           "\"note_audio_rendered\":false,\"reference_pcm_read\":false,"
           "\"qualification\":\"selected scalar/state checks only; no audio gate\"}\n",
           solver_checked, zero_checked, history_checked,
           (double)largest_contraction, sizeof(s1_dry_voice));
    return 0;
}
