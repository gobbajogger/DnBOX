/* SPDX-License-Identifier: GPL-3.0-only */
#include "s1_dry_voice.h"
#include "fastmath.h"
#include <string.h>

/* Source provenance: raw phase/saw from the allowed DnBOX Juno/core;
 * envelope component is reused unchanged; four sequential poles and nonlinear
 * feedback are extracted from the allowed DnBOX chicago_mp4 component only.
 * Variant D retains C's two fixed substeps, physical R=.8*resonance and
 * centered weights (1,4,6,4,1)/16. Only nonzero-resonance feedback changes
 * to phi(d)=5*d with d=w/(1+5*c), the permitted primitive's unfitted slope
 * at zero. Its complete r=0 branch preserves C/B arithmetic. The measured
 * nominal-range hypothesis and limitations are in the frozen diagnosis.
 * Its original coefficient tables, ramps, postgain, kick, oscillators and other
 * engine code are NOT included. This analytic centered-feedback correction
 * remains an experiment, not a claim of S1 filter equivalence. */

_Static_assert(sizeof(float) == 4, "binary32 storage required");
_Static_assert(sizeof(s1_dry_voice) == 160, "bounded single-voice state changed");

static int finite_float(float x)
{
    fm_bits_t bits;
    bits.f = x;
    return (bits.u & 0x7f800000u) != 0x7f800000u;
}

static int range(float x, float lo, float hi)
{ return finite_float(x) && x >= lo && x <= hi; }

static int valid_adsr(const s1_dry_adsr *p, float release_min)
{
    return range(p->attack_s, 0.0005f, 2.0f) &&
           range(p->decay_s, 0.005f, 2.0f) &&
           range(p->sustain, 0.0f, 1.0f) &&
           range(p->release_s, release_min, 2.0f);
}

/* Existing float-only DnBOX math, outside the sample loop. Exactness for these
 * frozen ADSR coefficients is independently checked against the numeric
 * contract by the host test. Other physical values remain unqualified. */
static float retention(float seconds, float scale)
{
    if (seconds == 0.0f) return 0.0f;
    float denominator = S1_DRY_SAMPLE_RATE * seconds;
    denominator *= scale;
    return fm_expf(-1.0f / denominator);
}

static void prepare_adsr(dnbx_s1_envelope_params *out, const s1_dry_adsr *in)
{
    (void)dnbx_s1_envelope_configure(out, retention(in->attack_s, 0.6f),
        retention(in->decay_s, 1.0f), in->sustain,
        retention(in->release_s, 1.0f));
}

static void prepare_filter(s1_dry_prepared *p, float cutoff_hz)
{
    /* Analytic one-pole retention, full legal requested cutoff range. The
     * four-stage recurrence's transfer is measured, not equated to S1. */
    p->pole_retention = fm_expf(-FM_TWO_PI * cutoff_hz / (2.0f * S1_DRY_SAMPLE_RATE));
    p->cached_cutoff_hz = cutoff_hz;
}

void s1_dry_reset(s1_dry_voice *voice)
{
    if (voice) memset(voice, 0, sizeof *voice);
}

void s1_dry_frozen_controls(s1_dry_controls *p)
{
    if (!p) return;
    *p = (s1_dry_controls){
        {0.05f, 0.005f, 0.8f, 0.05f},
        {0.05f, 0.05f, 1.0f, 0.5f},
        2000.0f, 0.9f, 0.0f, 1.0f, 0.5f
    };
}

int s1_dry_configure(s1_dry_voice *voice, const s1_dry_controls *controls)
{
    if (!voice || !controls) return S1_DRY_ARGUMENT;
    if (!valid_adsr(&controls->amp, 0.004f) ||
        !valid_adsr(&controls->filter, 0.0f) ||
        !range(controls->cutoff_hz, 64.0f, 22050.0f) ||
        !range(controls->resonance, 0.0f, 0.98f) ||
        !range(controls->filter_depth, 0.0f, 1.2f) ||
        !range(controls->oscillator1_level, 0.0f, 1.0f) ||
        !range(controls->balance, 0.0f, 1.0f)) return S1_DRY_RANGE;
    s1_dry_prepared next = {0};
    next.controls = *controls;
    prepare_adsr(&next.amp, &controls->amp);
    prepare_adsr(&next.filter, &controls->filter);
    prepare_filter(&next, s1_dry_cutoff(controls->cutoff_hz,
        controls->filter_depth, voice->filter.level));
    voice->prepared = next;
    voice->configured = 1;
    return S1_DRY_OK;
}

int s1_dry_start(s1_dry_voice *voice, float frequency_hz,
                 unsigned velocity, float phase)
{
    if (!voice || !voice->configured) return S1_DRY_ARGUMENT;
    if (velocity > 127 || !range(frequency_hz, 0.0f, 22050.0f) ||
        frequency_hz == 0.0f || !range(phase, 0.0f, 1.0f) || phase == 1.0f)
        return S1_DRY_RANGE;
    if (!velocity) return s1_dry_note_off(voice);
    if (voice->started) return S1_DRY_UNSUPPORTED;
    float fraction = (float)velocity / 127.0f;
    voice->velocity_squared = fraction * fraction;
    voice->phase = phase;
    voice->phase_increment = frequency_hz / S1_DRY_SAMPLE_RATE;
    voice->gate = voice->started = 1;
    return S1_DRY_OK;
}

int s1_dry_note_off(s1_dry_voice *voice)
{
    if (!voice || !voice->configured) return S1_DRY_ARGUMENT;
    voice->gate = 0;
    return S1_DRY_OK;
}

