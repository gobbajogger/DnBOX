/* SPDX-License-Identifier: GPL-3.0-only */
/* Textually included by engine.c: replacement-only patch publication. */
#if X0X_S1_EXTRA
#include "../dsp/s1c/s1c_front.h"
#endif
#if defined(X0X_S1_BEND_RANGE_PROBE) && X0X_S1_BEND_RANGE_PROBE
#include "sj6_137_host_witness.h"
extern int s1_typed_tuple_editable(void);
#endif
static int16_t bass_named_preset[NBASS] = {-1, -1};
static void arp_adopt(int v, const sound_t *sound, int preset_change); /* engine_arp.inc */
static volatile uint8_t bass_preview_dirty, bass_wheel_mask, bass_edit_depth;
#if X0X_SYNTHONE_PATCH || X0X_BASS_NOTE_SOUND
static volatile uint8_t note_lock_applied[NBASS];
#endif
#if X0X_SYNTHONE_PATCH
/* One main-loop edit per part. A new adoption invalidates its predecessor,
 * even on a project reload or a new choice of the same named preset. */
static volatile uint32_t s1_patch_revision[NBASS];
static uint32_t s1_patch_identity(unsigned voice)
{ return (s1_patch_revision[voice] << 1) | voice; }

static volatile struct { uint8_t pending, slot, position; } s1_edit[NBASS];
int engine_s1_patch_pending(void)
{ return s1_edit[0].pending || s1_edit[1].pending; }

static void s1_patch_replaced(int voice)
{
    ++s1_patch_revision[voice];
    if (original_bass[voice].kind == ORIGINAL_SOUND_SYNTHONE) note_lock_applied[voice] = 0;
    s1_edit[voice].pending = 0;
}
#endif

static volatile uint32_t bass_request[NBASS], bass_applied[NBASS];
static uint32_t bass_request_serial;
static volatile uint32_t bass_publish_seq[NBASS], bass_midi_mark[NBASS], bass_cq_mark[NBASS];
typedef struct { uint32_t token, midi, cq; } bass_request_t;
/* Per-pattern preset recall (owner 2026-11-12 02:42): a request may wait for
 * its pattern. 0xFF = no gate (every ordinary preset choice). */
static volatile uint8_t bass_gate_pat[NBASS] = {0xFF, 0xFF};
static void bass_preset_boundary_voice(int v);
static const bass_identity_entry_t BASS_SOUND_REGISTRY[4] = {
    {ORIGINAL_SOUND_CHICAGO_ID, ORIGINAL_SOUND_SCHEMA, 22, 1},
    {ORIGINAL_SOUND_MIAMI_ID, ORIGINAL_SOUND_SCHEMA, 18, 1},
    {ORIGINAL_SOUND_SYNTHONE_ID, ORIGINAL_SOUND_SCHEMA, ORIGINAL_SYNTHONE_NPARAMS, 1},
    {ORIGINAL_SOUND_JUNO_ID, ORIGINAL_SOUND_SCHEMA, ORIGINAL_JUNO_NPARAMS, 1}
};

static int bass_request_snapshot(int voice, bass_request_t *r)
{
    uint32_t serial = bass_publish_seq[voice];
    if (serial & 1u) return 0;
    __asm__ volatile("" ::: "memory");
    r->token = bass_request[voice]; r->midi = bass_midi_mark[voice]; r->cq = bass_cq_mark[voice];
    __asm__ volatile("" ::: "memory");
    return serial == bass_publish_seq[voice];
}
static void bass_publish_request(int voice, uint32_t token)
{
    ++bass_publish_seq[voice];
    __asm__ volatile("" ::: "memory");
    bass_midi_mark[voice] = plat_midi_mark(); bass_cq_mark[voice] = cq_w;
    bass_request[voice] = token;
#if X0X_SUPERJ6_BUILTIN_INIT
    sj6_knob_clear((unsigned)voice);
#endif
    __asm__ volatile("" ::: "memory");
    ++bass_publish_seq[voice];
}
#if X0X_SYNTHONE_PATCH
X0X_COLD int engine_s1_patch_snapshot(unsigned voice, s1_patch_snapshot *out)
{
    if (voice >= NBASS || !out) return 0;
    uint32_t lock = plat_cq_lock();
    const original_runtime_part *owner = &original_bass[voice];
    int ready = owner->kind == ORIGINAL_SOUND_SYNTHONE && owner->status == ORIGINAL_RUNTIME_OK;
    if (ready) {
        out->revision = s1_patch_identity(voice);
        out->note_locked = note_lock_applied[voice];
        out->preset = bass_named_preset[voice];
        out->filter_link = (uint8_t)s1_part_patch_value(&owner->dsp.synthone, S1_PATCH_COUNT);
        for (unsigned i = 0; i < S1_PATCH_COUNT; ++i)
            out->physical[i] = s1_part_patch_value(&owner->dsp.synthone, i);
    }
    plat_cq_unlock(lock);
    return ready;
}

int engine_s1_envelope_view(unsigned voice, unsigned filter, float *level, unsigned *stage, unsigned *key_down)
{
    if (voice >= NBASS || !level || !stage || !key_down) return 0;
    uint32_t lock = plat_cq_lock();
    const original_runtime_part *owner = &original_bass[voice];
    int selected = -1;
#if X0X_S1C
    if (owner->kind == ORIGINAL_SOUND_SYNTHONE && owner->status == ORIGINAL_RUNTIME_OK)
        selected = s1_part_env_view(&owner->dsp.synthone, filter, level, stage, key_down) ? 0 : -1;
#else
    if (owner->kind == ORIGINAL_SOUND_SYNTHONE && owner->status == ORIGINAL_RUNTIME_OK) {
        const s1_dry_part *part = &owner->dsp.synthone;
        /* Same voice as engine_bass_voice_view: the loudest started one. */
        for (unsigned i = 0; i < S1_CORE_VOICES; ++i)
            if (part->voice[i].started &&
                (selected < 0 || part->voice[i].amp.level > part->voice[selected].amp.level))
                selected = (int)i;
        if (selected >= 0) {
            const dnbx_s1_envelope_state *e = filter ? &part->voice[selected].filter : &part->voice[selected].amp;
            *level = e->level; *stage = e->stage; *key_down = part->down[selected];
        }
    }
#endif
    plat_cq_unlock(lock);
    return selected >= 0;
}

#if defined(X0X_S1_BEND_RANGE_PROBE) && X0X_S1_BEND_RANGE_PROBE
static void s1_pb_load(unsigned v, const sound_t *sound)
{
    if (!live_s1_mod(sound,v)) return;
    const uint8_t *row = read137(sound,v);
    if (row[S1_PB_DN_BYTE]>S1_PB_LAST || row[S1_PB_UP_BYTE]>S1_PB_LAST) return;
    (void)s1_part_bend_ranges(&original_bass[v].dsp.synthone,
                             (float)s1_pb_decode(row[S1_PB_DN_BYTE])-24.f,
                             (float)s1_pb_decode(row[S1_PB_UP_BYTE]));
}
#if defined(X0X_SJ6_BEND_TYPED_PROBE) && X0X_SJ6_BEND_TYPED_PROBE
static int sj6_pb_apply(unsigned owner,const sound_t *sound)
{
    if (!live137(sound,owner) || original_bass[owner].kind!=ORIGINAL_SOUND_JUNO) return 0;
    const uint8_t *row=read137(sound,owner);
    return row[SJ6_PB_DN_BYTE]<=SJ6_PB_LIMIT && row[SJ6_PB_UP_BYTE]<=SJ6_PB_LIMIT &&
           juno6_set_bend_ranges(&original_bass[owner].dsp.juno.synth,
               sj6_pb_decode(row[SJ6_PB_DN_BYTE]),sj6_pb_decode(row[SJ6_PB_UP_BYTE]));
}
int sj6_typed_pb_edit(unsigned owner,unsigned up,unsigned pos)
{
    if (up>1 || pos>SJ6_PB_LIMIT || !live137(mot_base_snd,owner)) return 0;
    engine_project_hold(1);
    int ok=live137(mot_base_snd,owner) && s1_typed_tuple_editable() && !ui_project_busy() &&
           !note_lock_applied[owner] && !engine_bass_preset_pending(owner) &&
           original_bass[owner].kind==ORIGINAL_SOUND_JUNO &&
           original_bass[owner].status==ORIGINAL_RUNTIME_OK;
    if (ok) {
        const uint8_t *row=read137(mot_base_snd,owner);
        unsigned down=up ? sj6_pb_decode(row[SJ6_PB_DN_BYTE]) : SJ6_PB_LIMIT-pos;
        unsigned above=up ? pos : sj6_pb_decode(row[SJ6_PB_UP_BYTE]);
        ok=juno6_set_bend_ranges(&original_bass[owner].dsp.juno.synth,down,above);
        if(ok) slot137((sound_t *)mot_base_snd,owner)[SJ6_PB_DN_BYTE+up]=
            (uint8_t)sj6_pb_encode(up?above:down);
    }
    engine_project_hold(0);
    return ok;
}
void sj6_typed_pb_undo(unsigned mask)
{
    for (unsigned owner=0;owner<NBASS;++owner)
        if(mask&(1u<<owner)) (void)sj6_pb_apply(owner,mot_base_snd);
}
#endif
#if defined(X0X_SJ6_MOD_TYPED_PROBE)
static int sj6_typed_mod_apply(unsigned owner,const sound_t *sound)
{
    if(!live137(sound,owner) || original_bass[owner].kind!=ORIGINAL_SOUND_JUNO)return 0;
    unsigned route=read137(sound,owner)[SJ6_MOD_BYTE];
    if(route>SJ6_MOD_LAST)return 0;
    juno6_t *s=&original_bass[owner].dsp.juno.synth;
    /* Actual original Init (raw ID180 base 0; ID7 current 1; ID89 amount 0).
     * Prepare dormant state once; configuring at every edit would reset
     * in-flight alpha, phase or the most recent performance value. */
    if(!s->lfo1_pwm_route && !juno6_configure_lfo1_cutoff(s,0.f,1.f,0.f))return 0;
    return juno6_set_mod_destination(s,route);
}
int sj6_typed_mod_choose(unsigned owner,unsigned route)
{
    if(route>SJ6_MOD_LAST || !live137(mot_base_snd,owner))return 0;
    engine_project_hold(1);
    int ok=live137(mot_base_snd,owner)&&s1_typed_tuple_editable()&&!ui_project_busy()
        &&!note_lock_applied[owner]&&!engine_bass_preset_pending(owner)
        &&original_bass[owner].kind==ORIGINAL_SOUND_JUNO;
    if(ok){
        ok=juno6_set_mod_destination(&original_bass[owner].dsp.juno.synth,route);
        if(ok)slot137((sound_t *)mot_base_snd,owner)[SJ6_MOD_BYTE]=(uint8_t)route;
    }
    engine_project_hold(0);
    return ok;
}
void sj6_typed_mod_undo(unsigned mask)
{
    for(unsigned owner=0;owner<NBASS;owner++)
        if(mask&(1u<<owner)) (void)sj6_typed_mod_apply(owner,mot_base_snd);
}
#endif
int s1_typed_pb_undo_ready(unsigned mask)
{
    if (!s1_typed_tuple_editable() || ui_project_busy()) return 0;
    for (unsigned v=0; v<NBASS; ++v)
        if ((mask & (1u<<v)) && (note_lock_applied[v] || engine_bass_preset_pending(v))) return 0;
    return 1;
}
void s1_typed_pb_undo(unsigned mask)
{
    for (unsigned v=0; v<NBASS; ++v)
        if ((mask & (1u<<v)) && original_bass[v].kind==ORIGINAL_SOUND_SYNTHONE)
            s1_pb_load(v,mot_base_snd);
}
#endif
int engine_s1_patch_read(unsigned voice, unsigned factory_id, s1_patch_value *out)
{
    int slot = factory_id == S1_LINK_ID ? S1_PATCH_COUNT : s1_patch_slot(factory_id);
    if (voice >= NBASS || slot < 0 || !out) return 0;
    uint32_t lock = plat_cq_lock();
    const original_runtime_part *p = &original_bass[voice];
    if (p->kind != ORIGINAL_SOUND_SYNTHONE || p->status != ORIGINAL_RUNTIME_OK) {
        plat_cq_unlock(lock);
        return 0;
    }
#if defined(X0X_S1_BEND_RANGE_PROBE) && X0X_S1_BEND_RANGE_PROBE
    if (s1_bend_slot((unsigned)slot) && !live_s1_mod(mot_base_snd,voice)) {
        plat_cq_unlock(lock);
        return 0;
    }
#endif
    out->revision = s1_patch_identity(voice);
    out->preset = bass_named_preset[voice];
    out->note_locked = note_lock_applied[voice];
    out->physical = s1_part_patch_value(&p->dsp.synthone, (unsigned)slot);
    out->max_position = (uint8_t)s1_patch_steps((unsigned)slot);
    if (slot == S1_PATCH_COUNT) out->position = (uint8_t)out->physical;
#if X0X_S1_EXTRA
    else if (s1_lfo_rate_slot((unsigned)slot)) {
        out->max_position = (uint8_t)s1c_front_lfo_steps(&p->dsp.synthone.e);
        out->position = (uint8_t)s1c_front_lfo_position(&p->dsp.synthone.e, factory_id);
    }
#endif
    else {
        const s1_patch_descriptor *d = s1_patch_at((unsigned)slot);
        out->position = (uint8_t)((out->physical - d->minimum) *
            (out->max_position / (d->maximum - d->minimum)) + .5f);
    }
    plat_cq_unlock(lock);
    return 1;
}

int engine_s1_patch_set(unsigned voice, uint32_t revision, unsigned factory_id, unsigned position)
{
    int slot = factory_id == S1_LINK_ID ? S1_PATCH_COUNT : s1_patch_slot(factory_id);
    if (voice >= NBASS || slot < 0 || position > s1_patch_steps((unsigned)slot))
        return S1_EDIT_REJECTED;
    uint32_t lock = plat_cq_lock();
    int status = S1_EDIT_REJECTED;
    uint32_t request = bass_request[voice];
    if (original_bass[voice].kind == ORIGINAL_SOUND_SYNTHONE &&
        original_bass[voice].status == ORIGINAL_RUNTIME_OK &&
        s1_patch_identity(voice) == revision &&
        !((request & 0x80000000u) && request != bass_applied[voice])) {
        if (s1_edit[voice].pending || note_lock_applied[voice]) status = S1_EDIT_BUSY;
#if X0X_S1_EXTRA
#if defined(X0X_S1_BEND_RANGE_PROBE) && X0X_S1_BEND_RANGE_PROBE
        else if (s1_bend_slot((unsigned)slot)) {
            if (!live_s1_mod(mot_base_snd,voice) || !s1_typed_tuple_editable() || ui_project_busy()) {
                plat_cq_unlock(lock);
                return S1_EDIT_REJECTED;
            }
            s1_edit[voice].slot = (uint8_t)slot;
            s1_edit[voice].position = (uint8_t)position;
            s1_edit[voice].pending = 1;
            status = S1_EDIT_QUEUED;
        }
#endif
        else if (slot == S1_LFO_SYNC &&
                 s1_part_patch_value(&original_bass[voice].dsp.synthone, S1_LFO_SYNC) == (float)position)
            status = S1_EDIT_REJECTED;
        else if (s1_lfo_rate_slot((unsigned)slot) &&
                 position > s1c_front_lfo_steps(&original_bass[voice].dsp.synthone.e)) status = S1_EDIT_REJECTED;
        else if (slot >= S1_EXTRA_FIRST && !s1_base_edit_slot((unsigned)slot) &&
                 (!mot_base_snd || !original_sound_s1_extra_available(mot_base_snd->v[T_303][voice],
                  mot_base_snd->v[T_303][S1_EXTRA_ROW + voice], (unsigned)slot))) status = S1_EDIT_REJECTED;
#endif
        else {
            s1_edit[voice].slot = (uint8_t)slot;
#if X0X_S1_EXTRA
            if (slot >= S1_LFO_FIRST && !s1_base_edit_slot((unsigned)slot))
                position += s1_lfo_rate_slot((unsigned)slot) &&
                            s1c_front_lfo_steps(&original_bass[voice].dsp.synthone.e) < S1_PATCH_STEPS ?
                            S1_LFO_CODE_SYNC : 1u;
#endif
            s1_edit[voice].position = (uint8_t)position;
            s1_edit[voice].pending = 1;
            status = S1_EDIT_QUEUED;
        }
    }
    plat_cq_unlock(lock);
    return status;
}
#endif

