/* SPDX-License-Identifier: GPL-3.0-only */
#include "juno6.h"
#include "fastmath.h"
#include <string.h>
#define PI 3.14159265358979323846f
#define TWO_PI 6.28318530717958647692f
#define clampf fm_clampf
/* Independent stereo delay model fitted to original build-9 measurements.
 * Delay bounds and phase are inferred from repeated impulse arrival times. */
typedef struct {
    float phase, minimum, span, rise_fraction;
} chorus_delay_lane;

static const struct {
    float frequency, startup;
    chorus_delay_lane lane[2];
} factory_chorus_profile[] = {
    { .479316f, -71.03f, { { .524582f, 125.426f, 185.789f, .524583f },
                          { -.000051f, 125.351f, 192.028f, .475384f } } },
    { .819755f, -71.11f, { { .514213f, 125.443f, 185.763f, .514385f },
                          { -.000151f, 125.364f, 191.988f, .485616f } } },
    { 9.20039f, -157.58f, { { -.003153f, 211.801f, 13.929f, .498722f },
                           { -.003397f, 211.799f, 14.402f, .498676f } } },
};

void juno6_chorus_clear_factory(juno6_chorus_t *c)
{
    juno6_factory_chorus_t *fx = &c->factory;
    if (!fx->mode) return;
    memset(fx->history, 0, sizeof *fx->history);
    memset(c->factory_dry, 0, sizeof c->factory_dry);
    fx->last_wet[0] = fx->last_wet[1] = fx->dc[0] = fx->dc[1] = 0.f;
    fx->write = 0;
    fx->shared = 1;
}

void juno6_chorus_select_factory(juno6_chorus_t *c, juno6_chorus_memory *history, unsigned mode, float mix)
{
    juno6_factory_chorus_t *fx = &c->factory;
    if (mode > sizeof factory_chorus_profile / sizeof *factory_chorus_profile || (mode && !history))
        return;
    if (fx->mode || mode) {
        memset(c->delay, 0, sizeof c->delay);
        c->write = 0;
    }
    fx->history = mode ? history : NULL;
    fx->mode = (uint8_t)mode;
    if (!mode) return;
    juno6_chorus_clear_factory(c);
    unsigned profile = mode - 1;
    const float rate = c->sample_rate;
    const float scale = rate * (1.f / 48000.f);
    /* Include the four-frame blur group delay in the wet read position. */
    const float trim = (48000.f - rate) * .00067f - 4.f;
    const float center = clampf(48.5f + (rate - 48000.f) * .00034077f, 8.f, 55.f);
    /* The shorter interpolator covers the near-integer alignment at the
     * firmware sample rate. Keep the longer filter for fractional centers. */
    fx->short_alignment = center - (unsigned)center < .2f;
    const unsigned taps = fx->short_alignment ? 4u : SJ6_CHORUS_ALIGNMENT_TAPS;
    fx->dry_first = (unsigned)center + 1u - taps / 2u;
    for (unsigned tap = 0; tap < taps; ++tap) {
        float x = (float)(fx->dry_first + tap) - center;
        float q = x * (TWO_PI / 64.f);
        float window = .346961f + .484695f * fm_cosf(q)
            + .176521f * fm_cosf(2.f * q) + .105375f * fm_cosf(3.f * q);
        fx->alignment[tap] = window * (x > -.0001f && x < .0001f ? 1.f : fm_sinf(PI * x) / (PI * x));
    }
    for (unsigned side = 0; side < 2; ++side) {
        const chorus_delay_lane *lane = &factory_chorus_profile[profile].lane[side];
        fx->rise[side] = (scale * lane->span) / lane->rise_fraction;
        fx->fall[side] = (scale * lane->span) / (1.f - lane->rise_fraction);
        fx->minimum[side] = scale * lane->minimum + trim;
        const float offset = lane->phase < 0.f ? 1.f + lane->phase : lane->phase;
        fx->phase_offset[side] = (uint32_t)(offset * 4294967296.f);
    }
    fx->phase = 0;
    fx->phase_step = (uint32_t)(factory_chorus_profile[profile].frequency * (4294967296.f / rate));
    fx->warm = scale * factory_chorus_profile[profile].startup;
    fx->warm_step = fm_expf(-1.f / (rate * .3415f));
    fx->dc_step = fm_expf(-175.f / rate);
    fx->mix = clampf(mix, 0.f, 1.f);
    fx->dry_gain = 1.f + .011f * (1.f - fx->mix);
}

/* Six exponent bits and nine mantissa bits preserve quiet delay tails
 * without clipping the normal synth range to an integer amplitude rail. */
static uint16_t chorus_store(float value)
{
    fm_bits_t bits = {.f = value};
    const uint32_t magnitude = bits.u & 0x7fffffffu;
    const unsigned sign = (bits.u >> 16) & 0x8000u;
    if (magnitude < (97u << 23)) return 0;
    const unsigned word = (magnitude - (96u << 23) + (1u << 13)) >> 14;
    return (uint16_t)(sign | (word > 0x7fffu ? 0x7fffu : word));
}

static float chorus_load(uint16_t word)
{
    if (!word) return 0.f;
    const fm_bits_t bits = {.u = ((uint32_t)(word & 0x8000u) << 16)
        | ((96u << 23) + ((uint32_t)(word & 0x7fffu) << 14))};
    return bits.f;
}

static unsigned wet_wrap(unsigned index)
{
    return index >= SJ6_CHORUS_WET_FRAMES ? index - SJ6_CHORUS_WET_FRAMES : index;
}

