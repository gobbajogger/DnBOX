/* SPDX-License-Identifier: GPL-3.0-only */
#include "original_runtime.h"
#include "../dsp/juno6_controls.h"
#include "../x0x_cold.h"
#if X0X_S1C
#include "chicago_fx2_compact.h"
#endif
#include <string.h>
#if X0X_SJ6_NATIVE_DELAY && X0X_SUPERJ6_FACTORY
#include "lab_delay.inc"
#endif

static int runtime_result(original_runtime_part *p, int ok)
{
    if (!ok) p->status = ORIGINAL_RUNTIME_FAULT;
    return p->status;
}

X0X_COLD int original_runtime_init(original_runtime_part *p, const uint8_t cell[32], float bpm,
    original_chicago_tuning *tuning, const original_chicago_fx2_cold_owner *fx)
{
    if (!p || !cell) return ORIGINAL_RUNTIME_INVALID;
    (void)original_sound_adopt_legacy_cell((uint8_t *)cell);
    if (!original_sound_valid(cell)) return ORIGINAL_RUNTIME_INVALID;
    unsigned kind = cell[29];
    unsigned factory = (unsigned)original_sound_factory_source(kind, cell[30]);
    /* The old active union is intentionally retired at this explicit cold
     * boundary. A caller must inspect the returned admission result. */
    memset(p, 0, sizeof *p);
    p->kind = (uint8_t)kind;
    int rc;
    if (kind == ORIGINAL_SOUND_JUNO) {
        juno6_init(&p->dsp.juno.synth,44100);
        juno6_chorus_init(&p->dsp.juno.chorus,44100);
        rc=ORIGINAL_RUNTIME_OK;
    } else if (kind == ORIGINAL_SOUND_SYNTHONE) {
        s1_part_init(&p->dsp.synthone);
#if X0X_S1C
        /* The part's Chicago FX2 object is idle while it plays Synth One: its
         * reverb lines and delay pool live there (retire re-initialises it). */
        if (!fx || !fx->context) return p->status = ORIGINAL_RUNTIME_NEEDS_FX;
        s1_part_bind(&p->dsp.synthone, fx->context);
        {
            unsigned program = original_sound_s1_program(cell);
            /* A missing pack or program plays INIT: still a valid, selectable sound. */
            if (program) (void)s1_part_program(&p->dsp.synthone, program - 1u);
        }
#endif
#if X0X_SYNTHONE_PATCH
        if (original_sound_edit_mask(cell) & S1_PATCH_MASK) s1_part_apply_cell(&p->dsp.synthone, cell);
#endif
        rc = ORIGINAL_RUNTIME_OK;
    } else if (kind == ORIGINAL_SOUND_MIAMI)
        rc = original_miami_backend_init(&p->dsp.miami, factory, bpm) ? 1 : -2;
    else
        rc = original_chicago_backend_init_compact(&p->dsp.chicago, factory, bpm, 0, tuning, fx);
    p->status = (int8_t)rc;
    if (rc != ORIGINAL_RUNTIME_OK) return rc;
    if (kind == ORIGINAL_SOUND_JUNO) {
        for(unsigned i=0;i<ORIGINAL_JUNO_NPARAMS-2u;i++){
            float value;
            if(!original_sound_value(cell,i,&value) || original_runtime_param(p,i,value)!=ORIGINAL_RUNTIME_OK) return runtime_result(p,0);
        }
    }
    uint32_t edits = original_sound_edit_mask(cell);
    /* All admitted factories have neutral bend/damper. Archived performance
     * edit bits never become inputs to a newly adopted runtime instance. */
    for (unsigned i = 0; i < original_sound_count(kind); ++i)
        if ((edits & (UINT32_C(1) << i)) && !original_sound_is_performance(kind, i)) {
        float value;
        if (!original_sound_value(cell, i, &value) ||
            original_runtime_param(p, i, value) != ORIGINAL_RUNTIME_OK)
            return runtime_result(p, 0);
    }
    /* A cold adoption can occur inside the current audio boundary. Publish a
     * prepared instance so its first queued/sequence note never precedes the
     * threshold snapshot. Further prepare calls are explicitly idempotent. */
    return original_runtime_prepare(p, bpm);
}

#if X0X_ARP_ENGINE
static void arp_params(original_runtime_part *p);
#endif
int original_runtime_prepare(original_runtime_part *p, float bpm)
{
    if (!p || p->status != ORIGINAL_RUNTIME_OK) return p ? p->status : 0;
    int ok;
    if (p->kind == ORIGINAL_SOUND_JUNO || p->kind == ORIGINAL_SOUND_SYNTHONE) {
#if X0X_ARP_ENGINE
        p->arp.bpm = bpm;   /* only the arp clock follows tempo: idempotent */
        arp_params(p);      /* engine_arp.h; defined with the arp below */
#else
        (void)bpm;          /* no tempo-dependent parameters: idempotent */
#endif
#if X0X_SJ6_NATIVE_DELAY && X0X_SUPERJ6_FACTORY
        original_runtime_delay_drain(p,bpm);
#endif
        ok = 1;
    } else if (p->kind == ORIGINAL_SOUND_MIAMI) {
        ok = original_miami_backend_set_tempo(&p->dsp.miami, bpm) &&
             original_miami_backend_prepare(&p->dsp.miami);
    } else {
        ok = original_chicago_backend_tempo(&p->dsp.chicago, bpm) == ORIGINAL_CHICAGO_BACKEND_OK &&
             original_chicago_backend_prepare(&p->dsp.chicago) == ORIGINAL_CHICAGO_BACKEND_OK;
    }
    return runtime_result(p, ok);
}