static int bass_has_locks(int voice)
{
    if (mot.lane) for (int k = 0; k < NLANE; ++k)
        if (mot.lane[k].used && mot.lane[k].t == T_303 && mot.lane[k].v == voice) return 1;
    return 0;
}
static void bass_tag_set(sound_t *sound, int voice, int bank, int preset, int edited)
{
    bass_sound_info_t info = {{original_sound_engine_id((unsigned)bank), ORIGINAL_SOUND_SCHEMA,
        (uint8_t)original_sound_count((unsigned)bank), BASS_IDENTITY_KNOWN},
        (uint16_t)preset, (uint8_t)(edited ? BASS_SOUND_EDITED : 0)};
    (void)bass_sound_tag_encode(&sound->bass[voice], &info, BASS_SOUND_REGISTRY, 4);
}
#if X0X_S1_EXTRA
_Static_assert(S1_EXTRA_ROW + NBASS <= NVOICES_MAX, "Synth One project rows fit");
_Static_assert(S1_EXTRA_ROW == BASS_SHARED_EXTRA_ROW, "Synth One uses shared bass rows");
_Static_assert(S1_EXTRA_ROW >= ARP_BLOCK_VOICE0 + NBASS, "Synth One records avoid ARP slots");
#if X0X_SUPERJ6_BUILTIN_INIT
_Static_assert(S1_EXTRA_ROW >= ORIGINAL_SJ6_EXTRA_ROW + NBASS, "Synth One records avoid SuperJ6 rows");
#endif
#endif
#if X0X_SYNTHONE_PATCH
static void s1_patch_boundary(void)
{
    if (bass_edit_depth || !engine_s1_patch_pending()) return;
    for (unsigned v = 0; v < NBASS; ++v) {
        if (!s1_edit[v].pending || note_lock_applied[v]) continue;
        s1_edit[v].pending = 0;
        if (!mot_base_snd || original_bass[v].kind != ORIGINAL_SOUND_SYNTHONE ||
            original_bass[v].status != ORIGINAL_RUNTIME_OK) continue;
        sound_t *sound = (sound_t *)mot_base_snd;
        uint8_t *cell = sound->v[T_303][v];
#if X0X_S1_EXTRA
#if defined(X0X_S1_BEND_RANGE_PROBE) && X0X_S1_BEND_RANGE_PROBE
        if (s1_bend_slot(s1_edit[v].slot)) {
            if (!live_s1_mod(sound,v) || !s1_typed_tuple_editable() || ui_project_busy()) continue;
            slot137(sound,v)[s1_edit[v].slot == S1_BEND_LO ? S1_PB_DN_BYTE : S1_PB_UP_BYTE] =
                (uint8_t)s1_pb_encode(s1_edit[v].position);
            s1_pb_load(v,sound);
            continue;
        }
#endif
        if (s1_edit[v].slot >= S1_EXTRA_FIRST && !s1_base_edit_slot(s1_edit[v].slot)) {
            uint8_t *extra = sound->v[T_303][S1_EXTRA_ROW + v];
            s1_dry_part *part = &original_bass[v].dsp.synthone;
            unsigned slot = s1_edit[v].slot, code = s1_edit[v].position;
            if (slot == S1_LFO_SYNC) {
                uint8_t divisions[2] = {0};
                if (code == 1u) {
                    divisions[0] = (uint8_t)s1c_front_lfo_position(&part->e, s1_patch_at(S1_LFO1_RATE)->factory_id);
                    divisions[1] = (uint8_t)s1c_front_lfo_position(&part->e, s1_patch_at(S1_LFO2_RATE)->factory_id);
                }
                if (!original_sound_s1_sync_set(cell, extra, code, divisions)) continue;
            } else if (!original_sound_s1_extra_set(cell, extra, slot, code)) continue;
            s1_part_apply_extra(part, extra);
#if defined(X0X_S1_AUTOPAN_PROBE) && X0X_S1_AUTOPAN_PROBE
            /* The admitted physical clock edit owns its refreshed dependent
               normalized display and actual Hz snapshot, not another write. */
            if(slot==S1_LFO_SYNC && live_s1_mod(sound,v) &&
               s1_pan_rate_word(read137(sound,v)))
                s1_pan_rate_put(slot137(sound,v),part->e.t[36]);
#endif
        } else
#endif
        {
            if (!original_sound_s1_set(cell, s1_edit[v].slot, s1_edit[v].position)) continue;
            s1_part_apply_cell(&original_bass[v].dsp.synthone, cell);
        }
        bass_tag_set(sound, v, ORIGINAL_SOUND_SYNTHONE, bass_named_preset[v],
                     original_sound_is_edited(cell));
    }
}
#endif
#if X0X_SUPERJ6_BUILTIN_INIT
enum { SJ6_MAILBOX_WORD_BITS = 32, SJ6_MAILBOX_WORDS = 2,
       SJ6_MAILBOX_SLOTS = SJ6_MAILBOX_WORDS * SJ6_MAILBOX_WORD_BITS };
typedef struct { uint32_t word[SJ6_MAILBOX_WORDS]; } sj6_knob_bits;
static volatile sj6_knob_bits sj6_knob_mask[NBASS];
#if defined(X0X_HOST) && defined(X0X_SJ6_MAILBOX_PROBE)
static uint8_t sj6_knob_value[NBASS][SJ6_MAILBOX_SLOTS];
#else
static uint8_t sj6_knob_value[NBASS][SJ6_KNOBS];
#endif
_Static_assert((unsigned)SJ6_KNOBS <= SJ6_MAILBOX_SLOTS, "SuperJ6 knob mailbox capacity");

static uint32_t sj6_knob_bit(unsigned slot)
{ return UINT32_C(1) << (slot % SJ6_MAILBOX_WORD_BITS); }

static int sj6_knob_has(unsigned owner, unsigned slot)
{ return (sj6_knob_mask[owner].word[slot / SJ6_MAILBOX_WORD_BITS] & sj6_knob_bit(slot)) != 0; }

static void sj6_knob_clear(unsigned owner)
{
    sj6_knob_mask[owner].word[0] = 0;
    sj6_knob_mask[owner].word[1] = 0;
}

static void sj6_knob_post(unsigned owner, unsigned slot, int value)
{
    sj6_knob_value[owner][slot] = (uint8_t)value;
    sj6_knob_mask[owner].word[slot / SJ6_MAILBOX_WORD_BITS] |= sj6_knob_bit(slot);
}

static void sj6_apply_extra(juno6_t *s, unsigned mask, const uint8_t *values, int reset)
{
    const unsigned master = SJ6_MASTER_LEVEL - SJ6_OSC_KNOBS;
    if (mask & ((1u << master) - 1u) || reset)
        juno6_apply_panel_envelopes(s, mask, values, reset);
    if (mask & (1u << master)) juno6_apply_panel_master(s, values[master]);
    else if (reset) juno6_apply_panel_master(s, SJ6_MASTER_INIT);
    for (unsigned i = 0; i < SJ6_LFO_COUNT; ++i) {
        const unsigned rate = ORIGINAL_SJ6_RATE_INDEX + i, mode = ORIGINAL_SJ6_MODE_INDEX + i;
        int position = mask & (1u << rate) ? (int)(values[rate] & 127u) : -1;
        if (position >= 0 || (mask & (1u << mode))) {
            /* Old rate-only edits selected Hz. Preserve that saved behavior. */
            unsigned hz = (mask & (1u << mode)) ? values[rate] >> 7 : 1u;
            juno6_set_panel_lfo(s, i, position, hz);
        }
    }
}

static void sj6_restore_extra(original_runtime_part *part, const uint8_t *cell,
                              const uint8_t *extra, int reset)
{
    unsigned mask = original_sound_edit_mask(cell) & ORIGINAL_SJ6_EXTRA_BIT ?
                    original_sound_sj6_extra_mask(extra) : 0;
    if (mask || reset) sj6_apply_extra(&part->dsp.juno.synth, mask, extra + 2, reset);
}
static void sj6_restore_semi(original_runtime_part *part, const uint8_t *cell, const uint8_t *extra)
{
    unsigned marked = original_sound_edit_mask(cell) & ORIGINAL_SJ6_EXTRA_BIT;
    for (unsigned l = 0; l < DNBX_LAYERS; ++l) {
        juno6_set_layer_semi(&part->dsp.juno.synth, l,
                            marked ? original_sound_sj6_layer_semi(extra, l) : SJ6_SEMI_CENTER);
        juno6_set_layer_fine(&part->dsp.juno.synth, l, original_sound_sj6_fine(cell, l));
    }
}

static void sj6_restore_authored_lfo(original_runtime_part *part, const uint8_t *cell, const uint8_t *extra)
{
    juno6_lfo_words words;
    int has_words = (original_sound_edit_mask(cell) & ORIGINAL_SJ6_AUTHORED_LFO_BIT) &&
                    original_sound_sj6_lfo_words(extra, &words);
    juno6_set_lfo_words(&part->dsp.juno.synth, has_words ? &words : NULL, 0);
}

static int sj6_load_vibrato(const uint8_t *cell, const uint8_t *extra, const uint8_t *wire, float values[2])
{
    if (original_sound_sj6_vibrato_marker(cell) == 1u) {
        values[0] = unit_control(extra[ORIGINAL_SJ6_VIBRATO_OFFSET]);
        values[1] = unit_control(extra[ORIGINAL_SJ6_VIBRATO_OFFSET + 1u]);
        return 3;
    }
    return original_sound_sj6_vibrato_words(cell, wire, values);
}

_Static_assert((unsigned)SJ6_A_REFERENCE < SJ6_MAILBOX_WORD_BITS &&
               (unsigned)SJ6_LFO2_SHAPE == SJ6_MAILBOX_WORD_BITS, "SuperJ6 mailbox dispatch order");
_Static_assert(ORIGINAL_SJ6_VIBRATO_ROW == BASS_SHARED_EXTRA_ROW, "SuperJ6 uses shared bass rows");
_Static_assert(BASS_SHARED_EXTRA_ROW + NBASS <= NVOICES_MAX, "SuperJ6 shared rows fit");
_Static_assert(BASS_SHARED_EXTRA_BYTES >= ORIGINAL_SJ6_TUNING_WIRE_BYTES, "Undo copies the tuning record");

static int sj6_wire_storage_ready(unsigned owner, unsigned vibrato, unsigned a_reference)
{
    if (!mot_base_snd) return 1;
    const uint8_t *cell = mot_base_snd->v[T_303][owner];
    const uint8_t *wire = mot_base_snd->v[T_303][BASS_SHARED_EXTRA_ROW + owner];
    if (!original_sound_sj6_marker_valid(cell)) return 0;
    if (a_reference || original_sound_sj6_has_a_reference(cell))
        return original_sound_sj6_a_reference_available(cell, wire);
    return (!vibrato && !original_sound_sj6_vibrato_mask(cell)) ||
           original_sound_sj6_vibrato_available(cell, wire);
}

static int sj6_factory_storage_ready(unsigned owner, const sj6_factory_motion *motion)
{
    if (!sj6_wire_storage_ready(owner, motion->vibrato_present, motion->a_reference_present)) return 0;
    return !mot_base_snd || !(motion->semi_authored[0] | motion->semi_authored[1]) ||
           original_sound_sj6_semi_ready(mot_base_snd->v[T_303][owner],
                                        mot_base_snd->v[T_303][ORIGINAL_SJ6_EXTRA_ROW + owner]);
}

static void sj6_commit_factory(unsigned owner, const sj6_factory_motion *motion, unsigned chorus_mode)
{
    juno6_t *s = &original_bass[owner].dsp.juno.synth;
    juno6_set_lfo_words(s, &motion->lfo, 1);
    juno6_set_lfo2_pwm(s, 0);
    juno6_set_lfo2_shape(s, motion->lfo2_shape);
    juno6_set_vibrato_words(s, motion->vibrato, motion->vibrato_present, 1);
    if (motion->a_reference_present || (s->core.raw_present[SJ6_A_REFERENCE_ID / 32u] &
                                       (1u << (SJ6_A_REFERENCE_ID % 32u))))
        juno6_set_a_reference(s, motion->a_reference, motion->a_reference_present);
    for (unsigned l = 0; l < DNBX_LAYERS; ++l) if (motion->semi_authored[l])
        juno6_set_layer_semi(s, l, motion->semi_authored[l] - 1u);
    if (!mot_base_snd) return;
    sound_t *sound = (sound_t *)mot_base_snd;
    uint8_t *cell = sound->v[T_303][owner];
    uint8_t *extra = sound->v[T_303][ORIGINAL_SJ6_EXTRA_ROW + owner];
    for (unsigned l = 0; l < DNBX_LAYERS; ++l) if (motion->semi_authored[l])
        original_sound_sj6_semi_store(cell, extra, l, motion->semi_authored[l] - 1u);
    original_sound_sj6_lfo_store(cell, extra, &motion->lfo);
    original_sound_sj6_lfo2_pwm_store(cell, 0);
    original_sound_sj6_shape_store(cell, motion->lfo2_shape);
    original_sound_sj6_chorus_store(cell, chorus_mode);
    uint8_t *wire = sound->v[T_303][BASS_SHARED_EXTRA_ROW + owner];
    if (original_sound_sj6_vibrato_marker(cell) == 1u) memset(extra + ORIGINAL_SJ6_VIBRATO_OFFSET, 0, 2u);
    if (motion->vibrato_present || original_sound_sj6_vibrato(cell))
        original_sound_sj6_vibrato_store(cell, wire, motion->vibrato, motion->vibrato_present);
    if (motion->a_reference_present || original_sound_sj6_has_a_reference(cell))
        original_sound_sj6_a_reference_store(cell, wire, motion->a_reference, motion->a_reference_present);
    original_runtime_sj6_chorus(&original_bass[owner], cell);
    bass_tag_set(sound, owner, ORIGINAL_SOUND_JUNO, bass_named_preset[owner],
                 original_sound_is_edited(cell));
}

