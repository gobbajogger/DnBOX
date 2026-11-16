/* SPDX-License-Identifier: GPL-3.0-only */
/* X0X parameter descriptors, shared by every sound engine.
 *
 * Every user-facing sound parameter is a "pot": an integer 0..max (max is 127 for a
 * continuous control, n-1 for an n-way switch), exactly like 9W9's panel. The UI
 * draws and edits them from these descriptors alone, and a project stores the
 * integers, so a saved sound is engine-version independent as long as a pot keeps
 * its meaning. */
#pragma once
#include <stdint.h>

/* X0X's pan: a balance law, pot 0..127, 64 = centre. Centre is 1 on both sides exactly (a centred
 * mix is the mono one, bit for bit); toward a side the other fades along a quarter cosine and the
 * near side stays at 1. cosine: a 5-term even polynomial, |error| < 2e-4 over the quarter turn.
 * Bodies live in x0x_param.c so 808 / 909 / engine share one copy (was static inline per TU). */
void x0x_pan_gains(int pot, float *l, float *r);

/* Pan DISPLAY: L50 .. C .. R50 over the unchanged full range. Storage, the law above, MIDI / Remote CC,
 * presets and loads stay raw 0..127 (64 = centre), never clamped: hard left (0) and right (127) stay.
 *   x0x_pan_disp(raw) = clamp(round_half_away((raw - 64) * 50 / 63), -50, +50)   raw 0 and 1 both read L50
 *   x0x_pan_raw(d)    = 64 + round_half_away(d * 63 / 50), d = -50 -> 0, d = +50 -> 127 (both ends reachable)
 * x0x_pan_disp(x0x_pan_raw(d)) == d for every d; an edit moves in display steps (one detent = one step).
 * Readouts print the display value; markers sit at x0x_pan_pos, from the same value. */
static inline int x0x_div_half_away(int n, int d)                  /* n / d rounded half away from 0, d > 0 */
{
    return n >= 0 ? (2 * n + d) / (2 * d) : -((2 * -n + d) / (2 * d));
}
static inline int x0x_pan_disp(int raw)
{
    int d = x0x_div_half_away((raw - 64) * 50, 63);
    return d < -50 ? -50 : d > 50 ? 50 : d;
}
static inline int x0x_pan_raw(int d)
{
    if (d <= -50)
        return 0;
    if (d >= 50)
        return 127;
    return 64 + x0x_div_half_away(d * 63, 50);
}
/* a marker's offset along a w-pixel rail: L50 at 0, C at w / 2, R50 at w */
static inline int x0x_pan_pos(int raw, int w) { return (x0x_pan_disp(raw) + 50) * w / 100; }

/* a pan gain gliding toward its target over a block of n samples (one pole, ~10 ms): returns the
 * per-sample step for a linear ramp from *cur, and moves *cur to where the block ends. Settled, the
 * step is 0 and *cur is the target exactly */
float x0x_glide_block(float *cur, float tgt, int n);

typedef struct {
    const char *name;                 /* <= 6 characters: drawn above a knob */
    uint8_t max;                      /* 127 = continuous; else switch with max+1 positions */
    uint8_t def;                      /* power-on value */
    const char *const *names;         /* switch: max+1 labels (<= 5 chars), else 0 */
} x0x_param_t;

/* the physical unit of a pot's engine value, for UI readouts (x0x_param_si) */
enum { X0X_U_NONE, X0X_U_PCT, X0X_U_GAIN, X0X_U_HZ, X0X_U_MS, X0X_U_S, X0X_U_ST, X0X_U_RATIO };
