/* SPDX-License-Identifier: GPL-3.0-only */
/* Synth One / SuperJ6 arpeggiator + 16-step ARP SEQ hooks (H1-H5), Synths-owned.
 * Signatures follow /workspace/page-compliance/SYNTHS-HOOKS.md. All engine_arp_*
 * calls are main-loop only: reads take a short plat_cq_lock copy, writes queue
 * one pending block per voice (newest wins) that the audio side applies at the
 * next engine_render boundary and writes to proj.sound.v[T_303][2 + voice]. */
#pragma once
#include <stdint.h>
/* THE arp config switch (engine, runtime and ui_arp_hooks.h all read this one;
 * build with -DX0X_ARP_HOOKS=0/1 to override). 0 compiles the whole arp and
 * its UI pages out (types only). */
#ifndef X0X_ARP_HOOKS
#define X0X_ARP_HOOKS 0
#endif
/* Host UI tests swap the hooks for a fake (X0X_ARP_FAKE_H, same types);
 * the engine side then builds as hooks-off (X0X_ARP_ENGINE 0) and ui.c
 * includes the fake itself (ui_arp_hooks.h). */
#ifdef X0X_ARP_FAKE_H
#define X0X_ARP_ENGINE 0
#else
#define X0X_ARP_ENGINE X0X_ARP_HOOKS
enum { ARP_STEPS = 16 };
enum { ARP_UP, ARP_DOWN, ARP_UPDN, ARP_SEQ, ARP_NMODES };      /* MODE knob order on screen */
enum { ARP_NSPEED = 10, ARP_SPEED_1_16 = 8 };                   /* == CHI_ARP_NSPEED, CHI_ARP_SPD_1_16 */

typedef struct {
    uint32_t revision;      /* bumps on preset change / project load / any applied arp edit */
    uint8_t on;             /* 0/1 */
    uint8_t mode;           /* ARP_* */
    uint8_t range;          /* octaves 1..4 */
    uint8_t speed;          /* CHI_ARP_SPD_* index 0..9 (chicago_arp.h) */
    uint8_t length;         /* ARP SEQ steps 1..16 */
    uint8_t interval;       /* 0..12 st between octave passes (12 = octave) */
    uint8_t note_locked;    /* always 0: arp is not part of the note-lock cell */
    uint8_t gate;           /* appended: 1..100 % of a step */
} engine_arp_settings;

typedef struct {
    uint32_t revision;              /* same counter as settings */
    int8_t  offset[ARP_STEPS];      /* semitones from the held key: S1 -12..12, SJ6 -24..24 */
    uint16_t on_mask;               /* bit n: step n sounds */
    uint16_t oct_mask;              /* bit n: +1 octave */
} engine_arp_steps;

enum { ARP_F_ON, ARP_F_MODE, ARP_F_RANGE, ARP_F_SPEED, ARP_F_LENGTH, ARP_F_INTERVAL, ARP_F_GATE, ARP_NF };
enum { ARP_S_OFFSET, ARP_S_ON, ARP_S_OCT };    /* per-step field */

/* Persistent block, explicit bytes (layout in engine_arp_block_* below). */
#define ARP_BLOCK_BYTES 32u
#define ARP_BLOCK_VOICE0 2u               /* stored at sound.v[T_303][2 + voice] */
typedef struct {
    uint8_t on, mode, range, speed, length, interval, gate, src_type; /* src_type 0xFF: none */
    int8_t offset[ARP_STEPS];
    uint16_t on_mask, oct_mask;
    uint8_t amp2_mask;                    /* bit n: SJ6 field 119 + n stored */
    uint16_t amp2[4];                     /* unit * 65535 */
} engine_arp_cfg;
void engine_arp_cfg_default(engine_arp_cfg *);
/* 1 = a valid v1 block decoded; 0 = absent/unknown version, *out = defaults. */
int engine_arp_block_decode(const uint8_t block[ARP_BLOCK_BYTES], engine_arp_cfg *out);
void engine_arp_block_encode(const engine_arp_cfg *, uint8_t block[ARP_BLOCK_BYTES]);
/* Preset field decoders (pure). Return 1 when the id is an arp/seq/AMP2 field. */
int engine_arp_from_sj6(engine_arp_cfg *, unsigned id, float value);
int engine_arp_from_s1(engine_arp_cfg *, unsigned id, float value);
/* Frames (44.1 kHz) per arp step at bpm for a SPEED index. */
uint32_t engine_arp_step_frames(float bpm, unsigned speed);

#if X0X_ARP_ENGINE
#if !X0X_SYNTHONE_PATCH
enum { S1_EDIT_REJECTED, S1_EDIT_QUEUED, S1_EDIT_BUSY };
#endif
int engine_arp_settings_read(unsigned voice, engine_arp_settings *out);
int engine_arp_set(unsigned voice, uint32_t revision, unsigned field, int value);
int engine_arp_steps_read(unsigned voice, engine_arp_steps *out);
int engine_arp_step_set(unsigned voice, uint32_t revision, unsigned step, unsigned field, int value);
int engine_arp_view(unsigned voice, unsigned *step, uint32_t *fired_age_ms);
/* Re-queue the stored block (after an undo of proj.sound). */
int engine_arp_reload(unsigned voice);
/* Replace the whole arp block from a preset reader; returns S1_EDIT_*. */
int engine_arp_preset(unsigned voice, const engine_arp_cfg *cfg);
#endif
#endif /* !X0X_ARP_FAKE_H */