int engine_superj6_factory_pending(void)
{
    for (unsigned v = 0; v < NBASS; ++v)
        if (original_bass[v].kind == ORIGINAL_SOUND_JUNO &&
            (original_bass[v].dsp.juno.synth.pending_field_mask & SJ6_FACTORY_GROUP)) return 1;
    return 0;
}

int engine_superj6_knob_pending(void)
{
    return (sj6_knob_mask[0].word[0] | sj6_knob_mask[0].word[1] |
            sj6_knob_mask[1].word[0] | sj6_knob_mask[1].word[1]) != 0;
}

#if defined(DNBOX_137_HOST_WITNESS)
#include "sj6_137_host_witness.h"
#endif
extern int s1_typed_tuple_editable(void);
#if X0X_S1_FENCED_CLOCK
/* Volatile exact before-image: rate snapping cannot be inverted on Undo. */
static const uint8_t clock_tones[]={64,15,47,36,43};
static uint8_t tone_clock_active[NBASS];
static float tone_clock_before[NBASS][5];
#if defined(X0X_S1_AUTOPAN_PROBE) && X0X_S1_AUTOPAN_PROBE
static void s1_typed_pan_apply(const sound_t *,unsigned,int);
#endif
static void s1_typed_clock_tones(const sound_t *sound,unsigned v)
{
    if(!live_s1_mod(sound,v) || original_bass[v].kind!=ORIGINAL_SOUND_SYNTHONE)return;
    const uint8_t *p=read137(sound,v)+S1_ARP_CLOCK_BYTE;
    if(!s1_clock_valid(p))return;
    s1c *e=&original_bass[v].dsp.synthone.e;
    if(s1_clock_rate_authored(p)) {
        if(!tone_clock_active[v])
            for(unsigned i=0;i<sizeof clock_tones;i++)tone_clock_before[v][i]=e->t[clock_tones[i]];
        tone_clock_active[v]=1;
        (void)s1c_front_set(e,64,(float)s1_clock_bpm(p));
    } else if(tone_clock_active[v]) {
        for(unsigned i=0;i<sizeof clock_tones;i++)s1c_set(e,clock_tones[i],tone_clock_before[v][i]);
        tone_clock_active[v]=0;
    }
}
static void s1_typed_clock_apply(const sound_t *sound,unsigned v)
{
    original_runtime_part *part=&original_bass[v];
    if (part->kind!=ORIGINAL_SOUND_SYNTHONE) return;
    const uint8_t *p=read137(sound,v)+S1_ARP_CLOCK_BYTE;
    if(live_s1_mod(sound,v) && s1_clock_valid(p) && p[2])
        (void)original_runtime_s1_clock(part,s1_clock_bpm(p),p[2]-1u);
    else {
        part->arp.source_set=0;
        part->arp.source_wait=0;
        part->arp.source_phase=0;
    }
}
int engine_arp_source_get(unsigned owner,unsigned field)
{
    if (!live_s1_mod(mot_base_snd,owner)) return -1;
    const uint8_t *p=read137(mot_base_snd,owner)+S1_ARP_CLOCK_BYTE;
    if (!s1_clock_valid(p)) return -1;
    if (field==ARP_F_SOURCE_RATE) return p[2] ? s1_clock_bpm(p) : 120;
    if (field==ARP_F_SOURCE_FACTOR) return p[2] ? p[2]-1u : 9;
    return -1;
}
int engine_arp_source_set(unsigned owner,unsigned field,int value)
{
    if (field==ARP_F_SOURCE_RATE ? value<1||value>200 :
        field!=ARP_F_SOURCE_FACTOR || value<0||value>18) return 0;
    if(!live_s1_mod(mot_base_snd,owner))return 0;
    engine_project_hold(1);
    int ok=!ui_project_busy() && s1_typed_tuple_editable() && live_s1_mod(mot_base_snd,owner) &&
        !engine_bass_preset_pending(owner) && original_bass[owner].kind==ORIGINAL_SOUND_SYNTHONE;
    if(ok) {
        uint8_t *p=slot137((sound_t *)mot_base_snd,owner)+S1_ARP_CLOCK_BYTE;
        if(!s1_clock_valid(p))ok=0;
        else {
            unsigned rate=field==ARP_F_SOURCE_RATE ? (unsigned)value : p[2] ? s1_clock_bpm(p) : 120;
            unsigned factor=field==ARP_F_SOURCE_FACTOR ? (unsigned)value+1u : p[2] ? p[2] : 10;
            p[0]=(uint8_t)rate;p[2]=(uint8_t)factor;
            if(field==ARP_F_SOURCE_RATE)p[1]=S1_ARP_CLOCK_RATE_AUTHORED;
            s1_typed_clock_apply(mot_base_snd,owner);
            if(field==ARP_F_SOURCE_RATE) {
                s1_typed_clock_tones(mot_base_snd,owner);
#if defined(X0X_S1_AUTOPAN_PROBE) && X0X_S1_AUTOPAN_PROBE
                uint8_t *side=slot137((sound_t*)mot_base_snd,owner);
                if(s1_pan_rate_word(side))s1_pan_rate_put(side,original_bass[owner].dsp.synthone.e.t[36]);
#endif
            }
        }
    }
    engine_project_hold(0);
    return ok;
}
void s1_typed_clock_undo(unsigned mask) {
    for(unsigned v=0;v<NBASS;v++)if(mask&(1u<<v)) {
        s1_typed_clock_apply(mot_base_snd,v);
        s1_typed_clock_tones(mot_base_snd,v);
#if defined(X0X_S1_AUTOPAN_PROBE) && X0X_S1_AUTOPAN_PROBE
        if(live_s1_mod(mot_base_snd,v)&&s1_pan_rate_word(read137(mot_base_snd,v)))
            s1_typed_pan_apply(mot_base_snd,v,1);
#endif
    }
}
#endif
#if defined(X0X_S1_FEEDBACK_PROBE) && X0X_S1_FEEDBACK_PROBE
static void s1_typed_feedback_apply(const sound_t *sound,unsigned v)
{
    if(!live_s1_mod(sound,v)||original_bass[v].kind!=ORIGINAL_SOUND_SYNTHONE)return;
    s1c *e=&original_bass[v].dsp.synthone.e;
    unsigned code=read137(sound,v)[S1_FB_BYTE];
    /* Fresh typed Init is already source-direct before its first render,
       including unedited 0.1. No change of mode on the first detent. */
    e->delay_feedback_direct=1;
    (void)s1c_front_set(e,S1C_P_DLFB,code?.9f*(float)(code-1u)/127.f:s1c_front_init(S1C_P_DLFB));
}
int s1_typed_feedback_choose(unsigned v,unsigned pos)
{
    if(pos>127 || !live_s1_mod(mot_base_snd,v))return 0;
    engine_project_hold(1);
    int ok=live_s1_mod(mot_base_snd,v) && s1_typed_tuple_editable() && !ui_project_busy() &&
           !engine_bass_preset_pending(v) && original_bass[v].kind==ORIGINAL_SOUND_SYNTHONE;
    if(ok) {
        slot137((sound_t *)mot_base_snd,v)[S1_FB_BYTE]=(uint8_t)(pos+1u);
        s1_typed_feedback_apply(mot_base_snd,v);
    }
    engine_project_hold(0);
    return ok;
}
void s1_typed_feedback_undo(unsigned mask)
{
    for(unsigned v=0;v<NBASS;v++)if(mask&(1u<<v))s1_typed_feedback_apply(mot_base_snd,v);
}
#endif
#if X0X_SJ6_CLASSIC_I_PROBE
#include <math.h>
const classic_mode_i *sj6_classic_i_probe(unsigned v) {
 return v<NBASS && original_bass[v].classic_i_authored ? &original_bass[v].classic_i_effect : 0;
}
static int sj6_classic_i_active(const sound_t *sound,unsigned v) {
 return live137(sound,v) && original_bass[v].kind==ORIGINAL_SOUND_JUNO &&
   original_bass[v].dsp.juno.chorus.factory_mode>=JUNO6_CHORUS_I &&
   original_bass[v].dsp.juno.chorus.factory_mode<=JUNO6_CHORUS_BOTH;
}
static void sj6_classic_i_apply(const sound_t *sound,unsigned v) {
 original_runtime_part *p=&original_bass[v];
 if(!sj6_classic_i_active(sound,v)) {
  if (p->classic_i_authored) classic_wrapper_start(&p->classic_i_effect,0);
  return;
 }
 const unsigned mode=p->dsp.juno.chorus.factory_mode-JUNO6_CHORUS_OFF;
 const uint8_t *row=read137(sound,v)+SJ6_CLASSIC_I_BYTE;
 if(!sj6_classic_i_bytes_valid(row))return;
 const int set=row[0]||row[1]||row[2];
 if(set && !p->classic_i_authored) {
  /* Cold-created delay/ramp state, seeded by exact active mode defaults. */
  classic_mode_i *fx=&p->classic_i_effect;
  const float zero[4]={0.f,0.f,0.f,0.f};
  classic_wrapper_prepare(fx,zero,10000u);
  (void)classic_wrapper_enter(fx,mode);
  for(unsigned i=0;i<4;++i)
   fx->p[i].target=fx->p[i].current=fx->p[i].initial=fx->p[i].pending;
 }
 p->classic_i_authored=(uint8_t)!!set;
 if(set) {
  /* Original mode default transaction first; then restore authored overrides.
   * Pending targets latch at the render quantum, preserving existing ramps. */
  (void)classic_wrapper_enter(&p->classic_i_effect,mode);
  for(unsigned i=0;i<3;++i)
   if(row[i] && row[i]!=SJ6_CLASSIC_I_DEFAULT)
    (void)classic_wrapper_command(&p->classic_i_effect,42+i,(row[i]-1u)/127.f);
 }
}
int sj6_classic_i_choose(unsigned v,unsigned slot,unsigned pos) {
 if(slot>=3||pos>127||!sj6_classic_i_active(mot_base_snd,v))return 0;
 engine_project_hold(1);
 int ok=!ui_project_busy() && s1_typed_tuple_editable() && sj6_classic_i_active(mot_base_snd,v) &&
    !engine_bass_preset_pending(v)&&!note_lock_applied[v];
 if(ok) {
  slot137((sound_t*)mot_base_snd,v)[SJ6_CLASSIC_I_BYTE+slot]=(uint8_t)(pos+1u);
  sj6_classic_i_apply(mot_base_snd,v);
 }
 engine_project_hold(0);
 return ok;
}
void sj6_classic_i_undo(unsigned mask) {
 for(unsigned v=0;v<NBASS;++v)if(mask&(1u<<v))sj6_classic_i_apply(mot_base_snd,v);
}
#endif
#if defined(X0X_SJ6_MONO32_PROBE) && X0X_SJ6_MONO32_PROBE
static int sj6_voice_mode_set(unsigned v,unsigned mode)
{
 original_runtime_part *p=&original_bass[v];
 juno6_t *s=&p->dsp.juno.synth;
 if(s->mono32_on!=mode){
    /* Soft receiver release below; cut only this owner's stale ingress. */
    for(unsigned source=0;source<ORIGINAL_INPUT_COUNT;++source)
       (void)original_runtime_cut_source(p,source);
 }
 return juno6_mono32_set(s,mode);
}
static void sj6_voice_apply(const sound_t *sound,unsigned v)
{
 if(!live137(sound,v)||original_bass[v].kind!=ORIGINAL_SOUND_JUNO)return;
 const uint8_t *data=sj6_voice_read(sound,v);
 if(!sj6_voice_valid(v,data[0],data[1]))return;
 juno6_t *s=&original_bass[v].dsp.juno.synth;
 if(!sj6_voice_mode_set(v,data[0]&1u))return;
 (void)juno6_legato33_set(s,(data[0]>>1)&1u);
 (void)juno6_glide35_set(s,data[1]);
}
int engine_sj6_voice_undo_ready(unsigned mask)
{
 for(unsigned v=0;v<NBASS;v++)
  if((mask&(1u<<v))&&live137(mot_base_snd,v)&&original_bass[v].kind==ORIGINAL_SOUND_JUNO&&note_lock_applied[v])return 0;
 return 1;
}
void engine_sj6_voice_undo(unsigned mask)
{
 for(unsigned v=0;v<NBASS;v++)if(mask&(1u<<v))sj6_voice_apply(mot_base_snd,v);
}
int engine_sj6_mono32_get(unsigned v,unsigned field)
{
 if(v>=NBASS||!live137(mot_base_snd,v)||original_bass[v].kind!=ORIGINAL_SOUND_JUNO)return -1;
 if(field>2u)return -1;
 const juno6_t *s=&original_bass[v].dsp.juno.synth;
 return field==2u?s->glide35_detent:field?s->legato33_on:s->mono32_on;
}
int engine_sj6_mono32_edit(unsigned v,unsigned field,unsigned on)
{
 if(engine_sj6_mono32_get(v,field)<0 || on>(field==2u?127u:1u))return 0;
 engine_project_hold(1);
 int ok=engine_sj6_mono32_get(v,field)>=0&&!ui_project_busy()&&s1_typed_tuple_editable()&&
     !engine_bass_preset_pending(v)&&!note_lock_applied[v]&&
     (field==2u?juno6_glide35_set(&original_bass[v].dsp.juno.synth,on):
      field?juno6_legato33_set(&original_bass[v].dsp.juno.synth,on):
      sj6_voice_mode_set(v,on));
 if(ok){
    const juno6_t *s=&original_bass[v].dsp.juno.synth;
    uint8_t *data=sj6_voice_payload((sound_t *)mot_base_snd,v);
    data[0]=sj6_voice_tag(v,s->mono32_on|(s->legato33_on<<1),s->glide35_detent);
    data[1]=s->glide35_detent;
 }
 engine_project_hold(0);return ok;
}
#endif
#ifdef DNBOX_137_HOST_WITNESS
#if X0X_S1_DELAY_TRACK_TEST
/* An Init-owned Delay input ratio, never a T_MIX or global FX address. */
static void s1_typed_dict_apply(const sound_t *sound,unsigned owner)
{
    if(live_s1_mod(sound,owner) && original_bass[owner].kind==ORIGINAL_SOUND_SYNTHONE) {
        unsigned code=s1_dict_read(sound,owner);
        if(code<=S1_DICT_MAX)
            (void)s1c_front_set(&original_bass[owner].dsp.synthone.e,S1_DICT_PARAM,
                               (float)(50u+s1_dict_decode(code))*.01f);
    }
}
int s1_typed_dict_position(unsigned owner)
{
    if(!live_s1_mod(mot_base_snd,owner) ||
       original_bass[owner].kind!=ORIGINAL_SOUND_SYNTHONE) return -1;
    unsigned code=s1_dict_read(mot_base_snd,owner);
    return code<=S1_DICT_MAX ? (int)s1_dict_decode(code) : -1;
}
int s1_typed_dict_choose(unsigned owner,unsigned pos)
{
    if(pos>S1_DICT_MAX || s1_typed_dict_position(owner)<0)return 0;
    engine_project_hold(1);
    int before=s1_typed_dict_position(owner);
    int ok=before>=0 && (unsigned)before!=pos && s1_typed_tuple_editable() &&
           !ui_project_busy() && !engine_bass_preset_pending(owner) && !note_lock_applied[owner];
    if(ok) {
        s1_dict_store((sound_t *)mot_base_snd,owner,s1_dict_encode(pos));
        s1_typed_dict_apply(mot_base_snd,owner);
    }
    engine_project_hold(0);
    return ok;
}
int s1_typed_dict_undo_ready(unsigned mask)
{
    if(!s1_typed_tuple_editable())return 0;
    for(unsigned v=0;v<NBASS;v++)
        if((mask&(1u<<v)) && live_s1_mod(mot_base_snd,v) &&
           (note_lock_applied[v] || engine_bass_preset_pending(v)))return 0;
    return 1;
}
void s1_typed_dict_undo(unsigned mask)
{
    for(unsigned v=0;v<NBASS;v++)if(mask&(1u<<v))s1_typed_dict_apply(mot_base_snd,v);
}
#endif
static void s1_typed_a4_apply(const sound_t *sound,unsigned owner) {
    if(live_s1_mod(sound,owner) && original_bass[owner].kind==ORIGINAL_SOUND_SYNTHONE)
        (void)s1c_front_set(&original_bass[owner].dsp.synthone.e,S1_A4_PARAM,s1_a4_hz(read137(sound,owner)[S1_A4_BYTE]));
}
int s1_typed_a4_choose(unsigned owner,unsigned hz) {
    if(hz<410 || hz>470 || !live_s1_mod(mot_base_snd,owner))return 0;
    engine_project_hold(1);
    int ok=!ui_project_busy() && s1_typed_tuple_editable() &&
           live_s1_mod(mot_base_snd,owner) && !engine_bass_preset_pending(owner) &&
           original_bass[owner].kind==ORIGINAL_SOUND_SYNTHONE;
    if(ok) {
        slot137((sound_t *)mot_base_snd,owner)[S1_A4_BYTE]=(uint8_t)s1_a4_code(hz);
        s1_typed_a4_apply(mot_base_snd,owner);
    }
    engine_project_hold(0);
    return ok;
}
void s1_typed_a4_undo(unsigned mask) {
    for(unsigned v=0;v<NBASS;v++)if(mask&(1u<<v))s1_typed_a4_apply(mot_base_snd,v);
}
static void s1_typed_trans_apply(const sound_t *sound,unsigned owner) {
    if(live_s1_mod(sound,owner) && original_bass[owner].kind==ORIGINAL_SOUND_SYNTHONE)
        (void)s1c_front_set(&original_bass[owner].dsp.synthone.e,S1_TRANS_PARAM,s1_trans_st(read137(sound,owner)[S1_TRANS_BYTE]));
}
int s1_typed_trans_choose(unsigned owner,int st) {
    if(st < -24 || st > 24 || !live_s1_mod(mot_base_snd,owner))return 0;
    engine_project_hold(1);
    int ok=!ui_project_busy() && s1_typed_tuple_editable() &&
           live_s1_mod(mot_base_snd,owner) && !engine_bass_preset_pending(owner) &&
           original_bass[owner].kind==ORIGINAL_SOUND_SYNTHONE;
    if(ok) {
        slot137((sound_t *)mot_base_snd,owner)[S1_TRANS_BYTE]=(uint8_t)s1_trans_code(st);
        s1_typed_trans_apply(mot_base_snd,owner);
    }
    engine_project_hold(0);
    return ok;
}
void s1_typed_trans_undo(unsigned mask) {
    for(unsigned v=0;v<NBASS;v++)if(mask&(1u<<v))s1_typed_trans_apply(mot_base_snd,v);
}
#if defined(X0X_S1_AUTOPAN_PROBE) && X0X_S1_AUTOPAN_PROBE
static s1_dry_part *s1_pan_owner(const sound_t *sound,unsigned v)
{
    return live_s1_mod(sound,v) && original_bass[v].kind==ORIGINAL_SOUND_SYNTHONE
            ? &original_bass[v].dsp.synthone : 0;
}
static void s1_typed_pan_apply(const sound_t *sound,unsigned v,int force_rate)
{
    s1_dry_part *p=s1_pan_owner(sound,v);
    if(!p)return;
    const uint8_t *slot=read137(sound,v);
    unsigned a=slot[S1_PAN_AMOUNT_BYTE];uint32_t r=s1_pan_rate_word(slot);
    if(force_rate && (r || p->e.pan_authored))
        (void)s1c_front_set(&p->e,36,r?s1_pan_rate_hz(r):p->clock_factory_pan);
    p->e.pan_authored=(uint8_t)(a||r);
    (void)s1c_front_set(&p->e,35,(float)a/127.f);
}
int s1_typed_pan_read(unsigned v,unsigned *pos,unsigned *max,int *sync,float *hz)
{
    s1_dry_part *p=s1_pan_owner(mot_base_snd,v);
    if(!p)return 0;
    *sync=s1c_front_pan_sync(&p->e);
    *pos=s1c_front_pan_position(&p->e);*max=s1c_front_pan_stops(&p->e);*hz=p->e.t[36];
    return 1;
}
int s1_typed_pan_choose(unsigned v,int rate,unsigned value)
{
    if(!s1_pan_owner(mot_base_snd,v) || value>127)return 0;
    engine_project_hold(1);
    s1_dry_part *p=s1_pan_owner(mot_base_snd,v);
    int ok=p && !ui_project_busy() && s1_typed_tuple_editable() && !engine_bass_preset_pending(v);
    if(ok) {
        uint8_t *slot=slot137((sound_t *)mot_base_snd,v);
        if(rate) {
            ok=s1c_front_pan_select(&p->e,value);
            if(ok)s1_pan_rate_put(slot,p->e.t[36]);
        } else slot[S1_PAN_AMOUNT_BYTE]=(uint8_t)value;
        if(ok)s1_typed_pan_apply(mot_base_snd,v,0);
    }
    engine_project_hold(0);
    return ok;
}
void s1_typed_pan_undo(unsigned mask)
{
    for(unsigned v=0;v<NBASS;v++)if(mask&(1u<<v))s1_typed_pan_apply(mot_base_snd,v,1);
}
#endif
static void s1_typed_crush_apply(const sound_t *sound,unsigned owner) {
    if(live_s1_mod(sound,owner) && original_bass[owner].kind==ORIGINAL_SOUND_SYNTHONE)
        (void)s1c_front_set(&original_bass[owner].dsp.synthone.e,S1_CRUSH_PARAM,read137(sound,owner)[S1_CRUSH_BYTE]);
}
int s1_typed_crush_choose(unsigned owner,unsigned choice) {
    if(choice>S1_CRUSH_LAST || !live_s1_mod(mot_base_snd,owner))return 0;
    engine_project_hold(1);
    int ok=!ui_project_busy() && s1_typed_tuple_editable() &&
           live_s1_mod(mot_base_snd,owner) && !engine_bass_preset_pending(owner) &&
           original_bass[owner].kind==ORIGINAL_SOUND_SYNTHONE;
    if(ok) {
        slot137((sound_t *)mot_base_snd,owner)[S1_CRUSH_BYTE]=(uint8_t)choice;
        s1_typed_crush_apply(mot_base_snd,owner);
    }
    engine_project_hold(0);
    return ok;
}
void s1_typed_crush_undo(unsigned mask) {
    for(unsigned v=0;v<NBASS;v++)if(mask&(1u<<v))s1_typed_crush_apply(mot_base_snd,v);
}
static void s1_typed_rvfb_apply(const sound_t *sound,unsigned owner) {
    if(live_s1_mod(sound,owner) && original_bass[owner].kind==ORIGINAL_SOUND_SYNTHONE)
        (void)s1c_front_set(&original_bass[owner].dsp.synthone.e,S1C_P_RVFB,(float)s1_rvfb_pos(read137(sound,owner)[S1_RVFB_BYTE])*(1.f/128.f));
}
int s1_typed_rvfb_choose(unsigned owner,unsigned pos) {
    if(pos>S1_RVFB_LAST || !live_s1_mod(mot_base_snd,owner))return 0;
    engine_project_hold(1);
    int ok=!ui_project_busy() && s1_typed_tuple_editable() &&
           live_s1_mod(mot_base_snd,owner) && !engine_bass_preset_pending(owner) &&
           original_bass[owner].kind==ORIGINAL_SOUND_SYNTHONE;
    if(ok) {
        slot137((sound_t *)mot_base_snd,owner)[S1_RVFB_BYTE]=(uint8_t)s1_rvfb_code(pos);
        s1_typed_rvfb_apply(mot_base_snd,owner);
    }
    engine_project_hold(0);
    return ok;
}
void s1_typed_rvfb_undo(unsigned mask) {
    for(unsigned v=0;v<NBASS;v++)if(mask&(1u<<v))s1_typed_rvfb_apply(mot_base_snd,v);
}
int s1_typed_mod_choose(unsigned owner,unsigned route) {
    if (route>2 || !live_s1_mod(mot_base_snd,owner))return 0;
    engine_project_hold(1);
    int ok=!ui_project_busy() && s1_typed_tuple_editable() && live_s1_mod(mot_base_snd,owner) &&
           !note_lock_applied[owner] &&
           !engine_bass_preset_pending(owner) && original_bass[owner].kind==ORIGINAL_SOUND_SYNTHONE;
    if(ok) {
        slot137((sound_t *)mot_base_snd,owner)[3]=(uint8_t)route;
        s1_part_mod_route(&original_bass[owner].dsp.synthone,route);
    }
    engine_project_hold(0);
    return ok;
}
void s1_typed_mod_undo(unsigned mask) {
    for(unsigned owner=0;owner<NBASS;owner++)if((mask&(1u<<owner)) &&
       live_s1_mod(mot_base_snd,owner) && !note_lock_applied[owner] &&
       original_bass[owner].kind==ORIGINAL_SOUND_SYNTHONE)
        s1_part_mod_route(&original_bass[owner].dsp.synthone,read137(mot_base_snd,owner)[3]);
}
void sj6_137_undo_apply(unsigned mask)
{
    for(unsigned v=0;v<NBASS;++v)
        if ((mask&(1u<<v)) && live137(mot_base_snd,v)) {
            sj6_knob_mask[v].word[1]&=~sj6_knob_bit(SJ6_FILTER_VELOCITY);
            if (!note_lock_applied[v] && original_bass[v].kind==ORIGINAL_SOUND_JUNO)
                juno6_set_filter_velocity(&original_bass[v].dsp.juno.synth,read137(mot_base_snd,v)[2]);
        }
}
#endif
static int sj6_knob_ready(unsigned owner)
{
    const original_runtime_part *p = &original_bass[owner];
    return p->kind == ORIGINAL_SOUND_JUNO && p->status == ORIGINAL_RUNTIME_OK &&
           !(p->dsp.juno.synth.pending_field_mask & SJ6_FACTORY_GROUP);
}