int original_runtime_retire(original_runtime_part *p)
{
    if (!p) return ORIGINAL_RUNTIME_INVALID;
    if (p->kind == ORIGINAL_SOUND_CHICAGO && p->status != ORIGINAL_RUNTIME_NEEDS_FX && p->dsp.chicago.ready) {
        int rc = original_chicago_backend_retire(&p->dsp.chicago);
        if (rc != ORIGINAL_CHICAGO_BACKEND_OK) return rc;
    }
#if X0X_S1C
    if (p->kind == ORIGINAL_SOUND_SYNTHONE && p->dsp.synthone.arena) {
        (void)original_chicago_fx2_compact_init((original_chicago_fx2_compact *)(void *)p->dsp.synthone.arena);
        p->dsp.synthone.arena = NULL;
    }
#endif
#if X0X_SJ6_NATIVE_DELAY && X0X_SUPERJ6_FACTORY
    if (p->kind==ORIGINAL_SOUND_JUNO) {
        memset(&p->dsp.juno.lab_delay,0,sizeof p->dsp.juno.lab_delay);
        memset(&p->dsp.juno.synth.delay24,0,sizeof p->dsp.juno.synth.delay24);
    }
#endif
    p->status = ORIGINAL_RUNTIME_INVALID;
    return ORIGINAL_RUNTIME_OK;
}
#if X0X_S1C
_Static_assert(sizeof(original_chicago_fx2_compact) >= S1C_ARENA_BYTES, "Synth One effects fit the Chicago FX2 arena");
_Static_assert(sizeof(s1_dry_part) <= sizeof(original_miami_backend), "Synth One part does not grow the backend union");

int original_runtime_s1_program(original_runtime_part *p, unsigned program)
{
    if (!p || p->status != ORIGINAL_RUNTIME_OK || p->kind != ORIGINAL_SOUND_SYNTHONE) return ORIGINAL_RUNTIME_INVALID;
    return s1_part_program(&p->dsp.synthone, program) ? ORIGINAL_RUNTIME_OK : ORIGINAL_RUNTIME_INVALID;
}
#endif

int original_runtime_param(original_runtime_part *p, unsigned parameter, float value)
{
    if (!p || p->status != ORIGINAL_RUNTIME_OK) return p ? p->status : 0;
    if(p->kind==ORIGINAL_SOUND_JUNO){
        int ok=juno6_set(&p->dsp.juno.synth,parameter,value);
#if X0X_SUPERJ6_FACTORY
        if (!ok && parameter >= JUNO6_APP_PARAM_BASE + 119 &&
            parameter <= JUNO6_APP_PARAM_BASE + 122 && !juno6_can_edit_amp2(&p->dsp.juno.synth))
            return ORIGINAL_RUNTIME_INVALID;
        if (parameter >= JUNO6_APP_PARAM_BASE) return runtime_result(p, ok);
#endif
        if (ok) juno6_chorus_set(&p->dsp.juno.chorus,
            juno6_control_physical(13, p->dsp.juno.synth.macro[13]), 5.4f,
            juno6_control_physical(14, p->dsp.juno.synth.macro[14]));
        return runtime_result(p,ok);
    }
    if (p->kind == ORIGINAL_SOUND_SYNTHONE)
        return runtime_result(p, s1_part_param(&p->dsp.synthone, parameter, value));
    return runtime_result(p, p->kind == ORIGINAL_SOUND_MIAMI ?
        original_miami_backend_set_macro(&p->dsp.miami, parameter, value) :
        original_chicago_backend_set(&p->dsp.chicago, parameter, value) == ORIGINAL_CHICAGO_BACKEND_OK);
}

static int runtime_note(original_runtime_part *p, unsigned key, unsigned velocity)
{
    if(p->kind==ORIGINAL_SOUND_JUNO){
        if(velocity) return runtime_result(p,juno6_note_on(&p->dsp.juno.synth,(uint8_t)key,(uint8_t)velocity));
        juno6_note_off(&p->dsp.juno.synth,(uint8_t)key);return ORIGINAL_RUNTIME_OK;
    }
    if (p->kind == ORIGINAL_SOUND_SYNTHONE) {
        return runtime_result(p, s1_part_note(&p->dsp.synthone,key,velocity));
    }
    return runtime_result(p, p->kind == ORIGINAL_SOUND_MIAMI ?
        (velocity ? original_miami_backend_note_on(&p->dsp.miami, key, velocity) :
                    original_miami_backend_note_off(&p->dsp.miami, key)) :
        original_chicago_backend_note(&p->dsp.chicago, key, velocity) == ORIGINAL_CHICAGO_BACKEND_OK);
}

static int key_held(const original_runtime_part *p, unsigned key)
{
    for (unsigned s = 0; s < ORIGINAL_INPUT_COUNT; ++s) if (p->keys[s][key]) return 1;
    return 0;
}