float s1_dry_cutoff(float base_hz, float depth, float envelope)
{
    return fm_clampf(base_hz * (1.0f - depth * (1.0f - envelope)),
                    64.0f, 22050.0f);
}

static float chicago_nonlinear(float x)
{
    /* The existing DnBOX Chicago feedback primitive, unchanged arithmetic. */
    float magnitude = fm_fabsf(x);
    float u = magnitude >= 1.0f ? 0.0f : 1.0f - magnitude;
    float u2 = u * u, u4 = u2 * u2, u5 = u * u4;
    float result = 1.0f - u5;
    return x >= 0.0f ? result : -result;
}

static float feedback_solution(float w, float c)
{
    /* Nominal-range linear feedback: d + c*(5*d) = w. The full legal
     * coefficient domain gives 1 <= 1+5*c < 1.101. Frozen-coefficient
     * real-arithmetic stability does not qualify arbitrary modulation. */
    return w / (1.0f + 5.0f * c);
}

static float filter_substep(s1_dry_voice *voice, float input)
{
    float *history = voice->filter_history;
    float A = voice->prepared.pole_retention;
    float R = voice->prepared.controls.resonance * 0.8f;

    if (R == 0.0f) {
        /* B's audio arithmetic is copied exactly, including the nonlinear
         * evaluation and multiplication by zero. New history assignments
         * cannot alter the branch's arithmetic or signed-zero behavior. */
        float tmp = history[3];
        float nonlinear = chicago_nonlinear(history[4]);
        float v = input - R * nonlinear;
        float z1 = v + A * (history[0] - v);
        float z2 = z1 + A * (history[1] - z1);
        float z3 = z2 + A * (history[2] - z2);
        float z4 = z3 + A * (history[3] - z3);
        voice->feedback_lag4 = voice->feedback_lag3;
        voice->feedback_lag3 = history[4];
        history[0] = z1; history[1] = z2;
        history[2] = z3; history[3] = z4; history[4] = tmp;
        return z4;
    }

    /* Compute the forward cascade's current zero-feedback output without
     * committing any state. The same stock forward arithmetic is retained. */
    float q1 = input + A * (history[0] - input);
    float q2 = q1 + A * (history[1] - q1);
    float q3 = q2 + A * (history[2] - q2);
    float q = q3 + A * (history[3] - q3);
    float b = 1.0f - A;
    float b2 = b * b;
    float b4 = b2 * b2;

    /* d=(y_current+4*y[-1]+6*y[-2]+4*y[-3]+y[-4])/16. In real
     * arithmetic y_current=q-R*b4*phi(d), hence d+c*phi(d)=w. */
    float old_weighted = 4.0f * history[3];
    old_weighted = old_weighted + 6.0f * history[4];
    old_weighted = old_weighted + 4.0f * voice->feedback_lag3;
    old_weighted = old_weighted + voice->feedback_lag4;
    float w = 0.0625f * (q + old_weighted);
    float c = (0.0625f * R) * b4;
    float d = feedback_solution(w, c);

    /* Commit each forward pole once. Shift old output lags before any old
     * history[3]/history[4] is overwritten. No output gain or tail rule. */
    float tmp = history[3];
    float nonlinear = 5.0f * d;
    float v = input - R * nonlinear;
    float z1 = v + A * (history[0] - v);
    float z2 = z1 + A * (history[1] - z1);
    float z3 = z2 + A * (history[2] - z2);
    float z4 = z3 + A * (history[3] - z3);
    voice->feedback_lag4 = voice->feedback_lag3;
    voice->feedback_lag3 = history[4];
    history[0] = z1; history[1] = z2;
    history[2] = z3; history[3] = z4; history[4] = tmp;
    return z4;
}

float s1_dry_sample(s1_dry_voice *voice)
{
    if (!voice->started) return 0.0f;
    float amp = dnbx_s1_envelope_sample(&voice->amp, &voice->prepared.amp,
                                      voice->gate);
    float filter = dnbx_s1_envelope_sample(&voice->filter,
                                          &voice->prepared.filter, voice->gate);
    const s1_dry_controls *p = &voice->prepared.controls;
    float cutoff_hz = s1_dry_cutoff(p->cutoff_hz, p->filter_depth, filter);
    /* For the static depth-zero packet this is already prepared. A future
     * dynamic experiment updates the coefficient when effective cutoff changes;
     * it is not silently frozen to win a cache hit. */
    if (cutoff_hz != voice->prepared.cached_cutoff_hz)
        prepare_filter(&voice->prepared, cutoff_hz);
    float oscillator = 2.0f * voice->phase - 1.0f;
    float component_gain = (1.0f - p->balance) * p->oscillator1_level;
    float excitation = oscillator * component_gain;
    excitation *= voice->velocity_squared;
    excitation *= amp;
    /* Two fixed substeps with held input and boxcar decimation, no audio fitting.
     * The oscillator/envelopes and note events still advance once per output. */
    float first = filter_substep(voice, excitation);
    float second = filter_substep(voice, excitation);
    float output = 0.5f * (first + second);
    voice->phase += voice->phase_increment;
    if (voice->phase >= 1.0f) voice->phase -= 1.0f;
    return output;
}

int s1_dry_render(s1_dry_voice *voice, float *mono, unsigned frames)
{
    if (!voice || !voice->configured || (frames && !mono)) return S1_DRY_ARGUMENT;
    if (frames > S1_DRY_MAX_BLOCK) return S1_DRY_RANGE;
    for (unsigned frame = 0; frame < frames; ++frame)
        mono[frame] = s1_dry_sample(voice);
    return S1_DRY_OK;
}
