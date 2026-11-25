/* SPDX-License-Identifier: GPL-3.0-only */
/* Synth One control front end for the compact engine: the original app's helper laws
 * (range clamps, tempo-synced LFO / auto-pan / delay rates, arp tempo multiplier, A4
 * truncation) applied to raw program or editor writes before they reach s1c_set. */
#ifndef S1C_FRONT_H
#define S1C_FRONT_H
#include "s1c.h"
int s1c_front_set(s1c *s, unsigned id, float value);   /* 1 = applied */
float s1c_front_clamp(unsigned id, float value);
void s1c_front_bounds(unsigned id, float *minimum, float *maximum);
#if X0X_SYNTHONE_PATCH
#if defined(X0X_S1_MOD_RATE_PROBE) && X0X_S1_MOD_RATE_PROBE
void s1c_front_mod_rate(s1c *, unsigned id, unsigned cc1);
#endif
unsigned s1c_front_lfo_steps(const s1c *s);
unsigned s1c_front_lfo_position(const s1c *s, unsigned id);
void s1c_front_lfo_apply(s1c *s, unsigned id, unsigned code);
#endif
float s1c_front_init(unsigned id);                    /* INIT value of a parameter */
#endif

#if defined(X0X_S1_AUTOPAN_PROBE) && X0X_S1_AUTOPAN_PROBE
unsigned s1c_front_pan_stops(const s1c *);
int s1c_front_pan_select(s1c *,unsigned);
unsigned s1c_front_pan_position(const s1c *);
int s1c_front_pan_sync(const s1c *);
#endif
