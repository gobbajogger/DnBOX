/* SPDX-License-Identifier: GPL-3.0-only */
/* Selectable SuperJ6 voice. Preset numbers come from the R008 tables.
 * Time controls (attack, decay, release) are the resolved values in seconds.
 * A cutoff of 0..1 maps to 20 Hz .. 20 kHz. That map is the live engine's,
 * because the captured source stores the control and does not publish a Hz law.
 * Filter type 0 is the resonant lowpass, 1 is OTA, 2 is syrup. */
#include "sj6_engine.h"
#include <string.h>

#include "sj6_r008.inc"

enum { SJ6_RATE = 44100 };

static float bitsf(uint32_t u)
{
    float y;
    memcpy(&y, &u, 4);
    return y;
}

static int f32_ok(float x)
{
    uint32_t u;
    memcpy(&u, &x, 4);
    return ((u >> 23) & 255u) != 255u;
}

static float clampf(float x, float a, float b)
{
    if (x < a) return a;
    if (x > b) return b;
    return x;
}

static float ctrl(const Sj6Engine *e, unsigned id)
{
    return e->control[id];
}

static float physical(const Sj6Engine *e, unsigned id)
{
    unsigned x = sj6_expr_index[id];
    float v = e->control[id];
    if (!x) return v;
    return bitsf(sj6_expr_lo[x - 1]) + v * (bitsf(sj6_expr_hi[x - 1]) - bitsf(sj6_expr_lo[x - 1]));
}

void sj6_engine_init(Sj6Engine *e)
{
    unsigned i;
    memset(e, 0, sizeof *e);
    for (i = 0; i < SJ6_CTRLS; i++) {
        uint32_t u = sj6_default_bits[i];
        memcpy(&e->control[i], &u, 4);
    }
}

void sj6_engine_select(Sj6Engine *e, int on)
{
    e->selected = (uint8_t)(on != 0);
}

int sj6_engine_load(Sj6Engine *e, const uint16_t *ids, const float *values, unsigned n)
{
    unsigned i, bad = 0;
    for (i = 0; i < n; i++) {
        if (ids[i] >= SJ6_CTRLS || !f32_ok(values[i])) {
            bad++;
            continue;
        }
        e->control[ids[i]] = values[i];
    }
    return (int)bad;
}

static void voice_off(Sj6Rt *v)
{
    v->on = 0;
}

void sj6_engine_note_off(Sj6Engine *e, uint8_t note)
{
    unsigned i;
    for (i = 0; i < SJ6_POLY; i++) {
        if (e->voice[i].on && e->voice[i].note == note)
            sj6_env_release(&e->voice[i].amp, 5);
    }
}

static void arm_filter(Sj6Rt *v)
{
    v->last_cut = -1.f;
    if (v->kind == 1) sj6_ota_init(&v->flt.ota, (float)SJ6_RATE);
    else if (v->kind == 2) sj6_syrup_init(&v->flt.syrup, (float)SJ6_RATE);
    else sj6_lowpass_init(&v->flt.lp, (float)SJ6_RATE);
}