static float chorus_input(juno6_factory_chorus_t *fx, float *dry, uint16_t *wet,
                          unsigned write, float sample)
{
    const unsigned dry_mask = SJ6_CHORUS_DRY_FRAMES - 1;
    dry[write & dry_mask] = sample;
    const float a = sample + dry[(write - 8) & dry_mask];
    const float b = dry[(write - 1) & dry_mask] + dry[(write - 7) & dry_mask];
    const float c = dry[(write - 2) & dry_mask] + dry[(write - 6) & dry_mask];
    const float d = dry[(write - 3) & dry_mask] + dry[(write - 5) & dry_mask];
    wet[write] = chorus_store((a + 8.f * b + 28.f * c + 56.f * d
        + 70.f * dry[(write - 4) & dry_mask]) / 256.f);

    float aligned = 0.f;
    if (fx->short_alignment) {
        const unsigned start = write - fx->dry_first;
        aligned += fx->alignment[0] * dry[start & dry_mask];
        aligned += fx->alignment[1] * dry[(start - 1u) & dry_mask];
        aligned += fx->alignment[2] * dry[(start - 2u) & dry_mask];
        aligned += fx->alignment[3] * dry[(start - 3u) & dry_mask];
    } else {
        for (unsigned tap = 0; tap < SJ6_CHORUS_ALIGNMENT_TAPS; ++tap)
            aligned += fx->alignment[tap] * dry[(write - fx->dry_first - tap) & dry_mask];
    }

    return aligned;
}

static float chorus_read_tap(juno6_factory_chorus_t *fx, const uint16_t *wet,
                             unsigned side, unsigned write, uint32_t phase, float warm)
{
    const float cycle = (float)phase * (1.f / 4294967296.f);
    const float shape = fm_minf(cycle * fx->rise[side], (1.f - cycle) * fx->fall[side]);
    const float delay = clampf(fx->minimum[side] + shape + warm,
                               2.f, (float)(SJ6_CHORUS_WET_FRAMES - 3));
    const unsigned whole = (unsigned)delay;
    const unsigned index = write + SJ6_CHORUS_WET_FRAMES - whole;
    const float y0 = chorus_load(wet[wet_wrap(index)]);
    const float y1 = chorus_load(wet[wet_wrap(index - 1)]);
    const float slope0 = .5f * (y1 - chorus_load(wet[wet_wrap(index + 1)]));
    const float slope1 = .5f * (chorus_load(wet[wet_wrap(index - 2)]) - y0);
    const float frac = delay - whole;
    const float change = y1 - y0;
    const float delayed = y0 + frac * (slope0 + frac * (3.f * change - 2.f * slope0 - slope1
        + frac * (-2.f * change + slope0 + slope1)));

    return delayed;
}

static void render_shared_chorus(juno6_chorus_t *c, float *left, float *right, unsigned frames)
{
    juno6_factory_chorus_t *fx = &c->factory;
    unsigned write = fx->write;
    uint32_t phase = fx->phase;
    float warm = fx->warm;
    uint16_t *wet = fx->history->wet[0];
    for (unsigned i = 0; i < frames; ++i) {
        const float dry = fx->dry_gain * chorus_input(fx, c->factory_dry[0], wet, write, left[i]);
        float *out[2] = {left + i, right + i};
        for (unsigned side = 0; side < 2; ++side) {
            const float delayed = chorus_read_tap(fx, wet, side, write, phase + fx->phase_offset[side], warm);
            const float dc = delayed - fx->last_wet[side] + fx->dc_step * fx->dc[side];
            fx->last_wet[side] = delayed;
            fx->dc[side] = dc;
            *out[side] = dry + fx->mix * dc;
        }
        write = wet_wrap(write + 1u);
        phase += fx->phase_step;
        warm *= fx->warm_step;
    }
    fx->write = write;
    fx->phase = phase;
    fx->warm = warm;
}

__attribute__((noinline)) void juno6_chorus_render_factory(juno6_chorus_t *c, float *left, float *right, unsigned frames)
{
    juno6_factory_chorus_t *fx = &c->factory;
    if (fx->mix == 0.f) return;
    if (fx->shared) {
        if (!memcmp(left, right, frames * sizeof *left)) {
            render_shared_chorus(c, left, right, frames);
            return;
        }
        /* Materialize the peer history before either distinct input advances. */
        memcpy(c->factory_dry[1], c->factory_dry[0], sizeof c->factory_dry[0]);
        memcpy(fx->history->wet[1], fx->history->wet[0], sizeof fx->history->wet[0]);
        fx->shared = 0;
    }
    float *outputs[2] = {left, right};
    unsigned write = 0;
    uint32_t phase = 0;
    float warm = 0.f;
    for (unsigned side = 0; side < 2; ++side) {
        float *out = outputs[side];
        float *dry = c->factory_dry[side];
        uint16_t *wet = fx->history->wet[side];
        float last_wet = fx->last_wet[side];
        float dc = fx->dc[side];
        write = fx->write;
        phase = fx->phase + fx->phase_offset[side];
        warm = fx->warm;
        for (unsigned i = 0; i < frames; ++i) {
            const float sample = out[i];
            const float aligned = chorus_input(fx, dry, wet, write, sample);
            const float delayed = chorus_read_tap(fx, wet, side, write, phase, warm);

            dc = delayed - last_wet + fx->dc_step * dc;
            last_wet = delayed;
            out[i] = fx->dry_gain * aligned + fx->mix * dc;
            write = wet_wrap(write + 1u);
            phase += fx->phase_step;
            warm *= fx->warm_step;
        }
        fx->last_wet[side] = last_wet;
        fx->dc[side] = dc;
    }
    fx->write = write;
    fx->phase = phase - fx->phase_offset[1];
    fx->warm = warm;
}
