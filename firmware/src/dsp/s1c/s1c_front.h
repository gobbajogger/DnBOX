/* SPDX-License-Identifier: GPL-3.0-only */
/* Synth One control front end for the compact engine: the original app's helper laws
 * (range clamps, tempo-synced LFO / auto-pan / delay rates, arp tempo multiplier, A4
 * truncation) applied to raw program or editor writes before they reach s1c_set. */
#ifndef S1C_FRONT_H
#define S1C_FRONT_H
#include "s1c.h"
int s1c_front_set(s1c *s, unsigned id, float value);   /* 1 = applied */
float s1c_front_clamp(unsigned id, float value);
float s1c_front_init(unsigned id);                    /* INIT value of a parameter */
#endif
