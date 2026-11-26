#ifndef DNBX_HOST_SJ6_FILTER_PANEL_H
#define DNBX_HOST_SJ6_FILTER_PANEL_H
#include "warm_amp.h"
/* Old original Conductor -> AKCoreSampler. The separate 18.99x sender
 * targets setFilterResonance, not the per-sampler sustain parameter. */
static inline float sj6_filter_attack_panel(float raw)
{
    return sj6_amp_panel(raw,.002,16.,3.33);
}
static inline float sj6_filter_decay_panel(float raw)
{
    return sj6_amp_panel(raw,.002,24.,3.35);
}
static inline float sj6_filter_sustain_panel(float raw)
{
    return sj6_amp_panel(raw,0.,1.,2.5);
}
static inline float sj6_filter_strength_panel(float raw)
{
    return sj6_amp_panel(raw,0.,1.,3.03);
}
static inline float sj6_filter_tracking_panel(float raw)
{
    return .25f*raw;
}
static inline float sj6_filter_release_panel(float raw)
{
    const double taper=.002+(24.-.002)*pow((double)raw,3.35);
    return (float)(taper+.2);
}
static inline float sj6_filter_velocity_panel(float raw)
{
    return sj6_amp_panel(raw,0.,1.1,1.);
}
/* The filter resonance sender and bus volume sender are independent.
 * The DSP first narrows pushed gain to its Float32 slot, then exponentiates. */
static inline float sj6_filter_resonance_slot(float raw)
{
    return sj6_amp_panel(raw,0.,18.99,1.);
}
static inline float sj6_filter_damping_panel(float raw)
{
    const float slot=sj6_filter_resonance_slot(raw);
    return (float)pow(10.,(double)slot*-.05);
}
#endif