static int sj6_knob_position(unsigned owner, unsigned slot)
{
    if (sj6_knob_has(owner, slot)) return sj6_knob_value[owner][slot];
#if defined(X0X_HOST) && defined(X0X_SJ6_MAILBOX_PROBE)
    if (slot >= SJ6_KNOBS) return engine_superj6_mailbox_probe_read(owner, slot);
#endif
#if (defined(X0X_HOST) && defined(X0X_SJ6_FILTER_VEL_PROBE)) || (DNBOX_TYPED_CONTROLS_NATIVE_TEST && X0X_SJ6_VELOCITY_PORT)
    if (slot == SJ6_FILTER_VELOCITY) {
        return live137(mot_base_snd,owner) ?
            original_bass[owner].dsp.juno.synth.filter_velocity_position : -1;
    }
#endif
    if (slot == SJ6_OSC2_LEVEL || slot == SJ6_OSC1_LEVEL) {
        if (bass_named_preset[owner] != 0) return -1;
        unsigned code=slot==SJ6_OSC2_LEVEL ? original_bass[owner].dsp.juno.synth.osc2_level_code :
                      original_bass[owner].dsp.juno.synth.osc1_level_code;
        return code ? (int)code-1 : 128; /* explicit new control vs untouched Init */
    }
    if (slot == SJ6_LFO2_SHAPE)
        return (int)juno6_knob_position(&original_bass[owner].dsp.juno.synth, slot);
    if (slot == SJ6_LAYER1_FINE || slot == SJ6_LAYER2_FINE)
        return (int)juno6_layer_fine(&original_bass[owner].dsp.juno.synth, slot - SJ6_LAYER1_FINE);
    if (slot == SJ6_LAYER1_SEMI || slot == SJ6_LAYER2_SEMI)
        return (int)juno6_layer_semi(&original_bass[owner].dsp.juno.synth, slot - SJ6_LAYER1_SEMI);
    if (slot == SJ6_A_REFERENCE) {
        float raw;
        if (dnbx_read_raw(&original_bass[owner].dsp.juno.synth.core, SJ6_A_REFERENCE_ID, &raw) != DNBX_OK)
            return SJ6_A_REFERENCE_DEFAULT;
        return (int)(raw * SJ6_A_REFERENCE_STEPS + .5f);
    }
    if (slot >= SJ6_VIBRATO_DEPTH) {
        float raw;
        unsigned id = 105u + slot - SJ6_VIBRATO_DEPTH;
        if (dnbx_read_raw(&original_bass[owner].dsp.juno.synth.core, id, &raw) != DNBX_OK) return 0;
        return (int)(raw * 127.f + .5f);
    }
    if (slot >= SJ6_CHORUS_RATE)
        return mot_base_snd ? original_sound_panel_position(mot_base_snd->v[T_303][owner],13u + slot - SJ6_CHORUS_RATE) : 0;
    if (slot >= SJ6_CHORUS_I) {
        unsigned mode = original_bass[owner].dsp.juno.chorus.factory_mode;
        return mode ? ((mode - JUNO6_CHORUS_OFF) >> (slot - SJ6_CHORUS_I)) & 1u : 0;
    }
    if (slot >= SJ6_OSC_KNOBS && slot < SJ6_LFO2_PWM_DEPTH && mot_base_snd &&
#if X0X_BASS_NOTE_SOUND
        !note_lock_applied[owner] &&
#endif
        (original_sound_edit_mask(mot_base_snd->v[T_303][owner]) & ORIGINAL_SJ6_EXTRA_BIT)) {
        const uint8_t *extra = mot_base_snd->v[T_303][ORIGINAL_SJ6_EXTRA_ROW + owner];
        unsigned index = slot - SJ6_OSC_KNOBS;
        if (original_sound_sj6_extra_mask(extra) & (1u << index))
            return extra[2 + index] & (slot == SJ6_FILTER_LINK ? 1u :
                   slot <= SJ6_FILTER_ATTACK || slot >= SJ6_LFO1_RATE ? 127u : 255u);
    }
    return (int)juno6_knob_position(&original_bass[owner].dsp.juno.synth, slot);
}

/* Use the pending route, so a base edit following a route turn in the
 * same audio block still belongs to the selected oscillator. */
static unsigned sj6_panel_slot(unsigned owner, unsigned slot)
{
    if (slot != SJ6_LFO2_PWM_BASE) return slot;
    return sj6_knob_position(owner, SJ6_LFO2_PWM_ROUTE) == 2 ? SJ6_PWM2 : SJ6_PWM1;
}