#if X0X_ARP_ENGINE
/* ---- Persistent arp block v1 (engine_arp.h). Explicit bytes, no ABI layout. */
void engine_arp_cfg_default(engine_arp_cfg *c)
{
    memset(c, 0, sizeof *c);
    c->mode = ARP_UP; c->range = 1; c->speed = ARP_SPEED_1_16; c->length = ARP_STEPS;
    c->interval = 12; c->gate = 100; c->src_type = 0xFF; c->on_mask = 0xFFFF;
}
static uint8_t arp_clamp(unsigned v, unsigned lo, unsigned hi) { return (uint8_t)(v < lo ? lo : v > hi ? hi : v); }
int engine_arp_block_decode(const uint8_t b[ARP_BLOCK_BYTES], engine_arp_cfg *c)
{
    engine_arp_cfg_default(c);
    if (!b || b[0] != 'A' || b[1] != 1u) return 0;
    c->on = b[2] & 1u;
    c->mode = (uint8_t)(b[2] >> 1 & 3u);
    c->range = (uint8_t)((b[2] >> 3 & 3u) + 1u);
    c->speed = arp_clamp(b[3] & 15u, 0, ARP_NSPEED - 1u);
    c->length = (uint8_t)((b[3] >> 4) + 1u);
    c->interval = arp_clamp(b[4], 0, 12);
    c->gate = arp_clamp(b[5], 1, 100);
    c->src_type = b[6];
    c->amp2_mask = b[7] & 15u;
    for (unsigned i = 0; i < 4; ++i) c->amp2[i] = (uint16_t)(b[8 + 2 * i] | b[9 + 2 * i] << 8);
    c->on_mask = c->oct_mask = 0;
    for (unsigned i = 0; i < ARP_STEPS; ++i) {
        uint8_t s = b[16 + i];
        c->offset[i] = (int8_t)((int)arp_clamp(s & 63u, 0, 48) - 24);
        c->on_mask |= (uint16_t)((s >> 6 & 1u) << i);
        c->oct_mask |= (uint16_t)((s >> 7) << i);
    }
    return 1;
}
void engine_arp_block_encode(const engine_arp_cfg *c, uint8_t b[ARP_BLOCK_BYTES])
{
    memset(b, 0, ARP_BLOCK_BYTES);
    b[0] = 'A'; b[1] = 1u;
    b[2] = (uint8_t)((c->on & 1u) | (arp_clamp(c->mode, 0, ARP_NMODES - 1u) << 1) | ((arp_clamp(c->range, 1, 4) - 1u) << 3));
    b[3] = (uint8_t)(arp_clamp(c->speed, 0, ARP_NSPEED - 1u) | ((arp_clamp(c->length, 1, 16) - 1u) << 4));
    b[4] = arp_clamp(c->interval, 0, 12);
    b[5] = arp_clamp(c->gate, 1, 100);
    b[6] = c->src_type;
    b[7] = c->amp2_mask & 15u;
    for (unsigned i = 0; i < 4; ++i) { b[8 + 2 * i] = (uint8_t)c->amp2[i]; b[9 + 2 * i] = (uint8_t)(c->amp2[i] >> 8); }
    for (unsigned i = 0; i < ARP_STEPS; ++i) {
        int o = c->offset[i] < -24 ? -24 : c->offset[i] > 24 ? 24 : c->offset[i];
        b[16 + i] = (uint8_t)((o + 24) | (c->on_mask >> i & 1u) << 6 | (c->oct_mask >> i & 1u) << 7);
    }
}
static int arp_round(float v, int lo, int hi)
{
    if (!(v == v)) return lo;
    int r = (int)(v < 0 ? v - .5f : v + .5f);
    return r < lo ? lo : r > hi ? hi : r;
}
/* SuperJ6 app ids (public control catalog): 63..66 arp, 142..145 seq,
 * 147..162 step offsets (1/48 of the -24..24 knob), 163..178 toggles, 119..122
 * AMP2. arpEnable / seqEnable are exclusive in the app; SEQ wins if both. */
int engine_arp_from_sj6(engine_arp_cfg *c, unsigned id, float v)
{
    /* 12 seqDivision stops (provisional names 1/1 .. 1/64) -> CHI_ARP_SPD_*. */
    static const uint8_t div[12] = {0, 0, 2, 3, 5, 6, 7, 8, 9, 9, 9, 9};
    if (!c || !(v == v)) return 0;
    if (id >= 147 && id <= 162) { c->offset[id - 147] = (int8_t)arp_round((v - .5f) * 48.f, -24, 24); return 1; }
    if (id >= 163 && id <= 178) {
        uint16_t bit = (uint16_t)(1u << (id - 163));
        c->on_mask = (uint16_t)(v >= .5f ? c->on_mask | bit : c->on_mask & ~bit);
        return 1;
    }
    if (id >= 119 && id <= 122) {
        float u = v < 0 ? 0 : v > 1 ? 1 : v;
        c->amp2[id - 119] = (uint16_t)(u * 65535.f + .5f);
        c->amp2_mask |= (uint8_t)(1u << (id - 119));
        return 1;
    }
    switch (id) {
    case 63:
        if (c->mode != ARP_SEQ || !c->on) c->on = v >= .5f;
        return 1;
    case 64: {
        int t = arp_round(v * 6.f, 0, 6);
        c->src_type = (uint8_t)t;
        if (c->mode != ARP_SEQ) c->mode = (uint8_t)(t <= ARP_UPDN ? t : ARP_UP);
        return 1;
    }
    case 65: c->range = (uint8_t)arp_round(v * 3.f + 1.f, 1, 4); return 1;
    case 66: c->interval = (uint8_t)arp_round(v * 12.f, 0, 12); return 1;
    case 142:
        if (v >= .5f) { c->on = 1; c->mode = ARP_SEQ; }
        else if (c->mode == ARP_SEQ) c->mode = (uint8_t)(c->src_type <= ARP_UPDN ? c->src_type : ARP_UP);
        return 1;
    case 143: {
        int k = (int)(v * 12.f);
        c->speed = div[k < 0 ? 0 : k > 11 ? 11 : k];
        return 1;
    }
    case 144: c->gate = (uint8_t)arp_round(v * 100.f, 1, 100); return 1;
    case 145: c->length = (uint8_t)arp_round(v * 15.f + 1.f, 1, 16); return 1;
    default: return 0;
    }
}
/* Synth One parameter ids (S1Parameter.h): 60 arpDirection (0 up, 1 up/down,
 * 2 down), 61 arpInterval, 62 arpIsOn, 63 arpOctave 0..3, 65 arpIsSequencer,
 * 66 arpTotalSteps, 67..82 pattern, 83..98 octave boost, 99..114 note on.
 * 64 arpRate and 147 arpSeqTempoMultiplier are free BPM in the source; the
 * firmware arp follows project tempo with SPEED, so they are accepted only. */
