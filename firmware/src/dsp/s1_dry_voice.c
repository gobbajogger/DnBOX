/* SPDX-License-Identifier: GPL-3.0-only */
#include "s1_dry_voice.h"
#include "fastmath.h"
#include <string.h>

/* The oscillator and cascaded low-pass stages derive from DnBOX primitives.
 * The response calibration uses only original input/output observations. */
_Static_assert(sizeof(float) == 4, "binary32 storage required");
_Static_assert(sizeof(s1_dry_voice) == 124, "bounded single-voice state changed");

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
    /* Fitted to independent impulse observations at twelve cutoff settings.
     * Both coefficients are prepared once per cutoff update. */
    float x = cutoff_hz * (1.0f / 22050.0f);
    float step = x * (1.56874466f + x * (-1.73140822f +
                 x * (1.55879259f + x * (-0.711601815f + x * 0.118475673f))));
    p->pole_retention = 1.0f - step;
    p->feedback = p->resonance *
                  (3.98720139f + x * (3.68178143f - x * 3.93633599f));
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
    next.cutoff_hz = controls->cutoff_hz;
    next.resonance = controls->resonance;
    next.filter_depth = controls->filter_depth;
    next.oscillator1_level = controls->oscillator1_level;
    next.balance = controls->balance;
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
    if (voice->gate) {
        /* The instrument advances both envelopes on the release edge. This
         * preserves a retrigger even when both events precede the next sample. */
        voice->gate = 0;
        dnbx_s1_envelope_sample(&voice->amp, &voice->prepared.amp, 0);
        dnbx_s1_envelope_sample(&voice->filter, &voice->prepared.filter, 0);
    }
    return S1_DRY_OK;
}

float s1_dry_cutoff(float base_hz, float depth, float envelope)
{
    return fm_clampf(base_hz * (1.0f - depth * (1.0f - envelope)),
                    64.0f, 22050.0f);
}

static inline __attribute__((always_inline)) float filter_substep(s1_dry_voice *voice, float input)
{
    float *history = voice->filter_history;
    float A = voice->prepared.pole_retention;
    float tmp = history[3];
    float v = input - voice->prepared.feedback * (0.5f * (history[3] + history[4]));
    float z1 = v + A * (history[0] - v);
    float z2 = z1 + A * (history[1] - z1);
    float z3 = z2 + A * (history[2] - z2);
    float z4 = z3 + A * (history[3] - z3);
    history[0] = z1; history[1] = z2;
    history[2] = z3; history[3] = z4; history[4] = tmp;
    return z4;
}

void s1_dry_begin_block(s1_dry_voice *voice)
{
    if (!voice->gate && voice->amp.level < 0.01f)
        voice->started = 0;
}

static void update_cutoff(s1_dry_voice *voice, float filter)
{
    const s1_dry_prepared *p = &voice->prepared;
    float cutoff_hz = s1_dry_cutoff(p->cutoff_hz, p->filter_depth, filter);
    /* Update only when the filter envelope moves the effective cutoff. */
    if (cutoff_hz != voice->prepared.cached_cutoff_hz)
        prepare_filter(&voice->prepared, cutoff_hz);
}

static __attribute__((noinline)) float oscillator_filter(s1_dry_voice *voice, float amp, float component_gain)
{
    float oscillator = 2.0f * voice->phase - 1.0f;
    /* The unbandlimited source is a periodic 4096-point raw ramp. Its last
     * phase interval interpolates the final point across the seam. */
    if (voice->phase >= 4095.f / 4096.f)
        oscillator -= 2.f * (4096.f * voice->phase - 4095.f);
    float excitation = oscillator * component_gain;
    excitation *= voice->velocity_squared;
    excitation *= amp;
    /* The oscillator and envelopes advance once per output frame. */
    float first = filter_substep(voice, excitation);
    float second = filter_substep(voice, excitation);
    float output = 0.5f * (first + second);
    voice->phase += voice->phase_increment;
    if (voice->phase >= 1.0f) voice->phase -= 1.0f;
    return output;
}

static float component_gain(const s1_dry_voice *voice)
{
    const s1_dry_prepared *p = &voice->prepared;
    return (1.0f - p->balance) * p->oscillator1_level;
}

float s1_dry_sample(s1_dry_voice *voice)
{
    if (!voice->started) return 0.0f;
    float amp = dnbx_s1_envelope_sample(&voice->amp, &voice->prepared.amp, voice->gate);
    float filter = dnbx_s1_envelope_sample(&voice->filter, &voice->prepared.filter, voice->gate);
    update_cutoff(voice, filter);
    return oscillator_filter(voice, amp, component_gain(voice));
}

/* Note, gate and parameter changes arrive between spans. Within a span the
 * additive term stays fixed until the envelope crosses its attack threshold. */
static float envelope_term(const dnbx_s1_envelope_state *state,
                           const dnbx_s1_envelope_params *params)
{
    float target = state->stage == DNBX_S1_ENVELOPE_ATTACK ? 1.0f :
                   state->stage == DNBX_S1_ENVELOPE_DECAY ? params->sustain : 0.0f;
    return (1.0f - state->retention) * target;
}

static inline float advance_envelope(dnbx_s1_envelope_state *state,
                                     const dnbx_s1_envelope_params *params,
                                     float *term)
{
    if (state->stage == DNBX_S1_ENVELOPE_ATTACK && state->level >= 0.99f) {
        state->stage = DNBX_S1_ENVELOPE_DECAY;
        state->retention = params->decay_retention;
        *term = envelope_term(state, params);
    }
    state->level = state->retention * state->level + *term;
    return state->level;
}

void s1_dry_mix(s1_dry_voice *voice, float *mix, unsigned frames)
{
    if (!frames || !voice->started) return;
    /* Apply pending gate edges and coefficient latches on the first frame. */
    mix[0] += s1_dry_sample(voice);
    float amp_term = envelope_term(&voice->amp, &voice->prepared.amp);
    float filter_term = envelope_term(&voice->filter, &voice->prepared.filter);
    const float gain = component_gain(voice);
    const int modulated = voice->prepared.filter_depth != 0.0f;
    for (unsigned i = 1; i < frames; ++i) {
        float amp = advance_envelope(&voice->amp, &voice->prepared.amp, &amp_term);
        float filter = advance_envelope(&voice->filter, &voice->prepared.filter, &filter_term);
        if (modulated) update_cutoff(voice, filter);
        mix[i] += oscillator_filter(voice, amp, gain);
    }
}

int s1_dry_render(s1_dry_voice *voice, float *mono, unsigned frames)
{
    if (!voice || !voice->configured || (frames && !mono)) return S1_DRY_ARGUMENT;
    if (frames > S1_DRY_MAX_BLOCK) return S1_DRY_RANGE;
    if (frames) s1_dry_begin_block(voice);
    for (unsigned frame = 0; frame < frames; ++frame)
        mono[frame] = s1_dry_sample(voice);
    return S1_DRY_OK;
}