static void capture(Sj6Engine *e, Sj6Rt *v, uint8_t note, uint8_t vel)
{
    float cn, oct, semis, hz, inc, a, d, s, r;
    int shape = (int)(physical(e, 82) + 0.5f);
    int kind = (int)(ctrl(e, 209) + 0.5f);
    if (shape < 0) shape = 0;
    if (shape > 3) shape = 3;
    if (kind < 0 || kind > 2) kind = 0;
    v->kind = (uint8_t)kind;
    v->g_saw = (shape == 0 || shape == 2) ? clampf(ctrl(e, 3), 0.f, 2.f) : 0.f;
    v->g_pulse = (shape == 1 || shape == 2) ? clampf(ctrl(e, 1), 0.f, 2.f) : 0.f;
    v->g_sine = shape == 3 ? clampf(ctrl(e, 1) + ctrl(e, 3), 0.f, 2.f) : 0.f;
    if (v->g_sine > 1.f) v->g_sine = 1.f;
    v->g_sub = clampf(ctrl(e, 202), 0.f, 2.f);
    v->g_noise = clampf(ctrl(e, 201), 0.f, 2.f);
    v->master = clampf(ctrl(e, 0), 0.f, 2.f) * 0.2f;
    v->width = ctrl(e, 78) <= 0.f ? 0.5f : clampf(ctrl(e, 78), 0.05f, 0.95f);
    v->lfo_amt = ctrl(e, 87) > 0.f ? clampf(ctrl(e, 89), 0.f, 1.f) : 0.f;
    cn = ctrl(e, 129) > 0.f ? ctrl(e, 129) : ctrl(e, 7);
    cn = clampf(cn, 0.f, 1.f);
    /* 20 * 1000^cn: 0 -> 20 Hz, 1 -> 20 kHz. */
    v->cut_hz = 20.f * sj6_exp2(cn * 9.965784072875977f);
    v->res = ctrl(e, 130);
    if (ctrl(e, 8) > 0.f) v->res = 0.1f + ctrl(e, 8) * 9.9f;
    v->res = clampf(v->res, 0.1f, 10.f);
    oct = physical(e, 86) - 1.f;
    semis = (float)((int)note - 69) + oct * 12.f;
    hz = 440.f * sj6_exp2(semis * (1.f / 12.f));
    inc = hz * (1.f / (float)SJ6_RATE);
    if (inc < 0.000001f) inc = 0.000001f;
    if (inc > 0.45f) inc = 0.45f;
    sj6_osc_reset(&v->saw);
    sj6_osc_reset(&v->pulse);
    sj6_osc_reset(&v->sub);
    sj6_osc_reset(&v->sine);
    v->saw.increment = v->pulse.increment = v->sine.increment = inc;
    v->sub.increment = inc * 0.5f;
    v->noise.seed = 1u + note;
    sj6_noise_reset(&v->noise);
    a = clampf(ctrl(e, 36), 0.f, 12.f);
    d = clampf(ctrl(e, 37), 0.f, 12.f);
    s = clampf(ctrl(e, 38), 0.f, 1.f);
    r = clampf(ctrl(e, 39), 0.f, 12.f);
    sj6_adsr_init(&v->amp, 1, (float)SJ6_RATE, a, d, s, r);
    sj6_env_start(&v->amp);
    a = clampf(ctrl(e, 132), 0.f, 12.f);
    d = clampf(ctrl(e, 133), 0.f, 12.f);
    s = clampf(ctrl(e, 134), 0.f, 1.f);
    r = clampf(ctrl(e, 135), 0.f, 12.f);
    sj6_adsr_init(&v->filt, 1, (float)SJ6_RATE, a, d, s, r);
    sj6_env_start(&v->filt);
    arm_filter(v);
    v->note = note;
    v->vel = vel > 0 ? (float)vel / 127.f : 0.f;
    v->on = 1;
    v->age = ++e->tick;
}

void sj6_engine_note_on(Sj6Engine *e, uint8_t note, uint8_t vel)
{
    unsigned i;
    int slot = -1, oldest = 0;
    if (!vel) {
        sj6_engine_note_off(e, note);
        return;
    }
    for (i = 0; i < SJ6_POLY; i++) {
        if (e->voice[i].on && e->voice[i].note == note) {
            slot = (int)i;
            break;
        }
    }
    if (slot < 0) {
        for (i = 0; i < SJ6_POLY; i++) {
            if (!e->voice[i].on) {
                slot = (int)i;
                break;
            }
        }
    }
    if (slot < 0) {
        for (i = 1; i < SJ6_POLY; i++)
            if (e->voice[i].age < e->voice[oldest].age) oldest = (int)i;
        slot = oldest;
    }
    capture(e, &e->voice[slot], note, vel);
}

void sj6_engine_resynth(Sj6Engine *e, int which)
{
    const Sj6Voice *v;
    if (which < 0) {
        e->resynth_on = 0;
        return;
    }
    v = sj6_standin((unsigned)which);
    if (!v) {
        e->resynth_on = 0;
        return;
    }
    e->resynth_which = (uint8_t)which;
    e->resynth_on = 1;
    sj6_state_init(&e->resynth, v);
}

static void apply_cut(Sj6Rt *v, float cut)
{
    if (cut < 12.f) cut = 12.f;
    if (cut == v->last_cut) return;
    v->last_cut = cut;
    if (v->kind == 1) sj6_ota_set(&v->flt.ota, cut, v->res, 1);
    else if (v->kind == 2) sj6_syrup_set(&v->flt.syrup, cut, v->res);
    else sj6_lowpass_set(&v->flt.lp, cut, v->res);
}