#if defined(X0X_HOST) && defined(X0X_SJ6_DELAY_WITNESS)
/* Only the disposable typed app registers these independent float lines. */
#ifndef X0X_SJ6_DELAY_CAP
#define X0X_SJ6_DELAY_CAP 66151u
#endif
enum { SJ6_DELAY_CAP=X0X_SJ6_DELAY_CAP };
static float sj6_delay_host_lines[NBASS][2][SJ6_DELAY_CAP];
static unsigned sj6_delay_byte(unsigned slot)
{
    switch(slot){
    case SJ6_DELAY24:case SJ6_DELAYSYNC:return 0; case SJ6_DELAY25:return 1;
    case SJ6_DELAY26:return 2; default:return 3;
    }
}
static unsigned sj6_delay_read(unsigned slot,const uint8_t *b)
{
    if(slot==SJ6_DELAY24)return !!(b[0]&SJ6_DELAY_ENABLE_BIT);
    if(slot==SJ6_DELAYSYNC)return !!(b[0]&SJ6_DELAY_SYNC_BIT);
    return b[sj6_delay_byte(slot)];
}
int engine_superj6_delay_clock_refused(unsigned v)
{
    if(v>=NBASS)return 0;
    uint32_t lock=plat_cq_lock();
    int refused=live137(mot_base_snd,v)&&original_bass[v].kind==ORIGINAL_SOUND_JUNO&&
        original_bass[v].dsp.juno.synth.delay24.clock_refused;
    plat_cq_unlock(lock);return refused;
}
static void sj6_typed_delay_apply(unsigned v,float bpm)
{
    if(!live137(mot_base_snd,v)||original_bass[v].kind!=ORIGINAL_SOUND_JUNO)return;
    juno6_t *s=&original_bass[v].dsp.juno.synth;
    const uint8_t *bytes=read137(mot_base_snd,v)+SJ6_DELAY_BYTE;
    if(!sj6_delay_bytes_valid(bytes))return;
    if(!s->delay24.tap){
        memset(sj6_delay_host_lines[v],0,sizeof sj6_delay_host_lines[v]);
        if(!juno6_delay24_arm(s,sj6_delay_host_lines[v][0],sj6_delay_host_lines[v][1],SJ6_DELAY_CAP,0.f,10000u))return;
    }
    /* A live tempo or reopen may make the accepted project time too long.
     * Fail dry without rewriting the recorded mode/value; continue history. */
    s->delay24.clock_refused=!juno6_delay25_profile(s,bytes[1],bpm,!!(bytes[0]&SJ6_DELAY_SYNC_BIT));
    for(unsigned slot=SJ6_DELAY24;slot<=SJ6_DELAY26;++slot){
        if(slot==SJ6_DELAY25)continue;
        unsigned raw=sj6_delay_read(slot,bytes);
        if(raw!=juno6_delay_position(s,slot))(void)juno6_delay24_set(s,slot,raw);
    }
}
#endif
X0X_COLD int engine_superj6_knob_position(unsigned owner, unsigned slot)
{
#if defined(X0X_HOST) && defined(X0X_SJ6_DELAY_WITNESS)
    if(owner<NBASS && slot>=SJ6_DELAY24 && slot<=SJ6_DELAYSYNC){
        uint32_t lock=plat_cq_lock();
        int ret=live137(mot_base_snd,owner) && sj6_knob_ready(owner) &&
            original_bass[owner].dsp.juno.synth.delay24.tap ?
            (int)sj6_delay_read(slot,read137(mot_base_snd,owner)+SJ6_DELAY_BYTE) : -1;
        plat_cq_unlock(lock);return ret;
    }
#endif
    if (owner >= NBASS || slot > SJ6_LFO2_PWM_BASE) return -1;
    uint32_t lock = plat_cq_lock();
    int position = sj6_knob_ready(owner) ? sj6_knob_position(owner, sj6_panel_slot(owner, slot)) : -1;
    plat_cq_unlock(lock);
    return position;
}

static int sj6_knob_can_turn(unsigned owner)
{
    return !project_hold && mot_base_snd && sj6_knob_ready(owner) && !engine_bass_preset_pending(owner)
#if X0X_BASS_NOTE_SOUND
        && !note_lock_applied[owner]
#endif
        ;
}

static int sj6_knob_target(int before, int value, int max, int relative)
{
    if (relative) {
        if (value < -before) return 0;
        if (value > max - before) return max;
        return before + value;
    }
    return value < 0 ? 0 : value > max ? max : value;
}

static int sj6_semi_storage_ready(unsigned owner)
{
    return original_sound_sj6_semi_ready(mot_base_snd->v[T_303][owner],
                                         mot_base_snd->v[T_303][ORIGINAL_SJ6_EXTRA_ROW + owner]);
}

X0X_COLD int engine_superj6_knob_turn(unsigned owner, unsigned slot, int value, int relative)
{
#if defined(X0X_HOST) && defined(X0X_SJ6_DELAY_WITNESS)
    if(owner<NBASS && slot>=SJ6_DELAY24 && slot<=SJ6_DELAYSYNC){
        uint32_t lock=plat_cq_lock();int changed=0;
        if(live137(mot_base_snd,owner) && sj6_knob_can_turn(owner) &&
           !ui_project_busy() && s1_typed_tuple_editable() &&
           original_bass[owner].dsp.juno.synth.delay24.tap){
            uint8_t *b=slot137((sound_t *)mot_base_snd,owner)+SJ6_DELAY_BYTE;
            unsigned before=sj6_delay_read(slot,b);
            unsigned bit=slot==SJ6_DELAY24?SJ6_DELAY_ENABLE_BIT:slot==SJ6_DELAYSYNC?SJ6_DELAY_SYNC_BIT:0;
            unsigned next=(unsigned)sj6_knob_target((int)before,value,bit?1:127,relative);
            unsigned sync=slot==SJ6_DELAYSYNC?next:!!(b[0]&SJ6_DELAY_SYNC_BIT);
            unsigned raw=slot==SJ6_DELAY25?next:b[1];
            if(next!=before && ((slot!=SJ6_DELAYSYNC&&slot!=SJ6_DELAY25) ||
                juno6_delay25_can(&original_bass[owner].dsp.juno.synth,raw,seq_tempo(&seq),sync))){
                if(bit)b[0]=(uint8_t)((b[0]&~bit)|(next?bit:0));
                else b[sj6_delay_byte(slot)]=(uint8_t)next;
                changed=1;
            }

        }
        plat_cq_unlock(lock);return changed;
    }
#endif
    if (owner >= NBASS || slot > SJ6_LFO2_PWM_BASE) return 0;
    uint32_t lock = plat_cq_lock();
    int changed = 0;
    if (sj6_knob_can_turn(owner) && ((slot != SJ6_OSC2_LEVEL && slot != SJ6_OSC1_LEVEL) || bass_named_preset[owner] == 0)) {
        slot = sj6_panel_slot(owner, slot);
        int before = sj6_knob_position(owner, slot);
#if (defined(X0X_HOST) && defined(X0X_SJ6_FILTER_VEL_PROBE)) || (DNBOX_TYPED_CONTROLS_NATIVE_TEST && X0X_SJ6_VELOCITY_PORT)
        if (slot == SJ6_FILTER_VELOCITY && before < 0) { plat_cq_unlock(lock); return 0; }
#endif
        value = sj6_knob_target(before, value, (int)original_sound_sj6_max(slot), relative);
        if (value != before && (slot < SJ6_VIBRATO_DEPTH || slot == SJ6_LFO2_SHAPE ||
            (slot >= SJ6_LAYER1_FINE ? original_sound_valid(mot_base_snd->v[T_303][owner]) :
             slot >= SJ6_LAYER1_SEMI ? sj6_semi_storage_ready(owner) :
             sj6_wire_storage_ready(owner, slot != SJ6_A_REFERENCE, slot == SJ6_A_REFERENCE)))) {
            sj6_knob_post(owner, slot, value);
            changed = 1;
        }
    }
    plat_cq_unlock(lock);
    return changed;
}

#if defined(X0X_HOST) && defined(X0X_SJ6_MAILBOX_PROBE)
/* Exercise the upper word through the same mailbox, without assigning a
 * panel, DSP, or project field. The PWM BASE panel alias stays separate. */
static int sj6_knob_probe_slot(unsigned owner, unsigned slot)
{ return owner < NBASS && slot >= SJ6_KNOBS && slot < SJ6_MAILBOX_SLOTS; }

int engine_superj6_mailbox_probe_turn(unsigned owner, unsigned slot, int value, int relative)
{
    if (!sj6_knob_probe_slot(owner, slot)) return 0;
    uint32_t lock = plat_cq_lock();
    int changed = 0;
    if (sj6_knob_can_turn(owner)) {
        int before = sj6_knob_position(owner, slot);
        value = sj6_knob_target(before, value, UINT8_MAX, relative);
        if (value != before) {
            sj6_knob_post(owner, slot, value);
            changed = 1;
        }
    }
    plat_cq_unlock(lock);
    return changed;
}

int engine_superj6_mailbox_probe_position(unsigned owner, unsigned slot)
{
    if (!sj6_knob_probe_slot(owner, slot)) return -1;
    uint32_t lock = plat_cq_lock();
    int position = sj6_knob_ready(owner) ? sj6_knob_position(owner, slot) : -1;
    plat_cq_unlock(lock);
    return position;
}
#endif

X0X_COLD int engine_superj6_a_reference(unsigned owner, float *hz)
{
    if (owner >= NBASS || !hz) return 0;
    uint32_t lock = plat_cq_lock();
    int ready = sj6_knob_ready(owner);
    if (ready) {
        float raw = .6f;
        if (sj6_knob_has(owner, SJ6_A_REFERENCE))
            raw = juno6_a_reference_unit(sj6_knob_value[owner][SJ6_A_REFERENCE]);
        else
            (void)dnbx_read_raw(&original_bass[owner].dsp.juno.synth.core, SJ6_A_REFERENCE_ID, &raw);
        *hz = juno6_a_reference_hz(raw);
    }
    plat_cq_unlock(lock);
    return ready;
}

X0X_COLD int engine_superj6_panel_envelope(unsigned owner, unsigned filter,
                                                juno6_envelope_snapshot *out, unsigned *linked)
{
    if (owner >= NBASS || filter > 1u || !out || !linked) return 0;
    uint32_t lock = plat_cq_lock();
    int ready = sj6_knob_ready(owner) &&
        juno6_read_panel_envelope(&original_bass[owner].dsp.juno.synth, filter, out, linked);
    plat_cq_unlock(lock);
    return ready;
}

X0X_COLD int engine_superj6_panel_master(unsigned owner, float *gain)
{
    if (owner >= NBASS || !gain) return 0;
    uint32_t lock = plat_cq_lock();
    int ready = sj6_knob_ready(owner);
    if (ready) *gain = original_bass[owner].dsp.juno.synth.master_gain;
    plat_cq_unlock(lock);
    return ready;
}

X0X_COLD int engine_superj6_lfo_view(unsigned owner, unsigned lfo, juno6_lfo_view *out)
{
    if (owner >= NBASS || lfo >= SJ6_LFO_COUNT || !out) return 0;
    uint32_t lock = plat_cq_lock();
    int ready = sj6_knob_ready(owner);
    if (ready) juno6_read_lfo(&original_bass[owner].dsp.juno.synth, lfo, out);
    plat_cq_unlock(lock);
    return ready;
}

/* Keep the original low-word DSP and persistence order before shape. */
__attribute__((noinline)) X0X_COLD static void sj6_knob_commit(unsigned v, uint32_t mask, unsigned upper)
{
    sound_t *sound = (sound_t *)mot_base_snd;
    juno6_t *s = &original_bass[v].dsp.juno.synth;
    uint8_t *cell = sound->v[T_303][v];
    unsigned oscillator = mask & ORIGINAL_SJ6_EDIT_MASK;
    unsigned extra = (mask >> SJ6_OSC_KNOBS) & ((1u << SJ6_EXTRA_COUNT) - 1u);
    if (oscillator) {
        juno6_apply_knobs(s, oscillator, sj6_knob_value[v]);
        original_sound_sj6_store(cell, oscillator, sj6_knob_value[v]);
    }
    unsigned mode_turns = (mask >> SJ6_LFO1_MODE) & ((1u << SJ6_LFO_COUNT) - 1u);
    if (extra || mode_turns) {
        const uint8_t *positions = sj6_knob_value[v];
        unsigned modes = mode_turns | (extra >> ORIGINAL_SJ6_RATE_INDEX), hz_modes = 0;
        for (unsigned i = 0; i < SJ6_LFO_COUNT; ++i) if (modes & (1u << i)) {
            unsigned slot = SJ6_LFO1_MODE + i;
            unsigned hz = mode_turns & (1u << i) ? positions[slot] : juno6_knob_position(s, slot);
            hz_modes |= hz << i;
        }
        uint8_t *row = sound->v[T_303][ORIGINAL_SJ6_EXTRA_ROW + v];
        original_sound_sj6_extra_store(cell, row, extra, positions + SJ6_OSC_KNOBS, modes, hz_modes);
        sj6_apply_extra(s, extra | (modes << ORIGINAL_SJ6_MODE_INDEX), row + 2, 0);
    }
    unsigned depth = mask & (1u << SJ6_LFO2_PWM_DEPTH), route = mask & (1u << SJ6_LFO2_PWM_ROUTE);
    if (depth || route) {
        unsigned setting = original_sound_sj6_lfo2_pwm(cell);
        if (depth) setting = (setting & ~SJ6_LFO2_PWM_DEPTH_MASK) | sj6_knob_value[v][SJ6_LFO2_PWM_DEPTH];
        if (route) setting = (setting & SJ6_LFO2_PWM_DEPTH_MASK) |
                             (sj6_knob_value[v][SJ6_LFO2_PWM_ROUTE] << SJ6_LFO2_PWM_ROUTE_SHIFT);
        juno6_set_lfo2_pwm(s, setting);
        original_sound_sj6_lfo2_pwm_store(cell, setting);
    }
    unsigned chorus_turns = (mask >> SJ6_CHORUS_I) & 15u;
    if (chorus_turns) {
        juno6_chorus_t *chorus = &original_bass[v].dsp.juno.chorus;
        unsigned prior = chorus->factory_mode;
        unsigned enabled = prior ? prior - JUNO6_CHORUS_OFF : 0;
        const uint8_t *positions = sj6_knob_value[v] + SJ6_CHORUS_I;
        for (unsigned i = 0; i < 2; ++i) {
            if (chorus_turns & (1u << i))
                enabled = (enabled & ~(1u << i)) | (positions[i] << i);
            if (chorus_turns & (4u << i))
                (void)original_sound_set(cell, 13u + i, positions[2u + i]);
        }
        chorus->factory_mode = (uint8_t)(JUNO6_CHORUS_OFF + enabled);
        original_sound_sj6_chorus_store(cell, chorus->factory_mode);
        if (!prior || chorus_turns >> 2) original_runtime_sj6_chorus(&original_bass[v], cell);
#if X0X_SJ6_CLASSIC_I_PROBE
        if(prior!=chorus->factory_mode && (chorus_turns&3u) &&
            live137(sound,v) && s1_typed_tuple_editable()) {
            if(chorus->factory_mode>=JUNO6_CHORUS_I) {
                /* One original active trio; new mode reapplies its defaults. */
                uint8_t *typed=slot137(sound,v)+SJ6_CLASSIC_I_BYTE;
                memset(typed,SJ6_CLASSIC_I_DEFAULT,SJ6_CLASSIC_I_COUNT);
            }
            sj6_classic_i_apply(sound,v);
        }
#endif
    }
    unsigned vibrato_turns = (mask >> SJ6_VIBRATO_DEPTH) & 3u;
    if (vibrato_turns && sj6_wire_storage_ready(v, 1u, 0u)) {
        uint8_t *extra = sound->v[T_303][ORIGINAL_SJ6_EXTRA_ROW + v];
        uint8_t *wire = sound->v[T_303][BASS_SHARED_EXTRA_ROW + v];
        float values[2] = {0};
        int present = sj6_load_vibrato(cell, extra, wire, values);
        if (present >= 0) {
            for (unsigned i = 0; i < 2; ++i)
                if (vibrato_turns & (1u << i))
                    values[i] = unit_control(sj6_knob_value[v][SJ6_VIBRATO_DEPTH + i]);
            present |= vibrato_turns;
            if (original_sound_sj6_vibrato_marker(cell) == 1u) memset(extra + ORIGINAL_SJ6_VIBRATO_OFFSET, 0, 2u);
            original_sound_sj6_vibrato_store(cell, wire, values, present);
            juno6_set_vibrato_words(s, values, present, 0);
        }
    }
    if ((mask & (1u << SJ6_A_REFERENCE)) && sj6_wire_storage_ready(v, 0u, 1u)) {
        uint8_t *wire = sound->v[T_303][BASS_SHARED_EXTRA_ROW + v];
        float raw = juno6_a_reference_unit(sj6_knob_value[v][SJ6_A_REFERENCE]);
        original_sound_sj6_a_reference_store(cell, wire, raw, 1u);
        juno6_set_a_reference(s, raw, 1u);
    }
    if (upper & sj6_knob_bit(SJ6_LFO2_SHAPE)) {
        unsigned shape = sj6_knob_value[v][SJ6_LFO2_SHAPE];
        juno6_set_lfo2_shape(s, shape);
        original_sound_sj6_shape_store(cell, shape);
    }
    for (unsigned l = 0; l < DNBX_LAYERS; ++l) {
        const unsigned slot = SJ6_LAYER1_SEMI + l;
        if (!(upper & sj6_knob_bit(slot)) || !sj6_semi_storage_ready(v)) continue;
        const unsigned position = sj6_knob_value[v][slot];
        original_sound_sj6_semi_store(cell, sound->v[T_303][ORIGINAL_SJ6_EXTRA_ROW + v], l, position);
        juno6_set_layer_semi(s, l, position);
    }
    for (unsigned l = 0; l < DNBX_LAYERS; ++l) {
        const unsigned slot = SJ6_LAYER1_FINE + l;
        if (!(upper & sj6_knob_bit(slot)) || !original_sound_valid(cell)) continue;
        const unsigned position = sj6_knob_value[v][slot];
        original_sound_sj6_fine_store(cell, l, position);
        juno6_set_layer_fine(s, l, position);
    }
    if ((upper & sj6_knob_bit(SJ6_OSC2_LEVEL)) && !original_sound_sj6_factory(cell)) {
        unsigned position=sj6_knob_value[v][SJ6_OSC2_LEVEL];
        cell[30]=(uint8_t)(128u | position);
        s->osc2_level_code=(uint8_t)(position+1u);
    }
    if ((upper & sj6_knob_bit(SJ6_OSC1_LEVEL)) && !original_sound_sj6_factory(cell)) {
        unsigned position=sj6_knob_value[v][SJ6_OSC1_LEVEL];
        original_sound_sj6_osc1_store(cell,position);
        s->osc1_level_code=(uint8_t)(position+1u);
    }
    bass_tag_set(sound, v, ORIGINAL_SOUND_JUNO, bass_named_preset[v],
                 original_sound_is_edited(cell));
}