int engine_arp_from_s1(engine_arp_cfg *c, unsigned id, float v)
{
    if (!c || !(v == v)) return 0;
    if (id >= 67 && id <= 82) { c->offset[id - 67] = (int8_t)arp_round(v, -12, 12); return 1; }
    if (id >= 83 && id <= 114) {
        unsigned i = (id - 83) % 16u;
        uint16_t bit = (uint16_t)(1u << i), *m = id <= 98 ? &c->oct_mask : &c->on_mask;
        *m = (uint16_t)(v >= .5f ? *m | bit : *m & ~bit);
        return 1;
    }
    switch (id) {
    case 60: {
        static const uint8_t dir[3] = {ARP_UP, ARP_UPDN, ARP_DOWN};
        if (c->mode != ARP_SEQ) c->mode = dir[arp_round(v, 0, 2)];
        c->src_type = dir[arp_round(v, 0, 2)];       /* remembered for SEQ off */
        return 1;
    }
    case 61: c->interval = (uint8_t)arp_round(v, 0, 12); return 1;
    case 62: c->on = v >= .5f; return 1;
    case 63: c->range = (uint8_t)arp_round(v + 1.f, 1, 4); return 1;
    case 65:
        if (v >= .5f) c->mode = ARP_SEQ;
        else if (c->mode == ARP_SEQ) c->mode = c->src_type <= ARP_UPDN ? c->src_type : ARP_UP;
        return 1;
    case 66: c->length = (uint8_t)arp_round(v, 1, 16); return 1;
    case 64: case 147: return 1;
    default: return 0;
    }
}

/* ---- Shared Synth One / SuperJ6 arp + ARP SEQ: the original AudioKit Synth
 * One S1Sequencer (dsp/synthone/sequencer, MIT), one copy for both owners; the
 * SuperJ6 app ("Synth One J6") runs the same sequencer. Live sources feed
 * heldNoteNumbersAE (newest first, as the source); SEQUENCE playback bypasses it. */
