/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once

/* Physical values of the compact patch macros. Inputs are already normalized.
 * Base cutoff and envelope depth are the (pre-limit) lowpass coefficient inputs.
 * Width and modulation depth use fractions of one oscillator cycle. */
#define JUNO6_PWM_DEPTH_MAX .2f
static inline float juno6_control_physical(unsigned id, float x)
{
    switch (id) {
    case 1: case 14: return .5f * x;         /* sub or chorus wet gain */
    case 2: return 35.f + x * x * 12000.f;  /* lowpass base Hz */
    case 4: return 7000.f * x;              /* lowpass envelope range, Hz */
    case 5: return .002f + x * x * 3.f;     /* attack, seconds */
    case 6: return .01f + x * x * 3.f;      /* decay, seconds */
    case 8: return .01f + x * x * 4.f;      /* release time constant, seconds */
    case 9: return 15.f + 2000.f * x * x;   /* high-pass Hz, bypass at x == 0 */
    case 10: return .1f + 8.f * x * x;      /* PWM LFO Hz */
    case 11: return JUNO6_PWM_DEPTH_MAX * x;
    case 12: return .1f + .8f * x;          /* base pulse duty */
    case 13: return .1f + 4.9f * x;         /* chorus Hz */
    default: return x;
    }
}