X0X_COLD static void sj6_knob_boundary(void)
{
    for (unsigned v = 0; v < NBASS; ++v) {
        const sj6_knob_bits pending = {{sj6_knob_mask[v].word[0], sj6_knob_mask[v].word[1]}};
        if (!(pending.word[0] | pending.word[1])) continue;
        sj6_knob_clear(v);
        if (!mot_base_snd || !sj6_knob_ready(v) || engine_bass_preset_pending(v)) continue;
#if X0X_BASS_NOTE_SOUND
        if (note_lock_applied[v]) continue;
#endif
#if (defined(X0X_HOST) && defined(X0X_SJ6_FILTER_VEL_PROBE)) || (DNBOX_TYPED_CONTROLS_NATIVE_TEST && X0X_SJ6_VELOCITY_PORT)
        if (pending.word[1] & sj6_knob_bit(SJ6_FILTER_VELOCITY)) {
            unsigned pos=sj6_knob_value[v][SJ6_FILTER_VELOCITY];
            if (!bass_named_preset[v] && live137(mot_base_snd,v) && pos<=127 &&
                juno6_set_filter_velocity(&original_bass[v].dsp.juno.synth,pos))
                slot137((sound_t *)mot_base_snd,v)[2]=(uint8_t)pos;
        }
#endif
        unsigned upper = pending.word[1] & (sj6_knob_bit(SJ6_LFO2_SHAPE) | sj6_knob_bit(SJ6_LAYER1_SEMI) |
                                             sj6_knob_bit(SJ6_LAYER2_SEMI) | sj6_knob_bit(SJ6_LAYER1_FINE) |
                                             sj6_knob_bit(SJ6_LAYER2_FINE) | sj6_knob_bit(SJ6_OSC2_LEVEL) | sj6_knob_bit(SJ6_OSC1_LEVEL));
        if (pending.word[0] || upper) sj6_knob_commit(v, pending.word[0], upper);
#if defined(X0X_HOST) && defined(X0X_SJ6_MAILBOX_PROBE)
        for (unsigned slot = SJ6_KNOBS; slot < SJ6_MAILBOX_SLOTS; ++slot)
            if (pending.word[1] & sj6_knob_bit(slot))
                engine_superj6_mailbox_probe_apply(v, slot, sj6_knob_value[v][slot]);
#endif
    }
}
#endif

/* Compact Synth One uses the cell's extension bytes for its 10-bit preset.
 * Other engines and stock INIT retain the legacy byte and project encoding. */
static unsigned bass_cell_preset(const uint8_t *cell)
{
#if X0X_S1C
    if (cell[29] == ORIGINAL_SOUND_SYNTHONE) return original_sound_s1_program(cell);
#endif
#if X0X_SUPERJ6_BUILTIN_INIT
    if (cell[29] == ORIGINAL_SOUND_JUNO) return original_sound_sj6_factory(cell);
#endif
    return cell[30];
}
#if X0X_S1C
enum { BASS_REQ_PRESET_BITS = 10 };
#else
enum { BASS_REQ_PRESET_BITS = 7 };
#endif
#define BASS_REQ_PRESET_MASK ((1u << BASS_REQ_PRESET_BITS) - 1u)
#define BASS_REQ_SERIAL_SHIFT (BASS_REQ_PRESET_BITS + 2)
#define BASS_REQ_SERIAL_MASK ((1u << (31 - BASS_REQ_SERIAL_SHIFT)) - 1u)
static int bass_tagged_cell_validate(uint8_t *cell, bass_sound_tag_t *tag, int *bank)
{
    bass_sound_info_t info;
    int adopted = original_sound_adopt_legacy_cell(cell);
    if (!original_sound_valid(cell)) return -9;
    if (adopted) {
        bass_sound_info_t next = {{ORIGINAL_SOUND_SYNTHONE_ID, ORIGINAL_SOUND_SCHEMA,
            (uint8_t)original_sound_count(ORIGINAL_SOUND_SYNTHONE), BASS_IDENTITY_KNOWN}, 0, 0};
        if (bass_sound_tag_encode(tag, &next, BASS_SOUND_REGISTRY, 4) != BASS_IDENTITY_READY)
            return -9;
        info = next;
    } else if (bass_sound_tag_decode(&info, tag, BASS_SOUND_REGISTRY, 4) != BASS_IDENTITY_READY) {
        return -9;
    }
    *bank = cell[29];
    if (info.identity.engine != original_sound_engine_id((unsigned)*bank) ||
        info.preset != bass_cell_preset(cell) || info.flags != (original_sound_is_edited(cell) ? BASS_SOUND_EDITED : 0)) return -9;
    return 0;
}
int engine_bass_sound_bank(const sound_t *sound, int voice)
{
    int bank;
    sound_t *mut = (sound_t *)sound;
    if (!mut || (unsigned)voice >= NBASS ||
        bass_tagged_cell_validate(mut->v[T_303][voice], &mut->bass[voice], &bank)) return -1;
#if X0X_SUPERJ6_BUILTIN_INIT
    if (bank == ORIGINAL_SOUND_JUNO && !original_sound_sj6_ext_valid(
            mut->v[T_303][voice], mut->v[T_303][ORIGINAL_SJ6_EXTRA_ROW + voice])) return -1;
#endif
#if X0X_S1_EXTRA || X0X_SUPERJ6_BUILTIN_INIT
    if (original_sound_bass_shared_size(mut->v[T_303][voice], mut->v[T_303][BASS_SHARED_EXTRA_ROW + voice]) < 0)
        return -1;
#endif
    return bank;
}
X0X_COLD int engine_bass_sound_validate(const sound_t *sound)
{
    if (!sound) return -1;
    for (int v = 0; v < NBASS; ++v) if (engine_bass_sound_bank(sound, v) < 0) return -9;
    return 0;
}
#if X0X_BASS_NOTE_SOUND
/* Per-note bass sound lock (owner 2026-11-12 02:43, 03:12). Capture is ported
 * from 279df014c/76f9f88fd (deleted with bass303 in 18481a63, last tree c4ecd248),
 * adapted to the original Chicago/Miami cell. Playback is new: the locked cell
 * and part mixer are applied at that note's onset, before the key. Capture runs
 * only while NOTE LOCK is on (runtime, not saved); with no lock records the
 * onset path does one flag read. */
static volatile uint8_t note_lock_capture, note_lock_any;
void engine_note_lock_capture(int on) { note_lock_capture = (uint8_t)(on != 0); }
int engine_note_lock_capture_on(void) { return note_lock_capture; }
int engine_bass_note_validate(const lane_t *l);
int engine_bass_note_validate(const lane_t *l)
{
#ifdef DNBOX_137_HOST_WITNESS
    if(l->i==NOTE_S1_MOD) {
        if(l->val[BASS_NOTE_TAG+11]>2 || !s1_typed_init_cell(l->val))return -1;
        lane_t v1=*l;
        v1.i=BASS_NOTE_VERSION;
        v1.val[BASS_NOTE_TAG+11]=0;
        return engine_bass_note_validate(&v1);
    }
    if(l->i==NOTE137) {
        unsigned x=l->val[BASS_NOTE_TAG+11];
#if DNBOX_TYPED_CONTROLS_NATIVE_TEST
        if(x)return -1; /* do not pretend the filtered velocity lock applied */
#endif
        if(x>127 || l->val[29]!=ORIGINAL_SOUND_JUNO || original_sound_sj6_factory(l->val)!=0)return -1;
        lane_t v1=*l;
        v1.i=BASS_NOTE_VERSION;
        v1.val[BASS_NOTE_TAG+11]=0;
        return engine_bass_note_validate(&v1);
    }
#endif
    int bank;
    lane_t *mut=(lane_t *)l;
    if(l->used!=1 || l->t!=BASS_NOTE_TARGET || l->pat>=NPAT ||
       l->part<PART_303A || l->part>PART_303B || l->v>=NSTEPS ||
       l->i!=BASS_NOTE_VERSION || l->rsv)return -1;
    int rc=bass_tagged_cell_validate(mut->val,(bass_sound_tag_t *)(mut->val+BASS_NOTE_TAG),&bank);
    if(rc)return rc;
    unsigned used = BASS_NOTE_USED;
#if X0X_S1_EXTRA
    _Static_assert(BASS_NOTE_USED + S1_EXTRA_BYTES <= NSTEPS, "Synth One note-lock extension fits");
    if (bank == ORIGINAL_SOUND_SYNTHONE && (original_sound_edit_mask(l->val) & S1_EXTRA_MARK)) {
        if (!original_sound_s1_extra_valid(l->val, l->val+BASS_NOTE_USED)) return -1;
        used += s1_extra_size(l->val+BASS_NOTE_USED);
    }
#endif
#if X0X_SUPERJ6_BUILTIN_INIT
    _Static_assert(BASS_NOTE_USED + ORIGINAL_SJ6_EXTRA_SIZE <= NSTEPS, "SuperJ6 note-lock extension fits");
    if (bank == ORIGINAL_SOUND_JUNO && (original_sound_edit_mask(l->val) & ORIGINAL_SJ6_AUTHORED_LFO_BIT))
        return -1; /* The authored plane belongs to the selected base sound. */
    if (bank == ORIGINAL_SOUND_JUNO && (original_sound_edit_mask(l->val) & ORIGINAL_SJ6_EXTRA_BIT)) {
        if (!original_sound_sj6_extra_valid(l->val + BASS_NOTE_USED)) return -1;
        used += ORIGINAL_SJ6_EXTRA_SIZE;
    }
#endif
    for (unsigned i = used; i < NSTEPS; ++i) if (l->val[i]) return -1;
    for(int i=0;i<MX_NPARAMS;i++)if(l->val[BASS_NOTE_MIX+i]>engine_param(T_MIX,l->part,i)->max)return -1;
    return 0;
}
static int bass_note_capture(lane_t *lanes,int pat,int voice,int step,const sound_t *sound,int effective)
{
    if(!lanes || sound!=mot_base_snd || (unsigned)pat>=NPAT ||
       (unsigned)voice>=NBASS || (unsigned)step>=NSTEPS || engine_bass_sound_bank(sound,voice)<0)return -1;
    int k=bass_note_find(lanes,pat,voice,step);
    if(k<0)for(int j=0;j<NLANE;j++)if(!lanes[j].used){k=j;break;}
    if(k<0)return -7;
    lane_t next={0};
    next.used=1;next.gen=(uint8_t)(lanes[k].gen+1);next.t=BASS_NOTE_TARGET;
    next.pat=(uint8_t)pat;next.part=(uint8_t)(PART_303A+voice);next.v=(uint8_t)step;next.i=BASS_NOTE_VERSION;
    __builtin_memcpy(next.val,sound->v[T_303][voice],NPARAMS_MAX);
#if X0X_S1_EXTRA
    if (original_sound_s1_has_extra(next.val)) {
        const uint8_t *extra = sound->v[T_303][S1_EXTRA_ROW+voice];
        memcpy(next.val+BASS_NOTE_USED, extra, s1_extra_size(extra));
    }
#endif
#if X0X_SUPERJ6_BUILTIN_INIT
    if (next.val[29] == ORIGINAL_SOUND_JUNO) {
        next.val[25] &= (uint8_t)~(ORIGINAL_SJ6_AUTHORED_LFO_BIT >> 24);
        if (original_sound_edit_mask(next.val) & ORIGINAL_SJ6_EXTRA_BIT)
            memcpy(next.val + BASS_NOTE_USED, sound->v[T_303][ORIGINAL_SJ6_EXTRA_ROW + voice],
                   ORIGINAL_SJ6_EXTRA_SIZE);
    }
#endif
    __builtin_memcpy(next.val+BASS_NOTE_MIX,sound->v[T_MIX][PART_303A+voice],MX_NPARAMS);
    if(effective) {
        /* Audio owns motion runtime. Capture what this accepted edge hears,
         * including an in-flight ramp, without rewriting the selected base. */
        for(int j=0;j<NLANE;j++) {
            const lane_t *l=&mot.lane[j];
            int value=motion_value(&mot,j);
            if(value<0 || l->part!=PART_303A+voice)continue;
            if(l->t==T_303 && l->v==voice && l->i<NPARAMS_MAX)
                (void)original_sound_set(next.val,l->i,(unsigned)value);
            if(l->t==T_MIX && l->v==PART_303A+voice && l->i<MX_NPARAMS)
                next.val[BASS_NOTE_MIX+l->i]=(uint8_t)value;
        }
    }
    {   /* the tag describes the captured cell (bass_tag_set, into the record) */
        const uint8_t *cell=next.val;
        bass_sound_info_t info = {{original_sound_engine_id((unsigned)cell[29]), ORIGINAL_SOUND_SCHEMA,
            (uint8_t)original_sound_count((unsigned)cell[29]), BASS_IDENTITY_KNOWN},
            (uint16_t)bass_cell_preset(cell), (uint8_t)(original_sound_is_edited(cell) ? BASS_SOUND_EDITED : 0)};
        if(bass_sound_tag_encode((bass_sound_tag_t *)(next.val+BASS_NOTE_TAG),&info,
                                BASS_SOUND_REGISTRY,4)!=BASS_IDENTITY_READY)return -1;
    }
#ifdef DNBOX_137_HOST_WITNESS
    if(sound==&proj.sound && proj.set.snd_rev==REV137 &&
       next.val[29]==ORIGINAL_SOUND_JUNO && original_sound_sj6_factory(next.val)==0) {
        if(!canonical137(sound))return -1;
        next.i=NOTE137;
        next.val[BASS_NOTE_TAG+11]=effective && note_lock_applied[voice] ?
            original_bass[voice].dsp.juno.synth.filter_velocity_position : read137(sound,voice)[2];
    }
    if(sound==&proj.sound && proj.set.snd_rev==REV137 && s1_typed_init_cell(next.val)) {
        if(!canonical137(sound))return -1;
        next.i=NOTE_S1_MOD;
        next.val[BASS_NOTE_TAG+11]=effective && note_lock_applied[voice] ?
            original_bass[voice].dsp.synthone.mod_wheel_route : read137(sound,voice)[3];
    }
#endif
    if(engine_bass_note_validate(&next))return -1;
    lanes[k].used=0;
    __asm__ volatile("" ::: "memory");
    __builtin_memcpy((uint8_t *)&lanes[k]+1,(const uint8_t *)&next+1,sizeof next-1);
    __asm__ volatile("" ::: "memory");
    lanes[k].used=1;
    return 0;
}
/* 0 captured, 1 NOTE LOCK is off (nothing stored), <0 refused (pool full / sound). */
int engine_bass_note_capture(lane_t *lanes,int pat,int voice,int step,const sound_t *sound)
{return note_lock_capture?bass_note_capture(lanes,pat,voice,step,sound,0):1;}
/* Sequencer calls these only for actual recorder writes, before changing notes.
 * A refusal affects storage only: the accepted live note still auditions. */
