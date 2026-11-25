/* SPDX-License-Identifier: GPL-3.0-only */
/* Helper laws follow AudioKit Synth One's S1Rate / setSynthParameter (MIT, AudioKit
 * contributors); this is an independent C rewrite for the compact engine. */
#include "s1c_front.h"
#include "s1c_tables.inc"
#if X0X_S1_AUTOPAN_PORT && X0X_S1_AUTOPAN_PROBE
#include "s1_pan_port.h"
#endif
#if X0X_SYNTHONE_PATCH
#ifdef S1C_FASTMATH
#include "../fastmath.h"
#define powf fm_powf
#else
#include <math.h>
#endif
#endif
static float dict_f(uint8_t i) { union { uint32_t u; float f; } v; v.u = s1c_dict[i]; return v.f; }
#define MINV(id) dict_f(s1c_imin[id])
#define MAXV(id) dict_f(s1c_imax[id])
float s1c_front_init(unsigned id) { return id < 150 ? dict_f(s1c_iinit[id]) : 0.f; }

enum { F_L1R = 15, F_PANF = 36, F_DLT = 43, F_L2R = 47, F_ARPRATE = 64, F_TSYNC = 139, F_PB = 140,
       F_A4 = 143, F_TMULT = 147, F_RATES = 19, F_TWOBARS = 4 };
/* Store bars in exact binary units of 1/64 (8 bars to 1/64).
 * Every triplet uses a power-of-two base; scaling the rounded 2/3
 * by that base is exactly the old factor table's binary32 value. */
static const uint16_t bars64[F_RATES] = {512,384,256,192,128,64,64,32,32,16,16,8,8,4,4,2,2,1,1};
#define TRIP(i) ((i) == 6 || ((i) >= 8 && ((i) & 1) == 0))
static float rate_time(float bpm, int i)
{
    return ((float)bars64[i] * (1.f / 16.f)) / (bpm * (1.f / 60.f)) / (TRIP(i) ? 1.5f : 1.f);
}
static float rate_factor(int i)
{
    const float base = (float)bars64[i] * (1.f / 64.f);
    return TRIP(i) ? base * (1.f / 1.5f) : base;
}
static float fabs_(float x) { return x < 0.f ? -x : x; }
float s1c_front_clamp(unsigned id, float x)
{
    float lo = MINV(id), hi = MAXV(id);
    return x < lo ? lo : x > hi ? hi : x;
}
static void put(s1c *s, unsigned id, float x) { s1c_set(s, id, s1c_front_clamp(id, x)); }
void s1c_front_bounds(unsigned id, float *minimum, float *maximum)
{
    *minimum = MINV(id);
    *maximum = MAXV(id);
}
static int nearest_division(float x, float bpm, float lo, float hi)
{
    int best = 0;
    float diff = 1e9f;
    for (int i = 0; i < F_RATES; i++) {
        float f = 1.f / rate_time(bpm, i);
        if (f < lo || f > hi) continue;
        float d = fabs_(f - x);
        if (d < diff) { diff = d; best = i; }
    }
    return best;
}
static float nearest_frequency(float x, float bpm, float lo, float hi)
{ return 1.f / rate_time(bpm, nearest_division(x, bpm, lo, hi)); }

#if X0X_SYNTHONE_PATCH
#if defined(X0X_S1_MOD_RATE_PROBE) && X0X_S1_MOD_RATE_PROBE
void s1c_front_mod_rate(s1c *s, unsigned id, unsigned cc1)
{
    const float x = (float)cc1 / 127.f;
    const float lo = MINV(id), hi = MAXV(id);
    const float hz = s->t[F_TSYNC] > 0.f
        ? 1.f / rate_time(s->t[F_ARPRATE], (int)(x * (F_RATES - 1)))
        : lo + powf(x, 2.5f) * (hi - lo);
    (void)s1c_front_set(s, id, hz);
}
#endif
unsigned s1c_front_lfo_steps(const s1c *s)
{
    if (s->t[F_TSYNC] <= 0.f) return 127u;
    /* At 200 BPM the original binary32 last triplet can exceed MAXV. */
    return 1.f / rate_time(s->t[F_ARPRATE], F_RATES - 1) > MAXV(F_L1R) ?
           F_RATES - 2u : F_RATES - 1u;
}

unsigned s1c_front_lfo_position(const s1c *s, unsigned id)
{
    float low, high, hz = s->t[id];
    s1c_front_bounds(id, &low, &high);
    if (s->t[F_TSYNC] > 0.f)
        return (unsigned)nearest_division(hz, s->t[F_ARPRATE], low, high);
    float linear = (s1c_front_clamp(id, hz) - low) / (high - low);
    return (unsigned)(powf(linear, 0.4f) * 127.f + 0.5f);
}

void s1c_front_lfo_apply(s1c *s, unsigned id, unsigned code)
{
    float value;
    if (code >= 129u) value = 1.f / rate_time(s->t[F_ARPRATE], (int)(code - 129u));
    else {
        float normalized = (float)(code - 1u) / 127.f;
        const float low = MINV(id), high = MAXV(id);
        value = low + powf(normalized, 2.5f) * (high - low);
    }
    (void)s1c_front_set(s, id, value);
}
#endif

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
#if defined(X0X_S1_AUTOPAN_PROBE) && X0X_S1_AUTOPAN_PROBE
unsigned s1c_front_pan_stops(const s1c *s)
{
    if (s->t[F_TSYNC]<=0.f) return 127;
    int j=F_RATES-1;
    while(j>0 && 1.f/rate_time(s->t[F_ARPRATE],j)>MAXV(F_PANF)) --j;
    return (unsigned)j;
}
int s1c_front_pan_select(s1c *s,unsigned pos)
{
    if(pos>s1c_front_pan_stops(s)) return 0;
#if !X0X_S1_AUTOPAN_PORT
    const float lo=MINV(F_PANF),hi=MAXV(F_PANF);
#endif
    const float v=s->t[F_TSYNC]>0.f ? 1.f/rate_time(s->t[F_ARPRATE],(int)pos)
#if X0X_S1_AUTOPAN_PORT
                                    : s1_pan_free_select(pos);
#else
                                    : lo+(hi-lo)*powf((float)pos/127.f,2.5f);
#endif
    return s1c_front_set(s,F_PANF,v);
}
unsigned s1c_front_pan_position(const s1c *s)
{
    const float lo=MINV(F_PANF),hi=MAXV(F_PANF);
    const float hz=s1c_front_clamp(F_PANF,s->t[F_PANF]);
    return s->t[F_TSYNC]>0.f ? (unsigned)nearest_division(hz,s->t[F_ARPRATE],lo,hi) :
#if X0X_S1_AUTOPAN_PORT
           s1_pan_free_position(hz);
#else
           (unsigned)(127.f*powf((hz-lo)/(hi-lo),.4f)+.5f);
#endif
}
int s1c_front_pan_sync(const s1c *s){return s->t[F_TSYNC]>0.f;}
#endif
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
        s1c_set(s, id, (float)(int)s1c_front_clamp(id, x));
    else
        put(s, id, x);
    return 1;
}
