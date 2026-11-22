/* SPDX-License-Identifier: GPL-3.0-only */
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
    fx->last_wet[0] = fx->last_wet[1] = fx->dc[0] = fx->dc[1] = 0.f;
    fx->write = 0;
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
    const float center = clampf(48.5f + (rate - 48000.f) * .00034077f, 8.f, 55.f);
    fx->dry_first = (unsigned)center - 5;
    for (unsigned tap = 0; tap < SJ6_CHORUS_ALIGNMENT_TAPS; ++tap) {
        float x = (float)(fx->dry_first + tap) - center;
        float q = x * (TWO_PI / 64.f);
        float window = .346961f + .484695f * fm_cosf(q)
            + .176521f * fm_cosf(2.f * q) + .105375f * fm_cosf(3.f * q);
        fx->alignment[tap] = window * (x > -.0001f && x < .0001f ? 1.f : fm_sinf(PI * x) / (PI * x));
    }
    for (unsigned side = 0; side < 2; ++side) {
        const chorus_delay_lane *lane = &factory_chorus_profile[profile].lane[side];
        fx->rise[side] = lane->span / lane->rise_fraction;
        fx->fall[side] = lane->span / (1.f - lane->rise_fraction);
    }
    fx->phase = 0;
    fx->phase_step = (uint32_t)(factory_chorus_profile[profile].frequency * (4294967296.f / rate));
    fx->warm = factory_chorus_profile[profile].startup;
    fx->warm_step = fm_expf(-1.f / (rate * .3415f));
    fx->dc_step = fm_expf(-175.f / rate);
    fx->mix = clampf(mix, 0.f, 1.f);
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

static float render_chorus_lane(juno6_factory_chorus_t *fx, const chorus_delay_lane *lane,
                                unsigned side, float sample, float phase, float scale, float trim)
{
    const unsigned dry_mask = SJ6_CHORUS_DRY_FRAMES - 1;
    const unsigned write = fx->write;
    uint16_t *dry = fx->history->dry[side];
    uint16_t *wet = fx->history->wet[side];
    dry[write & dry_mask] = chorus_store(sample);
    const float a = sample + chorus_load(dry[(write - 8) & dry_mask]);
    const float b = chorus_load(dry[(write - 1) & dry_mask]) + chorus_load(dry[(write - 7) & dry_mask]);
    const float c = chorus_load(dry[(write - 2) & dry_mask]) + chorus_load(dry[(write - 6) & dry_mask]);
    const float d = chorus_load(dry[(write - 3) & dry_mask]) + chorus_load(dry[(write - 5) & dry_mask]);
    wet[write] = chorus_store((a + 8.f * b + 28.f * c + 56.f * d
        + 70.f * chorus_load(dry[(write - 4) & dry_mask])) / 256.f);

    float aligned = 0.f;
    for (unsigned tap = 0; tap < SJ6_CHORUS_ALIGNMENT_TAPS; ++tap)
        aligned += fx->alignment[tap] * chorus_load(dry[(write - fx->dry_first - tap) & dry_mask]);

    float cycle = phase + lane->phase;
    if (cycle < 0.f) cycle += 1.f;
    cycle -= (unsigned)cycle;
    const float shape = fm_minf(cycle * fx->rise[side], (1.f - cycle) * fx->fall[side]);
    const float delay = clampf(scale * (lane->minimum + shape + fx->warm) + trim,
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

    const float dc_blocked = delayed - fx->last_wet[side] + fx->dc_step * fx->dc[side];
    fx->last_wet[side] = delayed;
    fx->dc[side] = dc_blocked;
    return aligned * (1.f + .011f * (1.f - fx->mix)) + fx->mix * dc_blocked;
}

static void render_factory_chorus(juno6_chorus_t *c, float *left, float *right, unsigned frames)
{
    juno6_factory_chorus_t *fx = &c->factory;
    if (fx->mix == 0.f) return;
    const chorus_delay_lane *lane = factory_chorus_profile[fx->mode - 1].lane;
    const float scale = c->sample_rate * (1.f / 48000.f);
    /* Offset accounts for filter alignment and the four-frame blur delay. */
    const float trim = (48000.f - c->sample_rate) * .00067f - 4.f;
    for (unsigned i = 0; i < frames; ++i) {
        const float phase = (float)fx->phase * (1.f / 4294967296.f);
        left[i] = render_chorus_lane(fx, &lane[0], 0, left[i], phase, scale, trim);
        right[i] = render_chorus_lane(fx, &lane[1], 1, right[i], phase, scale, trim);
        fx->write = wet_wrap(fx->write + 1u);
        fx->phase += fx->phase_step;
        fx->warm *= fx->warm_step;
    }
}
