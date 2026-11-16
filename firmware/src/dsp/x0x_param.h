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

/* X0X's pan: a balance law, pot 14..114 (clamped, below), 64 = centre. Centre is 1 on both sides exactly (a centred
 * mix is the mono one, bit for bit); toward a side the other fades along a quarter cosine and the
 * near side stays at 1. cosine: a 5-term even polynomial, |error| < 2e-4 over the quarter turn.
 * Bodies live in x0x_param.c so 808 / 909 / engine share one copy (was static inline per TU). */
void x0x_pan_gains(int pot, float *l, float *r);

/* Pan clips at 50 either side (L50 .. C .. R50). Storage keeps the pot format (raw 0..127, 64 =
 * centre, so files stay compatible); every entry point clamps through x0x_pan_clamp to 14..114.
 * A legacy value past the clip plays, reads and marks exactly as the clip (L50 / R50). */
enum { X0X_PAN_C = 64, X0X_PAN_SPAN = 50, X0X_PAN_MIN = X0X_PAN_C - X0X_PAN_SPAN, X0X_PAN_MAX = X0X_PAN_C + X0X_PAN_SPAN };
static inline int x0x_pan_clamp(int pot)
{
    return pot < X0X_PAN_MIN ? X0X_PAN_MIN : pot > X0X_PAN_MAX ? X0X_PAN_MAX : pot;
}
/* an absolute control's full travel (0..127, e.g. a fader or SMC knob) onto L50 .. R50, no dead
 * travel: 0 -> 14, 64 -> 64, 127 -> 114 */
static inline int x0x_pan_from_travel(int t)
{
    t = t < 0 ? 0 : t > 127 ? 127 : t;
    return t <= 64 ? X0X_PAN_MIN + (t * X0X_PAN_SPAN + 32) / 64 : X0X_PAN_C + ((t - 64) * X0X_PAN_SPAN + 31) / 63;
}
/* the inverse, for pickup against the stored value: 14 -> 0, 64 -> 64, 114 -> 127 */
static inline int x0x_pan_to_travel(int pot)
{
    pot = x0x_pan_clamp(pot);
    return pot <= X0X_PAN_C ? ((pot - X0X_PAN_MIN) * 64 + 25) / X0X_PAN_SPAN : 64 + ((pot - X0X_PAN_C) * 63 + 25) / X0X_PAN_SPAN;
}
/* a marker's offset along a w-pixel rail: L50 at 0, C at w / 2, R50 at w (readout and marker share it) */
static inline int x0x_pan_pos(int pot, int w) { return (x0x_pan_clamp(pot) - X0X_PAN_MIN) * w / (2 * X0X_PAN_SPAN); }
/* the signed amount a readout prints: -50 .. 0 .. +50 (L50 .. C .. R50) */
static inline int x0x_pan_amount(int pot) { return x0x_pan_clamp(pot) - X0X_PAN_C; }

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