static float voice_sample(Sj6Engine *e, Sj6Rt *v, float lfo)
{
    float mix = 0.f, env, fenv, cut, y, strength;
    if (!v->on) return 0.f;
    if (v->g_saw > 0.f) mix += sj6_osc_saw(&v->saw, 1.f) * v->g_saw;
    if (v->g_pulse > 0.f) mix += sj6_osc_pwm(&v->pulse, 1.f, v->width) * v->g_pulse;
    if (v->g_sine > 0.f) mix += sj6_osc_sine(&v->sine, 1.f) * v->g_sine;
    if (v->g_sub > 0.f) mix += sj6_osc_saw(&v->sub, 1.f) * v->g_sub;
    if (v->g_noise > 0.f) mix += sj6_noise_sample(&v->noise) * v->g_noise;
    env = sj6_env_sample(&v->amp);
    fenv = sj6_env_sample(&v->filt);
    if (env == 0.f && v->amp.cur == 0) {
        voice_off(v);
        return 0.f;
    }
    strength = clampf(ctrl(e, 136), 0.f, 1.f);
    cut = v->cut_hz * (1.f + lfo * v->lfo_amt);
    cut = cut + (fenv - 0.5f) * strength * v->cut_hz;
    apply_cut(v, cut);
    if (v->kind == 1) y = sj6_ota_process(&v->flt.ota, mix);
    else if (v->kind == 2) y = sj6_syrup_process(&v->flt.syrup, mix);
    else y = sj6_lowpass_process(&v->flt.lp, mix);
    y = y * env * v->vel * v->master;
    return f32_ok(y) ? y : 0.f;
}

static float lfo_wave(Sj6Engine *e, float bpm)
{
    float rate = 0.f, wave = 0.f;
    int64_t ix = 0;
    unsigned kind;
    float norm = clampf(ctrl(e, 88), 0.f, 1.f);
    if (ctrl(e, 87) <= 0.f) return 0.f;
    if (bpm < 1.f) bpm = 120.f;
    if (sj6_lfo_rate_tempo(norm, bpm, &rate)) return 0.f;
    if (sj6_lfo_phase(e->lfo_phase, rate, 1.f / (float)SJ6_RATE, &e->lfo_phase)) return 0.f;
    if (sj6_lfo_index(e->lfo_phase, &ix) || ix < 0) return 0.f;
    kind = 1u + (unsigned)(clampf(ctrl(e, 94), 0.f, 1.f) * 3.f);
    if (kind < 1u) kind = 1u;
    if (kind > 4u) kind = 4u;
    if (sj6_shape_sample(kind, (unsigned)ix, &wave)) return 0.f;
    return wave;
}

void sj6_engine_render(Sj6Engine *e, float *left, float *right, uint32_t n, float bpm)
{
    uint32_t i;
    unsigned v;
    if (!e || !left || !right || !n) return;
    for (i = 0; i < n; i++) {
        float lfo = lfo_wave(e, bpm);
        float s = 0.f;
        for (v = 0; v < SJ6_POLY; v++)
            s += voice_sample(e, &e->voice[v], lfo);
        if (e->resynth_on) {
            const Sj6Voice *zv = sj6_standin(e->resynth_which);
            int32_t y = 0;
            if (zv) {
                sj6_render(zv, &e->resynth, &y, 1, 0);
                s += (float)y * (1.f / 32768.f) * clampf(ctrl(e, 0), 0.f, 2.f);
            }
        }
        if (!f32_ok(s)) s = 0.f;
        left[i] += s;
        right[i] += s;
    }
}

unsigned sj6_engine_sounding(const Sj6Engine *e)
{
    unsigned i, n = 0;
    for (i = 0; i < SJ6_POLY; i++) n += e->voice[i].on;
    return n;
}

void sj6_engine_bytes(unsigned *per_voice, unsigned *pool, unsigned *poly)
{
    if (per_voice) *per_voice = (unsigned)sizeof(Sj6Rt);
    if (pool) *pool = (unsigned)sizeof(Sj6Engine);
    if (poly) *poly = SJ6_POLY;
}

unsigned sj6_fpu_estimate(unsigned voices)
{
    /* Sustain, pulse only, resonant lowpass coefficients already set,
     * shared LFO. Counted from the source as single-precision results
     * (multiplies, adds, and fmas), one pi32v2 FPU result each:
     *   pulse interior (two saw-pairs, decimate, scales)  28
     *   lowpass process                                    5
     *   mix, velocity, master                              4
     *   shared LFO (phase, index, shape)                   8
     * Horizontal envelopes do no floating multiply.
     * OTA and the attack envelope use the software binary64 path and
     * are not in this count. */
    return 8u + 37u * voices;
}

#if X0X_SUPERJ6_BENCH
volatile Sj6BenchRec sj6_bench;

void sj6_bench_add(uint32_t cycles, uint32_t samples, uint32_t voices)
{
    sj6_bench.magic = 0x534A3642u;
    sj6_bench.samples += samples;
    sj6_bench.cycles += cycles;
    sj6_bench.voices = voices;
    if (sj6_bench.samples)
        sj6_bench.per_sample_q8 = (uint32_t)(((uint64_t)sj6_bench.cycles << 8) / sj6_bench.samples);
}
#endif