#include "../dsp/synthone/sequencer/S1Sequencer.c"
static int arp_engaged(const original_runtime_part *p)
{
    return p->arp.cfg.on && (p->kind == ORIGINAL_SOUND_JUNO || p->kind == ORIGINAL_SOUND_SYNTHONE);
}
static int live_source(unsigned source) { return source != ORIGINAL_INPUT_SEQUENCE; }
__attribute__((noinline)) static int live_held(const original_runtime_part *p, unsigned key)
{
    for (unsigned s = 0; s < ORIGINAL_INPUT_COUNT; ++s) if (live_source(s) && p->keys[s][key]) return 1;
    return 0;
}
static void arp_key_on(void *ctx, int note, int velocity) { (void)runtime_note(ctx, (unsigned)note, (unsigned)velocity); }
static void arp_key_off(void *ctx, int note)
{
    original_runtime_part *p = ctx;
    if (!p->keys[ORIGINAL_INPUT_SEQUENCE][note]) (void)runtime_note(p, (unsigned)note, 0);
}
/* heldNoteNumbers: remove the pitch, then (on) insert it at the front. */
__attribute__((noinline)) static void arp_hold(original_runtime_part *p, unsigned key, unsigned velocity)
{
    original_runtime_arp *a = &p->arp;
    unsigned n = 0;
    for (unsigned i = 0; i < a->nheld; ++i) if (a->held[i].noteNumber != key) a->held[n++] = a->held[i];
    if (velocity) {
        if (n == S1_SEQ_MAX_HELD) --n;
        for (unsigned i = n; i; --i) a->held[i] = a->held[i - 1];   /* no memmove in the firmware libc */
        a->held[0].noteNumber = (uint8_t)key; a->held[0].velocity = (uint8_t)velocity; ++n;
    }
    a->nheld = (uint8_t)n;
}
/* engine_arp_cfg -> the S1Parameter values the sequencer reads. */
static const uint8_t arp_ticks[ARP_NSPEED] = {48, 36, 32, 24, 18, 16, 12, 8, 6, 3};   /* 24 PPQN */
uint32_t engine_arp_step_frames(float bpm, unsigned speed)
{
    const uint8_t *ticks = arp_ticks;
    bpm = bpm < 20.f ? (bpm > 0.f ? 20.f : 120.f) : bpm > 275.f ? 275.f : bpm;
    float f = 44100.f * 60.f * ticks[speed < ARP_NSPEED ? speed : ARP_SPEED_1_16] / (bpm * 24.f);
    return f < 1.f ? 1u : (uint32_t)(f + .5f);
}
static void arp_params(original_runtime_part *p)
{
    static const uint8_t dir[3] = {ArpeggiatorMode_Up, ArpeggiatorMode_Down, ArpeggiatorMode_UpDown};
    const engine_arp_cfg *c = &p->arp.cfg;
    S1SequencerParams *q = &p->arp.prm;
    q->arpIsOn = c->on;
    q->arpIsSequencer = c->mode == ARP_SEQ;
    q->arpDirection = dir[c->mode < 3 ? c->mode : 0];
    q->arpOctave = (uint8_t)(c->range ? c->range - 1 : 0);
    q->arpInterval = c->interval;
    q->arpTotalSteps = c->length ? c->length : 16;
    __builtin_memcpy(q->sequencerPattern, c->offset, sizeof q->sequencerPattern);
    q->sequencerOctBoost = c->oct_mask;
    q->sequencerNoteOn = c->on_mask;
    /* arpRate = project tempo; arpSeqTempoMultiplier = SPEED (exact frames). */
    q->arpBeatsPerSample = S1_SAMPLE_TICKS;
    q->seqGate = c->gate ? c->gate : 100;
    S1Sequencer_setStep(q, S1_SAMPLE_TICKS * engine_arp_step_frames(p->arp.bpm, c->speed));
    q->patternRange = p->kind == ORIGINAL_SOUND_SYNTHONE ? 12 : 24;
    q->clocked = 0;
}
void original_runtime_arp_clock(original_runtime_part *p, uint64_t pos_q8, uint32_t beat_q8, int start)
{
    if (!p) return;
    p->arp.clk_pos = pos_q8; p->arp.clk_beat = beat_q8;
    if (start) p->arp.clk_force = (uint8_t)arp_engaged(p);
}
/* Lock the sequencer to the transport: grid = beat * SPEED ticks / 24. */
static void arp_follow_clock(original_runtime_arp *a)
{
    const unsigned sp = a->cfg.speed < ARP_NSPEED ? a->cfg.speed : ARP_SPEED_1_16;
    uint32_t len = (uint32_t)((uint64_t)a->clk_beat * arp_ticks[sp] / 24u);
    if (len < 2u * S1_SAMPLE_TICKS) len = 2u * S1_SAMPLE_TICKS;
    a->prm.arpBeatsPerSample = S1_SAMPLE_TICKS;
    if (len != a->prm.arpSeqTempoMultiplier || !a->prm.clocked) S1Sequencer_setStep(&a->prm, len);
    a->prm.clocked = 1;
    /* position before this sample's increment; step -1 just before the first */
    int step; uint32_t phase;
    if (a->clk_pos >= S1_SAMPLE_TICKS) {
        uint64_t v = a->clk_pos - S1_SAMPLE_TICKS;
        step = (int)(v / len); phase = (uint32_t)v - (uint32_t)step * len;
    } else {
        step = -1; phase = len - (uint32_t)(S1_SAMPLE_TICKS - a->clk_pos);
    }
    S1Sequencer_clock(&a->seq, &a->prm, step, phase, a->clk_force);
    a->clk_force = 0;
}
int original_runtime_arp_config(original_runtime_part *p, const engine_arp_cfg *c)
{
    if (!p || !c) return 0;
    int ok = p->status == ORIGINAL_RUNTIME_OK, was = ok && arp_engaged(p);
    original_runtime_arp *a = &p->arp;
    if (was) S1Sequencer_reset(&a->seq, 0, p, arp_key_off);
    a->cfg = *c;
    if (!ok) return 0;
    arp_params(p);
    int now = arp_engaged(p);
    if (now && !was) a->nheld = 0;
    if (now != was) {                  /* direct live notes <-> the arp */
        for (unsigned key = 0; key < 128; ++key)
            if (live_held(p, key)) {
                if (now) {
                    if (!p->keys[ORIGINAL_INPUT_SEQUENCE][key]) (void)runtime_note(p, key, 0);
                    arp_hold(p, key, a->vel ? a->vel : 100u);
                } else (void)runtime_note(p, key, a->vel ? a->vel : 100u);   /* keys still down sound again */
            }
        if (!now) a->nheld = 0;
        S1Sequencer_init(&a->seq);
        a->fired = 0;
    }
    return 1;
}
int original_runtime_arp_view(const original_runtime_part *p, unsigned *step, uint32_t *frames)
{
    if (!p || p->status != ORIGINAL_RUNTIME_OK || !arp_engaged(p) || !p->arp.nheld || !p->arp.fired) return 0;
    *step = (unsigned)p->arp.seq.lastPosition % ARP_STEPS;
    *frames = p->arp.age;
    return 1;
}
/* A live or sequenced key's count already went down: the arp, or the voice. */
static int live_off(original_runtime_part *p, unsigned source, unsigned key)
{
    if (live_source(source) && arp_engaged(p)) {
        if (!live_held(p, key)) arp_hold(p, key, 0);
        return ORIGINAL_RUNTIME_OK;
    }
    return key_held(p, key) ? ORIGINAL_RUNTIME_OK : runtime_note(p, key, 0);
}
#else
static int live_off(original_runtime_part *p, unsigned source, unsigned key)
{
    (void)source;
    return key_held(p, key) ? ORIGINAL_RUNTIME_OK : runtime_note(p, key, 0);
}
#endif

/* Same-source, same-pitch MIDI offs carry no note identity. For a cancelled
 * press followed by a retrigger, keep the first ambiguous off pending for at
 * most 16 x 256 rendered frames (about 93 ms at 44.1 kHz). Another off within
 * that window releases the live press. If it never arrives, release the live
 * press at expiry instead of leaving a stuck note. An unusually late OLD off
 * cannot be distinguished from an early NEW off: expiry favors recovery. */
#define ORIGINAL_STALE_GRACE_TICKS 16u
static int runtime_stale_tick(original_runtime_part *p, unsigned frames)
{
    if (!p->stale_timers) return ORIGINAL_RUNTIME_OK;
    unsigned elapsed = (unsigned)p->stale_tick_frames + frames;
    unsigned ticks = elapsed / 256u;
    p->stale_tick_frames = (uint16_t)(elapsed % 256u);
    if (!ticks) return ORIGINAL_RUNTIME_OK;
    for (unsigned source = 0; source < ORIGINAL_INPUT_KEYS; ++source) {
        for (unsigned key = 0; key < 128; ++key) {
            uint8_t *ttl = &p->stale_ttl[source][key];
            if (!*ttl) continue;
            if (*ttl > ticks) { *ttl -= (uint8_t)ticks; continue; }
            *ttl = 0;
            --p->stale_timers;
            p->stale[source][key] = 0;
            unsigned lane = key >> 5;
            uint32_t mask = UINT32_C(1) << (key & 31u);
            if (p->stale_pending[source][lane] & mask) {
                p->stale_pending[source][lane] &= ~mask;
                if (p->keys[source][key]) {
                    --p->keys[source][key];
                    if (live_off(p, source, key) != ORIGINAL_RUNTIME_OK) return p->status;
                }
            }
        }
    }
    return ORIGINAL_RUNTIME_OK;
}