int seq_bass_sound_prepare(seq_t *s,int voice,int pat,int step)
{
    if(!note_lock_capture)return 1;
    if(s!=&seq || !mot.lane)return -1;
    return bass_note_capture(mot.lane,pat,voice,step,mot_base_snd,1);
}
void seq_bass_sound_clear(seq_t *s,int voice,int pat,int step)
{if(s==&seq && mot.lane)bass_note_clear(mot.lane,pat,voice,step);}
/* Main loop: whether any lock record exists (the onset path reads only this). */
void engine_note_lock_refresh(void)
{
    uint8_t any=0;
    if(mot.lane)for(int k=0;k<NLANE;k++)if(mot.lane[k].used && mot.lane[k].t==BASS_NOTE_TARGET){any=1;break;}
    note_lock_any=any;
}
#if X0X_SUPERJ6_BUILTIN_INIT
static uint8_t sj6_locked_extra[NBASS];
static void sj6_note_patch(int v, const lane_t *lock)
{
    original_runtime_part *part = &original_bass[v];
    if (part->kind != ORIGINAL_SOUND_JUNO) return;
    const uint8_t *base = mot_base_snd->v[T_303][v];
    const uint8_t *cell = lock ? lock->val : base;
#if defined(DNBOX_137_HOST_WITNESS) && (!DNBOX_TYPED_CONTROLS_NATIVE_TEST || X0X_SJ6_VELOCITY_PORT)
    if (live137(mot_base_snd,v)) {
        unsigned position=lock && lock->i==NOTE137 ? lock->val[BASS_NOTE_TAG+11] : read137(mot_base_snd,v)[2];
        juno6_set_filter_velocity(&part->dsp.juno.synth,position);
    }
#endif
    const uint8_t *extra = lock ? lock->val + BASS_NOTE_USED :
                          mot_base_snd->v[T_303][ORIGINAL_SJ6_EXTRA_ROW + v];
    const uint8_t *base_extra = mot_base_snd->v[T_303][ORIGINAL_SJ6_EXTRA_ROW + v];
    const unsigned marked = (original_sound_edit_mask(cell) & ORIGINAL_SJ6_EXTRA_BIT) &&
                            original_sound_sj6_extra_mask(extra);
    if (marked || sj6_locked_extra[v] || ((original_sound_edit_mask(base) & ORIGINAL_SJ6_EXTRA_BIT) &&
                                         original_sound_sj6_extra_mask(base_extra))) {
        sj6_restore_authored_lfo(part, base, mot_base_snd->v[T_303][ORIGINAL_SJ6_EXTRA_ROW + v]);
        sj6_restore_extra(part, cell, extra, 1);
    }
    sj6_restore_semi(part, cell, extra);
    part->dsp.juno.synth.osc2_level_code=(uint8_t)original_sound_sj6_osc2_code(cell);
    part->dsp.juno.synth.osc1_level_code=(uint8_t)original_sound_sj6_osc1_code(cell);
    sj6_locked_extra[v] = (uint8_t)(lock && marked);
    uint8_t knobs[SJ6_OSC_KNOBS];
    unsigned mask = original_sound_sj6_knobs(cell, knobs);
    if (mask) juno6_apply_knobs(&part->dsp.juno.synth, mask, knobs);
    juno6_set_lfo2_pwm(&part->dsp.juno.synth, original_sound_sj6_lfo2_pwm(cell));
    juno6_set_lfo2_shape(&part->dsp.juno.synth, original_sound_sj6_shape(cell));
    part->dsp.juno.chorus.factory_mode = (uint8_t)original_sound_sj6_chorus(cell);
}
#endif
static void note_lock_restore(int v)
{
    unsigned n=original_sound_count(original_bass[v].kind);
    note_lock_applied[v]=0;
#if X0X_SYNTHONE_PATCH
    if (original_bass[v].kind == ORIGINAL_SOUND_SYNTHONE) {
        const uint8_t *cell = mot_base_snd->v[T_303][v];
        s1_part_apply_cell(&original_bass[v].dsp.synthone, cell);
#ifdef DNBOX_137_HOST_WITNESS
        s1_part_mod_route(&original_bass[v].dsp.synthone,
                          live_s1_mod(mot_base_snd,v)?read137(mot_base_snd,v)[3]:0);
#endif
#if X0X_S1_EXTRA
        s1_part_apply_extra(&original_bass[v].dsp.synthone, original_sound_edit_mask(cell) & S1_EXTRA_MARK ?
                            mot_base_snd->v[T_303][S1_EXTRA_ROW+v] : NULL);
#endif
    }
#endif
    for(unsigned i=0;i<n;i++)if(!original_sound_is_performance(original_bass[v].kind,i))apply_base(T_303,v,(int)i);
#if X0X_SUPERJ6_BUILTIN_INIT
    sj6_note_patch(v, NULL);
#endif
    for(int i=0;i<MX_NPARAMS;i++)apply_base(T_MIX,PART_303A+v,i);
}
/* ISR, a bass onset (s_bon), before the key: the onset's locked sound, or the
 * knobs back after a locked onset. A lock for the other engine is skipped. */
