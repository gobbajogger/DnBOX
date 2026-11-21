/* SPDX-License-Identifier: GPL-3.0-only */
/* Isolated RESOURCE FIXTURE: stock 18-control facade over the fresh core.
 * This file is never a production proposal or original-control equivalence. */
#pragma once
#include <stdint.h>
#include "envelope_snapshot.h"
#include "dnbx_core.h"
#define JUNO6_VOICES 6u
#define JUNO6_CHORUS_BYTES 512u
#define JUNO6_CHORUS_SAMPLES 256u
typedef struct {
 dnbx_part core;
 float macro[18];
 float hpf, hp_state[2], hp_input[2];
} juno6_t;
typedef struct {
 int16_t delay[JUNO6_CHORUS_SAMPLES];
 uint16_t write;
 uint32_t phase;
 float sample_rate,depth_ms,rate_hz,mix;
} juno6_chorus_t;
void juno6_init(juno6_t*,float);
int juno6_note_on(juno6_t*,uint8_t,uint8_t);
void juno6_note_off(juno6_t*,uint8_t);
void juno6_all_off(juno6_t*);
int juno6_render_stereo(juno6_t*,float*,float*,unsigned);
void juno6_chorus_init(juno6_chorus_t*,float);
void juno6_chorus_set(juno6_chorus_t*,float,float,float);
void juno6_chorus_process(juno6_chorus_t*,float*,float*,unsigned);
int juno6_set(juno6_t*,unsigned,float);
int juno6_active(const juno6_t*);
int juno6_read_envelope(const juno6_t*,juno6_envelope_snapshot*);