int original_runtime_key(original_runtime_part *p, unsigned source, unsigned key, unsigned velocity)
{
    if (!p || source >= ORIGINAL_INPUT_COUNT || key > 127 || velocity > 127) return 0;
    if (p->status != ORIGINAL_RUNTIME_OK) return p->status;
    uint8_t *held = &p->keys[source][key];
    if (velocity) {
        /* Saturation is an explicit fault: never wrap a count and lose an off. */
        if (*held == UINT8_MAX) return runtime_result(p, 0);
        if (source < ORIGINAL_INPUT_KEYS && p->stale[source][key] && !p->stale_ttl[source][key]) {
            /* Start the window on the fresh press, never on the REC cancel. */
            if (!p->stale_timers) p->stale_tick_frames = 0;
            p->stale_ttl[source][key] = ORIGINAL_STALE_GRACE_TICKS;
            ++p->stale_timers;
        }
        ++*held;
#if X0X_ARP_ENGINE
        if (live_source(source) && arp_engaged(p)) {
            p->arp.vel = (uint8_t)velocity;
            arp_hold(p, key, velocity ? velocity : 1u);
            return ORIGINAL_RUNTIME_OK;
        }
#endif
        return runtime_note(p, key, velocity);
    }
    if (source < ORIGINAL_INPUT_KEYS && p->stale[source][key]) {
        --p->stale[source][key];
        if (*held) p->stale_pending[source][key >> 5] |= UINT32_C(1) << (key & 31u);
        return ORIGINAL_RUNTIME_OK;
    }
    if (!*held) return ORIGINAL_RUNTIME_OK;
    --*held;
    if (source < ORIGINAL_INPUT_KEYS)
        p->stale_pending[source][key >> 5] &= ~(UINT32_C(1) << (key & 31u));
    return live_off(p, source, key);
}

int original_runtime_release_source(original_runtime_part *p, unsigned source)
{
    if (!p || source >= ORIGINAL_INPUT_COUNT) return 0;
    if (p->status != ORIGINAL_RUNTIME_OK) return p->status;
    if (source < ORIGINAL_INPUT_KEYS) {
        for (unsigned key = 0; key < 128; ++key)
            if (p->stale_ttl[source][key]) --p->stale_timers;
        memset(p->stale[source], 0, sizeof p->stale[source]);
        memset(p->stale_ttl[source], 0, sizeof p->stale_ttl[source]);
        memset(p->stale_pending[source], 0, sizeof p->stale_pending[source]);
    }
    for (unsigned key = 0; key < 128; ++key) if (p->keys[source][key]) {
        p->keys[source][key] = 0;
        if (live_off(p, source, key) != ORIGINAL_RUNTIME_OK) return p->status;
    }
    return ORIGINAL_RUNTIME_OK;
}

/* End the voice of every key this external source holds. Its pending offs
 * are kept as stale counts, so the off of a cut press cannot end a newer
 * press of the same pitch. */
int original_runtime_cut_source(original_runtime_part *p, unsigned source)
{
    if (!p || source >= ORIGINAL_INPUT_COUNT) return 0;
    if (p->status != ORIGINAL_RUNTIME_OK) return p->status;
    for (unsigned key = 0; key < 128; ++key) if (p->keys[source][key]) {
        if (source < ORIGINAL_INPUT_KEYS) {
            unsigned owed = (unsigned)p->stale[source][key] + p->keys[source][key];
            p->stale[source][key] = (uint8_t)(owed > UINT8_MAX ? UINT8_MAX : owed);
            if (p->stale_ttl[source][key]) --p->stale_timers;
            p->stale_ttl[source][key] = 0;
            p->stale_pending[source][key >> 5] &= ~(UINT32_C(1) << (key & 31u));
        }
        p->keys[source][key] = 0;
        if (live_off(p, source, key) != ORIGINAL_RUNTIME_OK) return p->status;
    }
    return ORIGINAL_RUNTIME_OK;
}

int original_runtime_all_off(original_runtime_part *p)
{
    if (!p || p->status != ORIGINAL_RUNTIME_OK) return p ? p->status : 0;
    int ok;
    if(p->kind==ORIGINAL_SOUND_JUNO){
        juno6_all_off(&p->dsp.juno.synth);
        memset(p->dsp.juno.chorus.delay,0,sizeof p->dsp.juno.chorus.delay);
        p->dsp.juno.chorus.write=0;ok=1;
    } else if (p->kind == ORIGINAL_SOUND_SYNTHONE) {
        s1_part_all_off(&p->dsp.synthone);
        ok = 1;
    } else if (p->kind == ORIGINAL_SOUND_MIAMI) {
        ok = original_miami_backend_all_off(&p->dsp.miami);
    } else {
        ok = original_chicago_backend_all_off(&p->dsp.chicago) == ORIGINAL_CHICAGO_BACKEND_OK;
    }
    memset(p->keys, 0, sizeof p->keys);
    memset(p->stale, 0, sizeof p->stale);
    memset(p->stale_ttl, 0, sizeof p->stale_ttl);
    memset(p->stale_pending, 0, sizeof p->stale_pending);
    p->stale_tick_frames = p->stale_timers = 0;
#if X0X_ARP_ENGINE
    p->arp.nheld = 0; S1Sequencer_init(&p->arp.seq);
#endif
    return runtime_result(p, ok);
}

