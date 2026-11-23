/* SPDX-License-Identifier: GPL-3.0-only */
/* Textually included by engine.c after original_presets.inc: arp/ARP SEQ hooks
 * (engine_arp.h) and the persistent per-voice block at sound.v[T_303][2 + v].
 * Main loop: read = short locked copy; write = edit one pending copy (newest
 * wins). Audio boundary (arp_boundary): apply, store, bump the revision. */
#if X0X_ARP_HOOKS
#define ARP_SLOT(s, v) ((s)->v[T_303][ARP_BLOCK_VOICE0 + (unsigned)(v)])
static volatile uint32_t arp_revision[NBASS];
static engine_arp_cfg arp_pending[NBASS];
static volatile uint8_t arp_pending_flag[NBASS];

static int arp_kind_ok(unsigned v)
{
    const original_runtime_part *p = &original_bass[v];
    return p->status == ORIGINAL_RUNTIME_OK &&
           (p->kind == ORIGINAL_SOUND_JUNO || p->kind == ORIGINAL_SOUND_SYNTHONE);
}
static void arp_store(unsigned v, const engine_arp_cfg *c)
{
    if (mot_base_snd) engine_arp_block_encode(c, ARP_SLOT((sound_t *)mot_base_snd, v));
}
/* Audio side or held engine: a new owner adopts the stored block. A preset
 * change keeps the arp but drops stored AMP2 (the new program owns it). */
