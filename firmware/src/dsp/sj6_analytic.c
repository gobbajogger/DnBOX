/* SPDX-License-Identifier: GPL-3.0-only */
#include "sj6_analytic.h"
#include "sj6_sinf.h"
#include <string.h>

float fma32_fused(float a, float b, float c);

static float sj6_fma(float a, float b, float c) { return fma32_fused(a, b, c); }

static float sj6_floor(float x)
{
    /* Reachable oscillator phases stay in [0, about 2). */
    int i = (int)x;
    float f = (float)i;
    return f > x ? f - 1.f : f;
}

static float sj6_fabs(float x)
{
    uint32_t u;
    __builtin_memcpy(&u, &x, 4);
    u &= 0x7fffffffu;
    __builtin_memcpy(&x, &u, 4);
    return x;
}

static float blep(float t, float dt)
{
    if (t < dt) {
        float x = t / dt;
        return sj6_fma(-x, x, x + x) + (-1.f);
    }
    if (t > 1.f - dt) {
        float x = (t + (-1.f)) / dt;
        float y = sj6_fma(x, x, x);
        return (x + y) + 1.f;
    }
    return 0.f;
}

static float blamp(float t, float dt)
{
    if (t < dt) {
        float x = t / dt + (-1.f);
        return ((x * x) * x) * (-0.1666666716337204f);
    }
    if (t > 1.f - dt) {
        float x = (t + (-1.f)) / dt + 1.f;
        return (x * (x * x)) * 0.1666666716337204f;
    }
    return 0.f;
}

static float saw(float p, float dt)
{
    float t = p - sj6_floor(p);
    return sj6_fma(t, 2.f, -1.f) - blep(t, dt);
}

static float tri(float p, float dt)
{
    float t = p - sj6_floor(p);
    float shift = t + 0.5f;
    float base;
    if (shift >= 1.f) shift += -1.f;
    base = sj6_fma(sj6_fabs(sj6_fma(t, 2.f, -1.f)), 2.f, -1.f);
    return sj6_fma(dt * 8.f, blamp(shift, dt) - blamp(t, dt), base);
}

static float advance(Sj6Osc *o, float dt)
{
    float second = o->phase + dt;
    float mid = second;
    if (mid >= 1.f) mid += -1.f;
    o->phase = mid + dt;
    if (o->phase >= 1.f) o->phase += -1.f;
    return mid;
}

static float decimate(Sj6Osc *o, float first, float second)
{
    float a = sj6_fma(second - o->down[2], 0.079866424202919f, o->down[0]);
    float b = sj6_fma(a - o->down[3], 0.5453536510467529f, o->down[1]);
    float c = sj6_fma(first - o->down[6], 0.28382933139801025f, o->down[4]);
    float d = sj6_fma(c - o->down[7], 0.8344119191169739f, o->down[5]);
    o->down[0] = second;
    o->down[1] = a;
    o->down[2] = a;
    o->down[3] = b;
    o->down[4] = first;
    o->down[5] = c;
    o->down[6] = c;
    o->down[7] = d;
    return (b + d) * 0.5f;
}

void sj6_osc_reset(Sj6Osc *o)
{
    int i;
    o->phase = 0.f;
    o->increment = 0.f;
    for (i = 0; i < 8; i++) o->down[i] = 0.f;
}

void sj6_osc_restart(Sj6Osc *o)
{
    int i;
    o->phase = 0.f;
    for (i = 0; i < 8; i++) o->down[i] = 0.f;
}

float sj6_osc_saw(Sj6Osc *o, float gain)
{
    float dt = o->increment * 0.5f;
    float first = saw(o->phase - sj6_floor(o->phase), dt);
    float mid = advance(o, dt);
    float second = saw(mid, dt);
    return ((gain * 0.5f) * decimate(o, first, second)) * 0.6800000071525574f;
}

float sj6_osc_triangle(Sj6Osc *o, float gain)
{
    float dt = o->increment * 0.5f;
    float first = tri(o->phase, dt);
    float mid = advance(o, dt);
    float second = tri(mid, dt);
    return ((gain * 0.5f) * decimate(o, first, second)) * 0.9900000095367432f;
}

float sj6_osc_sine(Sj6Osc *o, float gain)
{
    float dt = o->increment * 0.5f;
    float first_phase = o->phase;
    float mid = advance(o, dt);
    float second = sj6_sinf(mid * 6.2831854820251465f);
    float first = sj6_sinf(first_phase * 6.2831854820251465f);
    return (gain * 0.5f) * decimate(o, first, second);
}

float sj6_osc_pwm(Sj6Osc *o, float gain, float width)
{
    float dt = o->increment * 0.5f;
    float first_phase = o->phase;
    float mid = advance(o, dt);
    float first = (saw(first_phase, dt) - saw(first_phase + width, dt)) * 0.5f;
    float second = (saw(mid, dt) - saw(mid + width, dt)) * 0.5f;
    float g = (gain * 0.8999999761581421f) * 0.6600000262260437f;
    return decimate(o, first * g, second * g);
}

static uint32_t noise_mix(uint32_t x)
{
    x ^= x >> 16;
    x *= 0x7feb352du;
    x ^= x >> 15;
    x *= 0x846ca68bu;
    return x ^ (x >> 16);
}

void sj6_noise_reset(Sj6Noise *n)
{
    uint32_t x = n->seed + 0x9e3779b9u;
    n->seed = x ? noise_mix(x) : 0x6d2b79f5u;
}

float sj6_noise_sample(Sj6Noise *n)
{
    uint32_t x = n->seed;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    n->seed = x;
    return sj6_fma((float)(x >> 8), 1.1920930376163597e-7f, -1.f);
}
