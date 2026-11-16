/* SPDX-License-Identifier: GPL-3.0-only */
#include "sj6_resynth.h"

static const int16_t sj6_sine[SJ6_SINE_N] = {
    0,804,1608,2410,3212,4011,4808,5602,6393,7179,7962,8739,9512,10278,11039,11793,
    12539,13279,14010,14732,15446,16151,16846,17530,18204,18868,19519,20159,20787,21403,22005,22594,
    23170,23731,24279,24811,25329,25832,26319,26790,27245,27683,28105,28510,28898,29268,29621,29956,
    30273,30571,30852,31113,31356,31580,31785,31971,32137,32285,32412,32521,32609,32678,32728,32757,
    32767,32757,32728,32678,32609,32521,32412,32285,32137,31971,31785,31580,31356,31113,30852,30571,
    30273,29956,29621,29268,28898,28510,28105,27683,27245,26790,26319,25832,25329,24811,24279,23731,
    23170,22594,22005,21403,20787,20159,19519,18868,18204,17530,16846,16151,15446,14732,14010,13279,
    12539,11793,11039,10278,9512,8739,7962,7179,6393,5602,4808,4011,3212,2410,1608,804,
    0,-804,-1608,-2410,-3212,-4011,-4808,-5602,-6393,-7179,-7962,-8739,-9512,-10278,-11039,-11793,
    -12539,-13279,-14010,-14732,-15446,-16151,-16846,-17530,-18204,-18868,-19519,-20159,-20787,-21403,-22005,-22594,
    -23170,-23731,-24279,-24811,-25329,-25832,-26319,-26790,-27245,-27683,-28105,-28510,-28898,-29268,-29621,-29956,
    -30273,-30571,-30852,-31113,-31356,-31580,-31785,-31971,-32137,-32285,-32412,-32521,-32609,-32678,-32728,-32757,
    -32767,-32757,-32728,-32678,-32609,-32521,-32412,-32285,-32137,-31971,-31785,-31580,-31356,-31113,-30852,-30571,
    -30273,-29956,-29621,-29268,-28898,-28510,-28105,-27683,-27245,-26790,-26319,-25832,-25329,-24811,-24279,-23731,
    -23170,-22594,-22005,-21403,-20787,-20159,-19519,-18868,-18204,-17530,-16846,-16151,-15446,-14732,-14010,-13279,
    -12539,-11793,-11039,-10278,-9512,-8739,-7962,-7179,-6393,-5602,-4808,-4011,-3212,-2410,-1608,-804
};

int32_t sj6_sine_at(uint32_t phase)
{
    uint32_t i = phase >> 24;
    uint32_t f = (phase >> 16) & 255u;
    int32_t a = sj6_sine[i & 255u];
    int32_t b = sj6_sine[(i + 1u) & 255u];
    return a + (((b - a) * (int32_t)f) >> 8);
}

static int32_t env_at(int n, int attack, int decay, int32_t peak, int32_t sus)
{
    if (attack < 1) attack = 1;
    if (decay < 1) decay = 1;
    if (n < attack) return (int32_t)(((int64_t)peak * n) / attack);
    if (n < attack + decay) {
        int t = n - attack;
        return peak + (int32_t)(((int64_t)(sus - peak) * t) / decay);
    }
    return sus;
}

void sj6_state_init(Sj6State *s, const Sj6Voice *v)
{
    unsigned i;
    for (i = 0; i < SJ6_PARTIALS; i++) s->phase[i] = 0;
    s->drift = 0;
    s->noise = (v && v->noise_seed) ? v->noise_seed : 1u;
    s->nstate = 0;
    s->frame = 0;
}

void sj6_render(const Sj6Voice *v, Sj6State *s, int32_t *out, int n, Sj6Cost *c)
{
    int i;
    for (i = 0; i < n; i++) {
        int32_t acc = 0;
        int p;
        int32_t drift = 32767;
        uint32_t nz;
        int32_t nenv, sample;
        if (v->drift_len) {
            s->drift += (0xffffffffu / v->drift_len);
            drift = 32767 + (int32_t)(((int64_t)v->drift_depth * sj6_sine_at(s->drift)) >> 15);
            if (c) { c->adds += 2; c->muls++; c->shifts++; }
        }
        for (p = 0; p < SJ6_PARTIALS; p++) {
            int32_t e, sines;
            const Sj6Partial *part = &v->partial[p];
            uint32_t at = s->frame + (uint32_t)i;
            int env_n = at > 0x7fffffffu ? 0x7fffffff : (int)at;
            if (!part->inc) continue;
            sines = sj6_sine_at(s->phase[p] + part->phase0);
            s->phase[p] += part->inc;
            e = env_at(env_n, part->attack, part->decay, part->peak, part->sustain);
            e = (int32_t)(((int64_t)e * drift) >> 15);
            acc += (int32_t)(((int64_t)sines * e) >> 15);
            if (c) { c->adds += 5; c->muls += 3; c->shifts += 4; c->branches++; }
        }
        nz = s->noise;
        nz ^= nz << 13; nz ^= nz >> 17; nz ^= nz << 5;
        s->noise = nz ? nz : 1u;
        {
            uint32_t at = s->frame + (uint32_t)i;
            int env_n = at > 0x7fffffffu ? 0x7fffffff : (int)at;
            nenv = env_at(env_n, v->partial[0].attack ? v->partial[0].attack : 1,
                          v->partial[0].decay ? v->partial[0].decay : 1,
                          v->noise_peak, v->noise_sustain);
        }
        {
            int32_t white = (int32_t)(nz >> 16) - 32768;
            if (v->noise_pole) {
                s->nstate += (white - s->nstate) >> v->noise_pole;
                white = s->nstate;
                if (c) { c->adds += 2; c->shifts++; }
            }
            acc += (int32_t)(((int64_t)white * nenv) >> 15);
        }
        if (c) { c->adds += 3; c->muls++; c->shifts += 3; }
        sample = acc;
        if (sample > 32767) sample = 32767;
        if (sample < -32768) sample = -32768;
        out[i] = sample;
        if (c) c->branches += 2;
    }
    s->frame += (uint32_t)n;
}

#ifdef SJ6_STANDIN_EMBED
#include "sj6_standin.inc"
unsigned sj6_standin_count(void)
{
    return (unsigned)(sizeof sj6_standins / sizeof sj6_standins[0]);
}
const Sj6Voice *sj6_standin(unsigned index)
{
    if (index >= sj6_standin_count()) return 0;
    return &sj6_standins[index];
}
static Sj6State sj6_keep_state;
static int32_t sj6_keep_y[8];
void sj6_resynth_keep(void)
{
    const Sj6Voice *v = sj6_standin(0);
    if (!v) return;
    sj6_state_init(&sj6_keep_state, v);
    sj6_render(v, &sj6_keep_state, sj6_keep_y, 8, 0);
}
#else
unsigned sj6_standin_count(void) { return 0; }
const Sj6Voice *sj6_standin(unsigned index) { (void)index; return 0; }
void sj6_resynth_keep(void) {}
#endif