static void arp_adopt(int v, const sound_t *sound, int preset_change)
{
    engine_arp_cfg c;
    (void)engine_arp_block_decode(sound ? ARP_SLOT(sound, v) : NULL, &c);
    if (preset_change && c.amp2_mask) { c.amp2_mask = 0; arp_store((unsigned)v, &c); }
    arp_pending_flag[v] = 0;
    (void)original_runtime_arp_config(&original_bass[v], &c);
#if X0X_SUPERJ6_FACTORY
    original_runtime_part *part = &original_bass[v];
    if (c.amp2_mask && part->kind == ORIGINAL_SOUND_JUNO && part->status == ORIGINAL_RUNTIME_OK)
        for (unsigned s = 0; s < 4; ++s) if (c.amp2_mask & (1u << s)) {
            part->dsp.juno.synth.pending_field[s] = c.amp2[s] * (1.f / 65535.f);
            part->dsp.juno.synth.pending_field_mask |= (uint8_t)(1u << s);
        }
#endif
    ++arp_revision[v];
}
#if X0X_SUPERJ6_FACTORY
static void arp_store_amp2(unsigned v, unsigned slot, float unit)
{
    if (!mot_base_snd || slot >= 4) return;
    engine_arp_cfg c = original_bass[v].arp.cfg;
    c.amp2_mask |= (uint8_t)(1u << slot);
    c.amp2[slot] = (uint16_t)(unit * 65535.f + .5f);
    original_bass[v].arp.cfg = c;
    arp_store(v, &c);
}
#endif
static void arp_boundary(void)
{
    for (unsigned v = 0; v < NBASS; ++v) {
        if (!arp_pending_flag[v]) continue;
        arp_pending_flag[v] = 0;
        if (!arp_kind_ok(v)) continue;
        engine_arp_cfg c = arp_pending[v];
        c.amp2_mask = original_bass[v].arp.cfg.amp2_mask;          /* AMP2 is not an arp edit */
        __builtin_memcpy(c.amp2, original_bass[v].arp.cfg.amp2, sizeof c.amp2);
        (void)original_runtime_arp_config(&original_bass[v], &c);
        arp_store(v, &c);
        ++arp_revision[v];
    }
}
/* Locked: 0 no owner, 1 ready. *c is the newest (pending or applied) block. */
static int arp_snapshot(unsigned v, engine_arp_cfg *c, uint32_t *rev)
{
    if (v >= NBASS) return 0;
    uint32_t lock = plat_cq_lock();
    int ok = arp_kind_ok(v);
    if (ok) {
        *c = arp_pending_flag[v] ? arp_pending[v] : original_bass[v].arp.cfg;
        *rev = arp_revision[v];
    }
    plat_cq_unlock(lock);
    return ok;
}
int engine_arp_settings_read(unsigned v, engine_arp_settings *out)
{
    engine_arp_cfg c; uint32_t rev;
    if (!out || !arp_snapshot(v, &c, &rev)) return 0;
    *out = (engine_arp_settings){rev, c.on, c.mode, c.range, c.speed, c.length, c.interval, 0, c.gate};
    return 1;
}
int engine_arp_steps_read(unsigned v, engine_arp_steps *out)
{
    engine_arp_cfg c; uint32_t rev;
    if (!out || !arp_snapshot(v, &c, &rev)) return 0;
    out->revision = rev;
    __builtin_memcpy(out->offset, c.offset, sizeof out->offset);
    out->on_mask = c.on_mask; out->oct_mask = c.oct_mask;
    return 1;
}
/* field < 16: step field (step << 4 | ARP_S_*) + 16; else settings field. */
static int arp_edit(unsigned v, uint32_t revision, unsigned step, unsigned field, int value)
{
    static const uint8_t lo[ARP_NF] = {0, 0, 1, 0, 1, 0, 1}, hi[ARP_NF] = {1, ARP_NMODES - 1, 4, 9, 16, 12, 100};
    if (v >= NBASS) return S1_EDIT_REJECTED;
    if (step == 0xFFu ? (field >= ARP_NF || value < lo[field] || value > hi[field])
                      : (step >= ARP_STEPS || field > ARP_S_OCT ||
                         (field == ARP_S_OFFSET ? (value < -24 || value > 24) : (value < 0 || value > 1))))
        return S1_EDIT_REJECTED;
    uint32_t lock = plat_cq_lock();
    int status = S1_EDIT_REJECTED;
    uint32_t request = bass_request[v];
    if (arp_kind_ok(v) && revision == arp_revision[v]) {
        if (project_hold || ((request & 0x80000000u) && request != bass_applied[v])) status = S1_EDIT_BUSY;
        else {
            engine_arp_cfg *c = &arp_pending[v];
            if (!arp_pending_flag[v]) *c = original_bass[v].arp.cfg;
            if (step != 0xFFu) {
                uint16_t bit = (uint16_t)(1u << step);
                int lim = original_bass[v].kind == ORIGINAL_SOUND_SYNTHONE ? 12 : 24;
                if (field == ARP_S_OFFSET) c->offset[step] = (int8_t)(value < -lim ? -lim : value > lim ? lim : value);
                else if (field == ARP_S_ON) c->on_mask = (uint16_t)(value ? c->on_mask | bit : c->on_mask & ~bit);
                else c->oct_mask = (uint16_t)(value ? c->oct_mask | bit : c->oct_mask & ~bit);
            } else {
                uint8_t *f[ARP_NF] = {&c->on, &c->mode, &c->range, &c->speed, &c->length, &c->interval, &c->gate};
                *f[field] = (uint8_t)value;
            }
            arp_pending_flag[v] = 1;
            status = S1_EDIT_QUEUED;
        }
    }
    plat_cq_unlock(lock);
    return status;
}
int engine_arp_set(unsigned v, uint32_t revision, unsigned field, int value)
{ return arp_edit(v, revision, 0xFFu, field, value); }
int engine_arp_step_set(unsigned v, uint32_t revision, unsigned step, unsigned field, int value)
{ return step == 0xFFu ? S1_EDIT_REJECTED : arp_edit(v, revision, step, field, value); }
int engine_arp_view(unsigned v, unsigned *step, uint32_t *fired_age_ms)
{
    if (v >= NBASS || !step || !fired_age_ms) return 0;
    uint32_t frames;
    unsigned s;
    if (!original_runtime_arp_view(&original_bass[v], &s, &frames)) return 0;
    *step = s;
    *fired_age_ms = frames / 44u;    /* 44.1 kHz; display only */
    return 1;
}
static int arp_queue(unsigned v, const engine_arp_cfg *c)
{
    if (v >= NBASS) return S1_EDIT_REJECTED;
    uint32_t lock = plat_cq_lock();
    int status = S1_EDIT_REJECTED;
    if (arp_kind_ok(v)) {
        if (project_hold) status = S1_EDIT_BUSY;
        else { arp_pending[v] = *c; arp_pending_flag[v] = 1; status = S1_EDIT_QUEUED; }
    }
    plat_cq_unlock(lock);
    return status;
}
int engine_arp_reload(unsigned v)
{
    engine_arp_cfg c;
    if (v >= NBASS || !mot_base_snd) return S1_EDIT_REJECTED;
    (void)engine_arp_block_decode(ARP_SLOT(mot_base_snd, v), &c);
    return arp_queue(v, &c);
}
int engine_arp_preset(unsigned v, const engine_arp_cfg *c)
{
    if (!c) return S1_EDIT_REJECTED;
    engine_arp_cfg n;
    uint8_t b[ARP_BLOCK_BYTES];
    engine_arp_block_encode(c, b);                 /* normalise domains through the codec */
    (void)engine_arp_block_decode(b, &n);
    return arp_queue(v, &n);
}
#else
static void arp_adopt(int v, const sound_t *sound, int preset_change) { (void)v; (void)sound; (void)preset_change; }
static void arp_boundary(void) {}
#if X0X_SUPERJ6_FACTORY
static void arp_store_amp2(unsigned owner, unsigned slot, float unit) { (void)owner; (void)slot; (void)unit; }
#endif
#endif