static void note_lock_onset(int v)
{
    const lane_t *l=0;
    if(note_lock_any && mot.lane) {
        int k=bass_note_find(mot.lane,seq.ppat[TRK_BASS0+v],v,eng_step[TRK_BASS0+v]);
        if(k>=0 && mot.lane[k].val[29]==(uint8_t)original_bass[v].kind)l=&mot.lane[k];
    }
#ifdef DNBOX_137_HOST_WITNESS
    if (l && (l->i==NOTE_S1_MOD) != live_s1_mod(mot_base_snd,v)) l=0;
    if (l && ((l->i==NOTE137) != live137(mot_base_snd,v) ||
              (l->i==NOTE137 && l->val[BASS_NOTE_TAG+11]>127))) l=0;
#endif
    if(!l) { if(note_lock_applied[v])note_lock_restore(v); return; }
#if X0X_SUPERJ6_BUILTIN_INIT
    if (original_bass[v].kind == ORIGINAL_SOUND_JUNO) sj6_knob_clear((unsigned)v);
#endif
#if X0X_SYNTHONE_PATCH
    if (original_bass[v].kind == ORIGINAL_SOUND_SYNTHONE) {
        s1_part_apply_cell(&original_bass[v].dsp.synthone, l->val);
#ifdef DNBOX_137_HOST_WITNESS
        s1_part_mod_route(&original_bass[v].dsp.synthone,
                          l->i==NOTE_S1_MOD?l->val[BASS_NOTE_TAG+11]:0);
#endif
#if X0X_S1_EXTRA
        s1_part_apply_extra(&original_bass[v].dsp.synthone, original_sound_edit_mask(l->val) & S1_EXTRA_MARK ?
                            l->val+BASS_NOTE_USED : NULL);
#endif
    }
#endif
    unsigned n=original_sound_count(original_bass[v].kind);
    for(unsigned i=0;i<n;i++) {
        float value;
        if(original_sound_is_performance(original_bass[v].kind,i))continue;
#if X0X_SUPERJ6_BUILTIN_INIT
        if (i == ORIGINAL_SJ6_HPF_PANEL && original_sound_sj6_hpf_authored(l->val)) {
            juno6_set_sj6_hpf(&original_bass[v].dsp.juno.synth, unit_control(l->val[i] & 127u));
            continue;
        }
#endif
        if(original_sound_value(l->val,i,&value))(void)original_runtime_param(&original_bass[v],i,value);
    }
#if X0X_SUPERJ6_BUILTIN_INIT
    sj6_note_patch(v, l);
#endif
    for(int i=0;i<MX_NPARAMS;i++)apply_param(T_MIX,PART_303A+v,i,l->val[BASS_NOTE_MIX+i]);
    note_lock_applied[v]=1;
}
/* ISR, transport stop: no lock outlives playback. */
static void note_lock_stop(void)
{ for(int v=0;v<NBASS;v++)if(note_lock_applied[v])note_lock_restore(v); }
#endif
/* ISR, every sequenced bass onset (s_bon), before the key. */
static void bass_onset(int v)
{
    if (bass_gate_pat[v] < NPAT && !bass_edit_depth) bass_preset_boundary_voice(v);
#if X0X_BASS_NOTE_SOUND
    note_lock_onset(v);
#endif
}
int engine_bass_sound_adopt(sound_t *sound, int bank0, int bank1)
{ (void)sound; (void)bank0; (void)bank1; return -9; }
void engine_bass_sound_refresh(sound_t *sound, int v)
{
    if (!sound || (unsigned)v >= NBASS || !original_sound_valid(sound->v[T_303][v])) return;
    const uint8_t *cell = sound->v[T_303][v];
    bass_tag_set(sound, v, cell[29], bass_cell_preset(cell), original_sound_is_edited(cell));
}
X0X_COLD static int bass_restore_part(const sound_t *sound, int v)
{
    if (engine_bass_sound_bank(sound, v) < 0) return 0;
    if (original_runtime_retire(&original_bass[v]) != ORIGINAL_RUNTIME_OK) return 0;
    int rc = original_runtime_init(&original_bass[v], sound->v[T_303][v], seq_tempo(&seq),
        &original_tuning, &original_fx_owner[v]);
    original_switch_gain[v] = 512u; original_quiet_samples[v] = 0;
#if X0X_SYNTHONE_PATCH
    s1_patch_replaced(v);
#endif
#if X0X_SUPERJ6_BUILTIN_INIT
    sj6_knob_clear((unsigned)v);
#if X0X_BASS_NOTE_SOUND
    sj6_locked_extra[v] = 0;
#endif
    if (rc == ORIGINAL_RUNTIME_OK && original_bass[v].kind == ORIGINAL_SOUND_JUNO) {
        original_bass[v].dsp.juno.synth.lfo2_entropy_state ^= (uint64_t)(unsigned)v << 32;
#if defined(DNBOX_137_HOST_WITNESS) && (!DNBOX_TYPED_CONTROLS_NATIVE_TEST || X0X_SJ6_VELOCITY_PORT)
        if (live137(sound,v)) juno6_set_filter_velocity(&original_bass[v].dsp.juno.synth,read137(sound,v)[2]);
#endif
#if defined(X0X_SJ6_BEND_TYPED_PROBE) && X0X_SJ6_BEND_TYPED_PROBE
        (void)sj6_pb_apply(v,sound);
#endif
#if defined(X0X_SJ6_MOD_TYPED_PROBE)
        (void)sj6_typed_mod_apply(v,sound);
#endif
        const uint8_t *cell = sound->v[T_303][v];
        const uint8_t *extra = sound->v[T_303][ORIGINAL_SJ6_EXTRA_ROW + v];
        if (original_sound_edit_mask(cell) & ORIGINAL_SJ6_AUTHORED_LFO_BIT)
            sj6_restore_authored_lfo(&original_bass[v], cell, extra);
        sj6_restore_extra(&original_bass[v], cell, extra, 0);
        sj6_restore_semi(&original_bass[v], cell, extra);
        const uint8_t *wire = sound->v[T_303][BASS_SHARED_EXTRA_ROW + v];
        float values[2] = {0}, a_reference;
        int present = sj6_load_vibrato(cell, extra, wire, values);
        if (present > 0) juno6_set_vibrato_words(&original_bass[v].dsp.juno.synth, values, present, 1);
        if (original_sound_sj6_a_reference_word(cell, wire, &a_reference) > 0)
            juno6_set_a_reference(&original_bass[v].dsp.juno.synth, a_reference, 1u);
    }
#endif
#if X0X_S1_EXTRA
    if (rc == ORIGINAL_RUNTIME_OK && original_bass[v].kind == ORIGINAL_SOUND_SYNTHONE &&
        (original_sound_edit_mask(sound->v[T_303][v]) & S1_EXTRA_MARK))
        s1_part_apply_extra(&original_bass[v].dsp.synthone, sound->v[T_303][S1_EXTRA_ROW+v]);
#endif
#ifdef DNBOX_137_HOST_WITNESS
    if (rc==ORIGINAL_RUNTIME_OK && live_s1_mod(sound,v))
        s1_part_mod_route(&original_bass[v].dsp.synthone,read137(sound,v)[3]);
#if defined(X0X_S1_BEND_RANGE_PROBE) && X0X_S1_BEND_RANGE_PROBE
    if (rc==ORIGINAL_RUNTIME_OK) s1_pb_load(v,sound);
#endif
#if X0X_S1_DELAY_TRACK_TEST
    if(rc==ORIGINAL_RUNTIME_OK)s1_typed_dict_apply(sound,v);
#endif
    if(rc==ORIGINAL_RUNTIME_OK) {s1_typed_a4_apply(sound,v);s1_typed_rvfb_apply(sound,v);s1_typed_trans_apply(sound,v);s1_typed_crush_apply(sound,v);}
#endif
#if X0X_SJ6_CLASSIC_I_PROBE
    if(rc==ORIGINAL_RUNTIME_OK)sj6_classic_i_apply(sound,v);
#endif
#if defined(X0X_S1_FEEDBACK_PROBE) && X0X_S1_FEEDBACK_PROBE
    if(rc==ORIGINAL_RUNTIME_OK)s1_typed_feedback_apply(sound,v);
#endif
#if X0X_SJ6_MONO32_PROBE
    if(rc==ORIGINAL_RUNTIME_OK)sj6_voice_apply(sound,v);
#endif
    arp_adopt(v, sound, 0);
#if X0X_S1_FENCED_CLOCK
    tone_clock_active[v]=0;
    if(rc==ORIGINAL_RUNTIME_OK) {s1_typed_clock_apply(sound,v);s1_typed_clock_tones(sound,v);}
#endif
#if defined(X0X_S1_AUTOPAN_PROBE) && X0X_S1_AUTOPAN_PROBE
    if(rc==ORIGINAL_RUNTIME_OK)s1_typed_pan_apply(sound,v,1);
#endif
    bass_named_preset[v] = (int16_t)bass_cell_preset(sound->v[T_303][v]);
    original_seq_note[v] = -1;
    __builtin_memset(original_key_pitch[v], 0, sizeof original_key_pitch[v]);
    bass_held[v] = bass_held_count[v] = 0;
    for (unsigned s = 0; s < 3; ++s) src_held[s] &= (uint16_t)~(1u << v);
    return rc;
}
X0X_COLD static void bass_restore_sound(const sound_t *sound)
{
    for (int v = 0; v < NBASS; ++v) {
        live_key_context_retire(v, 0);
        (void)bass_restore_part(sound, v);
    }
}
int engine_bass_bank(int v) { return (unsigned)v < NBASS ? original_bass[v].kind : -1; }
int engine_bass_bank_available(int v, int bank)
{
    return (unsigned)v < NBASS && (bank == ORIGINAL_SOUND_JUNO || bank == ORIGINAL_SOUND_SYNTHONE || bank == FACTORY_BANK_MIAMI ||
        (bank == FACTORY_BANK_CHICAGO && original_fx_ready));
}
const x0x_param_t *engine_bass_bank_param(int bank, int parameter)
{ return original_sound_param((unsigned)bank, (unsigned)parameter); }
int engine_bass_preview_changed(void) { return bass_preview_dirty; }
void engine_bass_wheel_owners(uint8_t mask) { bass_wheel_mask = mask; }
int engine_bass_wheel_active(void) { return bass_wheel_mask != 0; }
int engine_bass_preset_index(int v) { return (unsigned)v < NBASS ? bass_named_preset[v] : -1; }
int engine_bass_preset_edited(int v)
{
    return (unsigned)v >= NBASS || !mot_base_snd ||
        original_sound_is_edited(mot_base_snd->v[T_303][v]);
}
uint32_t engine_bass_choice_serial(void) { return bass_request_serial; }
int engine_bass_preset_pending(int v)
{
    bass_request_t request;
    if ((unsigned)v >= NBASS) return 0;
    if (!bass_request_snapshot(v, &request)) return 6;
    if (!(request.token & 0x80000000u) || request.token == bass_applied[v]) return 0;
    if (!plat_midi_passed(request.midi) || (int32_t)(cq_r - request.cq) < 0) return 5;
    if (bass_has_locks(v)) return 4;
    if (bass_wheel_mask & (1u << v)) return 3;
    if (original_runtime_owned(&original_bass[v]) || original_quiet_samples[v] < 512u) return 2;
    return 1;
}
X0X_COLD int engine_bass_requested_bank(int v)
{ bass_request_t r; return (unsigned)v < NBASS && bass_request_snapshot(v, &r) && engine_bass_preset_pending(v) ? (int)((r.token >> BASS_REQ_PRESET_BITS) & 3u) : engine_bass_bank(v); }
X0X_COLD int engine_bass_requested_preset(int v)
{ bass_request_t r; return (unsigned)v < NBASS && bass_request_snapshot(v, &r) && engine_bass_preset_pending(v) ? (int)(r.token & BASS_REQ_PRESET_MASK) : engine_bass_preset_index(v); }
X0X_COLD void engine_bass_cancel_preset(int v)
{ if ((unsigned)v < NBASS) bass_publish_request(v, 0x40000000u | ((++bass_request_serial & 0x3fffffu) << 8)); }
void engine_bass_edit_hold(int hold)
{
    __asm__ volatile("" ::: "memory");
    if (hold) ++bass_edit_depth;
    else if (bass_edit_depth) --bass_edit_depth;
    __asm__ volatile("" ::: "memory");
}
X0X_COLD int engine_bass_base_ready(unsigned mask, const bass_base_state_t *target)
{
    for (int v = 0; v < NBASS; ++v) if (mask & (1u << v)) {
        int bank;
        uint8_t cell[ORIGINAL_SOUND_BYTES];
        bass_sound_tag_t tag = target->tag[v];
        memcpy(cell, target->cell[v], sizeof cell);
        if (bass_tagged_cell_validate(cell, &tag, &bank)) return -5;
#if X0X_SUPERJ6_BUILTIN_INIT
        if (bank == ORIGINAL_SOUND_JUNO && !original_sound_sj6_ext_valid(cell, target->sj6_extra[v]))
            return -5;
#endif
#if X0X_S1_EXTRA || X0X_SUPERJ6_BUILTIN_INIT
        if (original_sound_bass_shared_size(cell, target->shared_extra[v]) < 0) return -5;
#endif
        if (!engine_bass_bank_available(v, bank)) return -2;
        if (bass_has_locks(v)) return -4;
        /* Typed history restore retains its existing admission contract. Only
         * factory requests own the measured fade/quiescence transition. */
        if ((bass_wheel_mask & (1u << v)) || original_runtime_active(&original_bass[v])) return 2;
    }
    return cq_r != cq_w || live_key_r != live_key_w || live_key_cancel ||
        !plat_midi_passed(plat_midi_mark()) ? 2 : 0;
}
X0X_COLD void engine_bass_base_restore(unsigned mask)
{
    for (int v = 0; v < NBASS; ++v) if (mask & (1u << v)) {
        engine_bass_cancel_preset(v);
        live_key_context_retire(v, 0);
        (void)bass_restore_part(mot_base_snd, v);
        for (int i = 0; i < MX_NPARAMS; ++i) apply_base(T_MIX, PART_303A + v, i);
    }
}
X0X_COLD int engine_bass_select_preset_at(int v, int bank, int preset, sound_t *sound, int pat)
{
    if ((unsigned)v >= NBASS || !sound || sound != mot_base_snd ||
        (unsigned)preset >= original_sound_available_factories((unsigned)bank)) return -1;
    if (!engine_bass_bank_available(v, bank)) return -2;
    if (bass_has_locks(v)) return -4;
#if X0X_S1_EXTRA || X0X_SUPERJ6_BUILTIN_INIT
    if (original_sound_bass_shared_size(sound->v[T_303][v], sound->v[T_303][BASS_SHARED_EXTRA_ROW + v]) < 0)
        return -5;
#endif
    bass_gate_pat[v] = (uint8_t)((unsigned)pat < NPAT ? pat : 0xFF);
    __asm__ volatile("" ::: "memory");
    uint32_t token = 0x80000000u | ((++bass_request_serial & BASS_REQ_SERIAL_MASK) << BASS_REQ_SERIAL_SHIFT) |
        ((uint32_t)bank << BASS_REQ_PRESET_BITS) | (uint32_t)preset;
#if X0X_SYNTHONE_PATCH
    s1_edit[v].pending = 0;
#endif
    bass_preview_dirty = 1;
    bass_publish_request(v, token);
    return 0;
}
int engine_bass_select_preset(int v, int bank, int preset, sound_t *sound)
{ return engine_bass_select_preset_at(v, bank, preset, sound, -1); }
int engine_bass_preset_gate(int v) { return (unsigned)v < NBASS && bass_gate_pat[v] < NPAT ? bass_gate_pat[v] : -1; }
static void bass_preset_boundary_voice(int v)
{
    {
        bass_request_t request;
        if (!bass_request_snapshot(v, &request)) return;
        uint32_t token = request.token;
        if (!(token & 0x80000000u)) { bass_applied[v] = token; return; }
        if (engine_bass_preset_pending(v) != 1) return;
        /* A pattern preset waits until its pattern plays (the bar switch). */
        if (bass_gate_pat[v] < NPAT && seq.ppat[TRK_BASS0 + v] != bass_gate_pat[v]) return;
        int bank = (int)((token >> BASS_REQ_PRESET_BITS) & 3u), preset = (int)(token & BASS_REQ_PRESET_MASK);
        uint8_t next[32];
        if (!engine_bass_bank_available(v, bank) || !original_sound_init(next, bank, preset)) return;
        sound_t *sound = (sound_t *)mot_base_snd;
#if X0X_S1_EXTRA || X0X_SUPERJ6_BUILTIN_INIT
        uint8_t *shared = sound->v[T_303][BASS_SHARED_EXTRA_ROW + v];
        int owned = original_sound_bass_shared_size(sound->v[T_303][v], shared);
        if (owned < 0) { bass_applied[v] = token; return; }
#endif
        live_key_context_retire(v, 1);
        if (original_runtime_retire(&original_bass[v]) != ORIGINAL_RUNTIME_OK) {
            bass_applied[v] = token;
            return;
        }
        int rc = original_runtime_init(&original_bass[v], next, seq_tempo(&seq),
            &original_tuning, &original_fx_owner[v]);
        if (rc != ORIGINAL_RUNTIME_OK) {
            /* A failed adoption remains visible and needs a fresh explicit
             * request; do not rerun expensive initialization every block. */
            bass_applied[v] = token;
            return;
        }
        original_switch_gain[v] = 512u; original_quiet_samples[v] = 0;
#if X0X_SUPERJ6_BUILTIN_INIT
        if (original_bass[v].kind == ORIGINAL_SOUND_JUNO)
            original_bass[v].dsp.juno.synth.lfo2_entropy_state ^= (uint64_t)(unsigned)v << 32;
        if (sound->v[T_303][v][29] == ORIGINAL_SOUND_JUNO) {
            uint32_t sj6_flags = original_sound_edit_mask(sound->v[T_303][v]);
            uint8_t *sj6_extra = sound->v[T_303][ORIGINAL_SJ6_EXTRA_ROW + v];
            if (sj6_flags & ORIGINAL_SJ6_EXTRA_BIT) memset(sj6_extra, 0, ORIGINAL_SJ6_EXTRA_SIZE);
            if (sj6_flags & ORIGINAL_SJ6_AUTHORED_LFO_BIT)
                memset(sj6_extra + ORIGINAL_SJ6_EXTRA_SIZE, 0, ORIGINAL_SJ6_LFO_AUTHORED_SIZE);
            if (original_sound_sj6_vibrato_marker(sound->v[T_303][v]) == 1u)
                memset(sj6_extra + ORIGINAL_SJ6_VIBRATO_OFFSET, 0, 2u);
        }
#endif
#if X0X_S1_EXTRA || X0X_SUPERJ6_BUILTIN_INIT
        if (owned) memset(shared, 0, (unsigned)owned);
#endif
        __builtin_memcpy(sound->v[T_303][v], next, sizeof next);
        bass_tag_set(sound, v, bank, preset, 0);
#ifdef DNBOX_137_HOST_WITNESS
#if X0X_S1_DELAY_TRACK_TEST
        s1_typed_dict_apply(sound,v);
#endif
        s1_typed_a4_apply(sound,v);
        s1_typed_rvfb_apply(sound,v);
        s1_typed_trans_apply(sound,v);
        s1_typed_crush_apply(sound,v);
#if defined(X0X_S1_FEEDBACK_PROBE) && X0X_S1_FEEDBACK_PROBE
        s1_typed_feedback_apply(sound,v);
#endif
        if (live_s1_mod(sound,v))
            s1_part_mod_route(&original_bass[v].dsp.synthone,read137(sound,v)[3]);
#if defined(X0X_S1_BEND_RANGE_PROBE) && X0X_S1_BEND_RANGE_PROBE
        if (live_s1_mod(sound,v)) s1_pb_load(v,sound);
#endif
#endif
#if defined(DNBOX_137_HOST_WITNESS) && (!DNBOX_TYPED_CONTROLS_NATIVE_TEST || X0X_SJ6_VELOCITY_PORT)
        if (live137(sound,v)) juno6_set_filter_velocity(&original_bass[v].dsp.juno.synth,read137(sound,v)[2]);
#endif
#if defined(X0X_SJ6_BEND_TYPED_PROBE) && X0X_SJ6_BEND_TYPED_PROBE
        (void)sj6_pb_apply(v,sound);
#endif
#if defined(X0X_SJ6_MOD_TYPED_PROBE)
        (void)sj6_typed_mod_apply(v,sound);
#endif
#if X0X_SYNTHONE_PATCH
        s1_patch_replaced(v);
#endif
#if X0X_SUPERJ6_BUILTIN_INIT
        sj6_knob_clear((unsigned)v);
#if X0X_BASS_NOTE_SOUND
        sj6_locked_extra[v] = 0;
#endif
#endif
#if X0X_SJ6_CLASSIC_I_PROBE
        sj6_classic_i_apply(sound,v);
#endif
#if X0X_SJ6_MONO32_PROBE
        sj6_voice_apply(sound,v);
#endif
        arp_adopt(v, sound, 1);
#if X0X_S1_FENCED_CLOCK
        tone_clock_active[v]=0;
        s1_typed_clock_apply(sound,v);
        s1_typed_clock_tones(sound,v);
#endif
#if defined(X0X_S1_AUTOPAN_PROBE) && X0X_S1_AUTOPAN_PROBE
        s1_typed_pan_apply(sound,v,1);
#endif
        bass_named_preset[v] = (int16_t)preset;
        /* A committed bank/preset replacement, never merely a pending request.
         * parameter -1 identifies the bank; parameter -2 the factory index. */
        PARAM_TRACE(PARAM_PRESET, T_303, v, -1, bank);
        PARAM_TRACE(PARAM_PRESET, T_303, v, -2, preset);
        original_seq_note[v] = -1;
        (void)original_runtime_prepare(&original_bass[v], seq_tempo(&seq));
        __asm__ volatile("" ::: "memory");
        bass_applied[v] = token;
    }
}
static void bass_preset_boundary(void)
{
    if (bass_edit_depth) return;
    for (int v = 0; v < NBASS; ++v) bass_preset_boundary_voice(v);
}

