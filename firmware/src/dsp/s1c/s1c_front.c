/* SPDX-License-Identifier: GPL-3.0-only */
/* Helper laws follow AudioKit Synth One's S1Rate / setSynthParameter (MIT, AudioKit
 * contributors); this is an independent C rewrite for the compact engine. */
#include "s1c_front.h"
#include "s1c_tables.inc"
static float dict_f(uint8_t i) { union { uint32_t u; float f; } v; v.u = s1c_dict[i]; return v.f; }
#define MINV(id) dict_f(s1c_imin[id])
#define MAXV(id) dict_f(s1c_imax[id])
float s1c_front_init(unsigned id) { return id < 150 ? dict_f(s1c_iinit[id]) : 0.f; }

enum { F_L1R = 15, F_PANF = 36, F_DLT = 43, F_L2R = 47, F_ARPRATE = 64, F_TSYNC = 139, F_PB = 140,
       F_A4 = 143, F_TMULT = 147, F_RATES = 19, F_TWOBARS = 4 };
/* bars per rate (8 bars .. 1/64 triplet) and the triplet flag */
static const float bars[F_RATES] = {8.f, 6.f, 4.f, 3.f, 2.f, 1.f, 1.f, 1.f/2.f, 1.f/2.f, 1.f/4.f, 1.f/4.f,
    1.f/8.f, 1.f/8.f, 1.f/16.f, 1.f/16.f, 1.f/32.f, 1.f/32.f, 1.f/64.f, 1.f/64.f};
#define TRIP(i) ((i) == 6 || ((i) >= 8 && ((i) & 1) == 0))
static float rate_time(float bpm, int i)
{
    return (4.f * bars[i]) / (bpm * (1.f / 60.f)) / (TRIP(i) ? 1.5f : 1.f);
}
static float rate_factor(int i)
{
    static const float f[F_RATES] = {8.f, 6.f, 4.f, 3.f, 2.f, 1.f, 1.f / 1.5f, 1.f / 2.f, 1.f / 2.f / 1.5f,
        1.f / 4.f, 1.f / 4.f / 1.5f, 1.f / 8.f, 1.f / 8.f / 1.5f, 1.f / 16.f, 1.f / 16.f / 1.5f,
        1.f / 32.f, 1.f / 32.f / 1.5f, 1.f / 64.f, 1.f / 64.f / 1.5f};
    return f[i];
}
static float fabs_(float x) { return x < 0.f ? -x : x; }
float s1c_front_clamp(unsigned id, float x)
{
    float lo = MINV(id), hi = MAXV(id);
    return x < lo ? lo : x > hi ? hi : x;
}
static void put(s1c *s, unsigned id, float x) { s1c_set(s, id, s1c_front_clamp(id, x)); }
static float nearest_frequency(float x, float bpm, float lo, float hi)
{
    float best = 1.f / rate_time(bpm, 0), diff = 1e9f;
    for (int i = 0; i < F_RATES; i++) {
        float f = 1.f / rate_time(bpm, i);
        if (f < lo || f > hi) continue;
        float d = fabs_(f - x);
        if (d < diff) { diff = d; best = f; }
    }
    return best;
}
static float nearest_time(float x, float bpm, float lo, float hi)
{
    float best = rate_time(bpm, F_RATES - 1), diff = 1e9f;
    for (int i = F_RATES - 1; i >= F_TWOBARS; i--) {
        float t = rate_time(bpm, i);
        if (t < lo || t > hi) continue;
        float d = fabs_(t - x);
        if (d < diff) { diff = d; best = t; }
    }
    return best;
}
static float nearest_factor(float x)
{
    float best = rate_factor(F_RATES - 1), diff = 1e9f;
    for (int i = F_RATES - 1; i >= 0; i--) {
        float d = fabs_(rate_factor(i) - x);
        if (d < diff) { diff = d; best = rate_factor(i); }
    }
    return best;
}
static void rate(s1c *s, unsigned id, float x)
{
    if (id == F_PB) { put(s, id, x); return; }
    if (id == F_TMULT) { put(s, id, nearest_factor(s1c_front_clamp(id, x))); return; }
    if (s->t[F_TSYNC] > 0.f) {
        float v = s1c_front_clamp(id, x);
        put(s, id, id == F_DLT ? nearest_time(v, s->t[F_ARPRATE], MINV(id), MAXV(id))
                               : nearest_frequency(v, s->t[F_ARPRATE], MINV(id), MAXV(id)));
    } else
        put(s, id, x);
}
int s1c_front_set(s1c *s, unsigned id, float x)
{
    if (id >= 150 || !(x == x) || x > 3.4e38f || x < -3.4e38f) return 0;
    if (id == F_TSYNC || id == F_ARPRATE) {
        put(s, id, x);
        rate(s, F_L1R, s->t[F_L1R]); rate(s, F_L2R, s->t[F_L2R]);
        rate(s, F_PANF, s->t[F_PANF]); rate(s, F_DLT, s->t[F_DLT]);
    } else if (id == F_L1R || id == F_L2R || id == F_PANF || id == F_DLT || id == F_PB || id == F_TMULT)
        rate(s, id, x);
    else if (id == F_A4)
        put(s, id, (float)(int)x);
    else
        put(s, id, x);
    return 1;
}
