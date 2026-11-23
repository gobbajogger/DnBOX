/* SPDX-License-Identifier: GPL-3.0-only */
/* Synth One part on the compact engine (dsp/s1c, option B: its own delay and reverb).
 * Same s1_part_* API as the dry voice part it replaces (X0X_S1C=1).
 * RAM: the engine core and its widen allpasses live in this struct, inside the runtime
 * backend union (no growth: checked against the Miami backend). The reverb lines and the
 * delay pool borrow the part's Chicago FX2 compact object, idle while the part plays
 * Synth One; original_runtime_retire re-initialises it before Chicago can use it again. */
#pragma once
#include "../dsp/s1c/s1c_config.h"
#include "../dsp/s1c/s1c.h"
#if X0X_SYNTHONE_PATCH
#include "s1_patch.h"
#endif

enum { S1_CORE_VOICES = S1C_VOICES, S1_PART_INIT = 0xFFFFu };

typedef struct {
    s1c e;
    uint16_t wbuf[S1C_WAP_WORDS];
    uint8_t *arena;            /* S1C_ARENA_BYTES, or NULL: no delay/reverb */
    uint16_t program;          /* pack program, or S1_PART_INIT */
    float detm;                /* program DETM: pitch bend scales it */
    uint8_t damper, held;      /* held: voices kept sounding by the damper */
} s1_dry_part;

/* Original bitCrushSampleRate is 2048..48000 Hz. Turning up the crush
 * amount lowers the clock from the INIT value to the minimum. */
static inline float s1_part_crush_hz(float amount) { return 48000.f - 45952.f * amount; }

/* Factory pack (tools/pack_synthone_factory.py image) and its size; NULL until the
 * board maps it. Set once at boot (device) or by a host test. */
extern const uint8_t *s1_part_pack;
extern uint32_t s1_part_pack_bytes;

void s1_part_init(s1_dry_part *part);                    /* INIT program, no effects arena */
void s1_part_bind(s1_dry_part *part, void *arena);       /* lend S1C_ARENA_BYTES (or NULL) */
int s1_part_program(s1_dry_part *part, unsigned program); /* pack program or S1_PART_INIT; 1 = loaded */
unsigned s1_part_programs(void);                         /* programs in the mapped pack, 0 if none */
/* Display name of pack program n (cold path, one shared buffer): "S1 n+1" without a pack. */
const char *s1_part_program_name(unsigned program);
void s1_part_all_off(s1_dry_part *part);
#if X0X_SYNTHONE_PATCH
int s1_patch_slot(unsigned factory_id);
float s1_part_patch_value(const s1_dry_part *, unsigned slot);
void s1_part_apply_cell(s1_dry_part *, const uint8_t cell[32]);
#endif
int s1_part_param(s1_dry_part *part, unsigned id, float value);
int s1_part_note(s1_dry_part *part, unsigned key, unsigned velocity);
int s1_part_active(const s1_dry_part *part);
/* UI observation: loudest voice's amp envelope, filter envelope, stage (0 attack,
 * 1 decay/sustain, 3 release) and gate. 0 when silent. */
int s1_part_voice_view(const s1_dry_part *part, float *level, float *modulation, unsigned *state, unsigned *key_down);
void s1_part_render(s1_dry_part *part, float *left, float *right, unsigned frames);