int original_runtime_owned(const original_runtime_part *p)
{
    if (!p || p->status != ORIGINAL_RUNTIME_OK) return 0;
    return p->kind == ORIGINAL_SOUND_JUNO ? juno6_active(&p->dsp.juno.synth) : p->kind == ORIGINAL_SOUND_SYNTHONE ? s1_part_active(&p->dsp.synthone) :
        p->kind == ORIGINAL_SOUND_MIAMI ? original_miami_backend_voice_active(&p->dsp.miami) :
        original_chicago_backend_active(&p->dsp.chicago);
}

int original_runtime_filter_view(const original_runtime_part *p, float *eg, float *depth)
{
    if (!p || p->status != ORIGINAL_RUNTIME_OK || p->kind != ORIGINAL_SOUND_CHICAGO) return 0;
    return original_chicago_backend_filter_view(&p->dsp.chicago, eg, depth);
}

X0X_COLD int original_runtime_voice_view(const original_runtime_part *p, float *level, float *modulation,
    unsigned *state, unsigned *key_down)
{
    if (!p || p->status != ORIGINAL_RUNTIME_OK || !level || !modulation || !state || !key_down) return 0;
    *level=0; *modulation=0; *state=6; *key_down=0;
    if (p->kind == ORIGINAL_SOUND_CHICAGO) {
        const original_chicago_voice *v=&p->dsp.chicago.voice;
        if (v->active) { *level=v->vca.level; *modulation=v->mg[0].unipolar;
            *state=v->vca.state; *key_down=v->key_down; }
    } else if (p->kind == ORIGINAL_SOUND_MIAMI) {
        int selected=-1;
        for (unsigned i=0;i<ORIGINAL_MIAMI_BACKEND_VOICES;i++) {
            const original_miami_selected_voice *v=&p->dsp.miami.voices[i];
            if (v->active && (selected<0 || v->vca.level>*level)) {
                selected=(int)i; *level=v->vca.level; *modulation=v->mg[0].unipolar;
                *state=v->vca.state; *key_down=v->key_down;
            }
        }
    } else if(p->kind==ORIGINAL_SOUND_JUNO){
#if X0X_SUPERJ6_FACTORY
        juno6_envelope_snapshot view;
        if(juno6_read_envelope(&p->dsp.juno.synth,&view)&&view.active&&
           (p->dsp.juno.synth.typed || view.level>0.f)){
            *level=view.level;*state=view.stage==1?0u:view.stage==4?3u:1u;*key_down=view.gate;
        }
#else
        for(unsigned i=0;i<JUNO6_VOICES;i++){
            const juno6_voice_t *v=&p->dsp.juno.synth.voice[i];
            if(v->stage && v->envelope>*level){*level=v->envelope;*state=v->stage==1?0u:v->stage==4?3u:1u;*key_down=v->gate;}
        }
#endif
    } else if (p->kind == ORIGINAL_SOUND_SYNTHONE) {
#if X0X_S1C
        (void)s1_part_voice_view(&p->dsp.synthone, level, modulation, state, key_down);
#else
        const s1_dry_part *part = &p->dsp.synthone;
        int selected = -1;
        for (unsigned i = 0; i < S1_CORE_VOICES; ++i) {
            if (part->voice[i].started &&
                (selected < 0 || part->voice[i].amp.level > part->voice[selected].amp.level))
                selected = (int)i;
        }
        if (selected >= 0) {
            const s1_dry_voice *voice = &part->voice[selected];
            *level = voice->amp.level;
            *modulation = voice->filter.level;
            *state = voice->amp.stage == DNBX_S1_ENVELOPE_ATTACK ? 0u :
                     voice->amp.stage == DNBX_S1_ENVELOPE_RELEASE ? 3u : 1u;
            *key_down = part->down[selected];
        }
#endif
    } else return 0;
    return 1;
}

X0X_COLD int original_runtime_adsr_view(const original_runtime_part *p, float values[4], unsigned *gate)
{
    if (!p || p->status != ORIGINAL_RUNTIME_OK || p->kind != ORIGINAL_SOUND_CHICAGO || !values || !gate) return 0;
    for (unsigned i=0;i<4;i++) values[i]=p->dsp.chicago.controls.vca[i];
    *gate=p->dsp.chicago.controls.routing.vca_eg_mode;
    return 1;
}

X0X_COLD int original_runtime_motion_view(const original_runtime_part *p, float *phase, float *bipolar,
    float *current, float *target, int *active)
{
    if (!p || p->status != ORIGINAL_RUNTIME_OK) return 0;
    if (p->kind == ORIGINAL_SOUND_JUNO || p->kind == ORIGINAL_SOUND_SYNTHONE) return 0;
    if (p->kind == ORIGINAL_SOUND_MIAMI)
        return original_miami_backend_motion_view(&p->dsp.miami, phase, bipolar, current, target, active);
    *phase = *bipolar = 0.0f;
    return original_chicago_backend_pitch_view(&p->dsp.chicago, current, target, active);
}

int original_runtime_env_view(const original_runtime_part *p, float adsr[4], float *level, int *state)
{
    if (!p || p->status != ORIGINAL_RUNTIME_OK || p->kind != ORIGINAL_SOUND_CHICAGO) return 0;
    return original_chicago_backend_env_view(&p->dsp.chicago, adsr, level, state);
}

int original_runtime_active(const original_runtime_part *p)
{
    if (!p || p->status != ORIGINAL_RUNTIME_OK) return 0;
    return p->kind == ORIGINAL_SOUND_JUNO ? juno6_active(&p->dsp.juno.synth) : p->kind == ORIGINAL_SOUND_SYNTHONE ? s1_part_active(&p->dsp.synthone) :
        p->kind == ORIGINAL_SOUND_MIAMI ? original_miami_backend_active(&p->dsp.miami) :
        original_chicago_backend_active(&p->dsp.chicago);
}

