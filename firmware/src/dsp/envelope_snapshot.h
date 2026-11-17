/* SPDX-License-Identifier: GPL-3.0-only */
/* Caller-owned observation values only. No generated tables, DSP owner state,
 * allocation or new publication semantics. Accessors retain their documented
 * serialized-call and disjoint-output requirements in their engine headers. */
#pragma once
#include <stdint.h>

typedef struct {
 float adsr[4], level;
 int selected_voice;
 unsigned stage, gate, active, shared_amp_filter;
} juno6_envelope_snapshot;

typedef struct {
 float raw_level, target_decay_ms, target_hold_ms;
 float ramp_start, ramp_end, ramp_samples, ramp_ratio;
 int32_t hold_left, ramp_left, ramp_ticks;
 unsigned active, hold_active, ramp_valid;
} drum909_amp_snapshot;
