/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef ORIGINAL_LINEAR_CONTROL_H
#define ORIGINAL_LINEAR_CONTROL_H

/* All 128 authored Q15 control words equal floor(32767 * value / 127).
 * 32767 = 258 * 127 + 1, so only the maximum value has a remainder. */
static inline int original_linear_control(unsigned value)
{
    return (int)(value * 258u + (value == 127u));
}

/* The remaining exact control curves are evaluated only by setters. All
 * ratios below are integer encodings verified against every original word. */
static inline int original_drive_gain(unsigned value)
{
    return (int)(32767u * value * value / (127u * 127u));
}

static inline int original_chorus_wet(unsigned value)
{
    return value < 32u ? (int)(value * 1024u) : 32767;
}

static inline int original_chorus_feedback(unsigned value)
{
    return value <= 32u ? 0 : (int)((value - 32u) * 6488u / 19u);
}

static inline unsigned original_decimator_frequency(unsigned value)
{
    static const unsigned short high[] = {23000, 30000, 39000, 50000, 65535};
    if (value >= 123u) return high[value - 123u];
    return 1486u + (value * value * 2453u) / 2137u;
}
#endif