static int render_backend(original_runtime_part *, original_runtime_workspace *, float *, float *, unsigned);
#if DNBOX_LAB_ARP_EVENTLESS
/* These parameters and held notes are span-owned. No sample may fire or
 * execute an early-gate write; unsigned wrap is deliberately a miss. */
static int arp_eventless_advance(S1Sequencer *s, const S1SequencerParams *p, int held, uint32_t *age, unsigned n)
{
    if (!n) return 1;
    const int on = p->arpIsOn == 1;
    const int prev = s->previousHeldNoteNumbersAECount;
    if ((prev == 0 && held > 0) || 
        (held == 0 && prev > 0)) return 0;
    if (on || s->sequencerLastNotesCount) {
        if (n > 256 || p->arpBeatsPerSample != S1_SAMPLE_TICKS) return 0;
        const uint32_t add = n * S1_SAMPLE_TICKS;
        const uint32_t end = s->mBeatTime + add;
        if (end < s->mBeatTime || end >= p->arpSeqTempoMultiplier ||
            (s->gateOpen && end >= p->gateTicks)) return 0;
        s->mBeatTime = end;
    }
    s->previousHeldNoteNumbersAECount = held;
    *age = UINT32_MAX - *age < n ? UINT32_MAX : *age + n;
    return 1;
}
#endif

int original_runtime_render(original_runtime_part *p, original_runtime_workspace *w,
    float *left, float *right, unsigned frames)
{
    if (!p || !w || !left || !right || frames > 256) return 0;
    if (p->status != ORIGINAL_RUNTIME_OK) return p->status;
    if (runtime_stale_tick(p, frames) != ORIGINAL_RUNTIME_OK) return p->status;
#if X0X_ARP_ENGINE
    if (arp_engaged(p) || p->arp.seq.sequencerLastNotesCount) {
        /* S1Sequencer::process runs once per sample, as in S1DSPKernel; the
         * block is split where it turns notes on or off. */
        original_runtime_arp *a = &p->arp;
        unsigned done = 0;
        if (a->clk_beat) arp_follow_clock(a);
#if DNBOX_LAB_ARP_EVENTLESS
        if (frames && arp_eventless_advance(&a->seq, &a->prm, a->nheld, &a->age, frames))
            return render_backend(p, w, left, right, frames);
#endif
        for (unsigned i = 0; i < frames; ++i) {
            if (S1Sequencer_willFire(&a->seq, &a->prm, a->nheld)) {
                if (i > done && render_backend(p, w, left + done, right + done, i - done) != ORIGINAL_RUNTIME_OK)
                    return p->status;
                done = i;
                int before = a->seq.mStepCounter, first = !a->seq.previousHeldNoteNumbersAECount && a->nheld;
                S1Sequencer_process(&a->seq, &a->prm, a->held, a->nheld, p, arp_key_on, arp_key_off);
                if (p->status != ORIGINAL_RUNTIME_OK) return p->status;
                if (a->seq.gateOpen && (first || a->seq.mStepCounter != before)) { a->fired = 1; a->age = 0; }
            } else S1Sequencer_process(&a->seq, &a->prm, a->held, a->nheld, p, arp_key_on, arp_key_off);
            a->age += a->age < UINT32_MAX;
        }
        return frames > done ? render_backend(p, w, left + done, right + done, frames - done) : p->status;
    }
#endif
    return render_backend(p, w, left, right, frames);
}

static int render_backend(original_runtime_part *p, original_runtime_workspace *w,
    float *left, float *right, unsigned frames)
{
    int ok;
    if(p->kind==ORIGINAL_SOUND_JUNO){
#if X0X_SUPERJ6_FACTORY
        ok=juno6_render_stereo(&p->dsp.juno.synth,left,right,frames);
        if(ok){
            juno6_chorus_process(&p->dsp.juno.chorus,left,right,frames);
#if X0X_SJ6_NATIVE_DELAY && X0X_SUPERJ6_FACTORY
            juno6_delay24_process(&p->dsp.juno.synth,left,right,frames);
#endif
            /* The original master attenuator follows the instrument effects. */
            float gain=p->dsp.juno.synth.master_gain;
            if(gain!=1.f) for(unsigned i=0;i<frames;++i){
                left[i]*=gain;
                right[i]*=gain;
            }
        }
#else
        juno6_render(&p->dsp.juno.synth,left,frames);
        memcpy(right,left,frames*sizeof *right);
        juno6_chorus_process(&p->dsp.juno.chorus,left,right,frames);ok=1;
#endif
    } else if (p->kind == ORIGINAL_SOUND_SYNTHONE) {
        s1_part_render(&p->dsp.synthone,left,right,frames);
        ok = 1;
    } else if (p->kind == ORIGINAL_SOUND_MIAMI) {
        original_miami_backend_workspace workspace = {w->miami.left, w->miami.right,
            {w->miami.dry[0], w->miami.dry[1], w->miami.scratch[0], w->miami.scratch[1], 256}};
        ok = original_miami_backend_render_stereo(&p->dsp.miami, &workspace, left, right, frames);
    } else {
        original_chicago_backend_workspace workspace = {w->chicago.left, w->chicago.right,
            w->chicago.converted[0], w->chicago.converted[1],
            {w->chicago.scratch[0], w->chicago.scratch[1], w->chicago.scratch[2], w->chicago.scratch[3]}, 256};
        ok = original_chicago_backend_render(&p->dsp.chicago, &workspace, NULL, 0,
            left, right, frames) == ORIGINAL_CHICAGO_BACKEND_OK;
    }
    return runtime_result(p, ok);
}

