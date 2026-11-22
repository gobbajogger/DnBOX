/* SPDX-License-Identifier: GPL-3.0-only */
/* Test-only translation unit for controlled event boundaries and state capture. */
#include <assert.h>
#include <stdio.h>
#include "../../firmware/src/seq/sequencer.h"
#include "../../firmware/src/dsp/master.h"
static uint32_t test_until(seq_t *, uint32_t);
static void test_advance(seq_t *, uint32_t, const seq_sink_t *);
static void test_master(master_t *, float *, float *, int, float);
#define seq_until_event test_until
#define seq_advance test_advance
#define master_process_st test_master
#include "../../firmware/src/app/engine.c"
#undef seq_until_event
#undef seq_advance
#undef master_process_st

static unsigned prefix, change;

int mix_test_kits_complete(void)
{
    unsigned kits = (1u << JB_909) | (1u << JB_808);
    return eng_c1_next == JB_A && (eng_c1_mask & kits) == kits && (eng_c1_mask & ~kits) != 0;
}

int mix_test_preview_worker(void (*fn)(uint32_t)) { return fn == preview_master; }

extern void mix_test_master_checkpoint(void);
static void test_master(master_t *m, float *l, float *r, int n, float volume)
{
    master_process_st(m, l, r, n, volume);
    if (prefix && m != &mst) mix_test_master_checkpoint();
}

static uint32_t test_until(seq_t *s, uint32_t n)
{
    return prefix && n == 256 ? prefix : seq_until_event(s, n);
}

static void test_advance(seq_t *s, uint32_t n, const seq_sink_t *sink)
{
    seq_advance(s, n, sink);
    if (!prefix || n != 256 - prefix) return;
    if (change == 1) s_drum(0, 0, 0, .95f);              /* kick-keyed PUMP */
    if (change == 2) { mot_playing = 1; seq.playing = 0; } /* regular motion release after advance */
    if (change == 3) {                                    /* same final pots, new limiter history */
        master_set(&mst, MST_LIMIT, 0);
        master_set(&mst, MST_LIMIT, 1);
    }
}

void mix_test_prepare_preview(unsigned frames, unsigned final_edit)
{
    prefix = frames;
    change = final_edit;
    master_set(&mst, MST_COMP1, 95);
    master_set(&mst, MST_PUMP, 110);
    master_set(&mst, MST_PUMPSRC, PUMP_909);
    master_set(&mst, MST_LIMIT, 1);
    if (change == 1) mst.pump_tgt = 0.0f;
    if (change == 2) {
        master_set(&mst, MST_DJF, 11);
        mot.lane[0].used = 1; mot.lane[0].gen = 4;
        mot.lane[0].t = T_MST; mot.lane[0].v = 0; mot.lane[0].i = MST_DJF;
        mot.rt[0].gen = 4; mot.rt[0].applied = 11;
    }
}

int mix_test_write_preview(FILE *f)
{
    /* The serial whole-block GR meter, delay/reverb histories and exact wet
     * bus metadata are observable across subsequent callbacks. */
    fxbus_t capture = fx;
    const uint8_t seen = tape_signal_seen;
    const uint32_t at = tape_signal_at;
    capture.dbuf = NULL;
    assert(fwrite(&mst, sizeof mst, 1, f) == 1);
    assert(fwrite(&capture, sizeof capture, 1, f) == 1);
    assert(fwrite(dly_buf, sizeof dly_buf, 1, f) == 1);
    assert(fwrite(&d8_verb, sizeof d8_verb, 1, f) == 1);
    assert(fwrite(&bfx_flt_st, sizeof bfx_flt_st, 1, f) == 1);
    assert(fwrite(&seen, sizeof seen, 1, f) == 1);
    assert(fwrite(&at, sizeof at, 1, f) == 1);
    return eng_mastered == 256;
}

/* Equal measured costs make the previous two-kit mask optimal without the
 * attack rule. This distinguishes an actual constraint from an incidental split. */
unsigned mix_test_attack_plan(unsigned scenario)
{
    const uint32_t kits = (1u << JB_909) | (1u << JB_808);
    const uint32_t bass = (1u << JB_A) | (1u << JB_B);
    assert(original_runtime_owned(&original_bass[0]));
    assert(!!original_runtime_owned(&original_bass[1]) == (scenario != 3));
    memset(eng_bass_cost, 0, sizeof eng_bass_cost);
    for (unsigned j = 0; j < JB_N; j++) eng_job_cost[j] = 5000;
    eng_plan = kits;
    recent_kit_hits[0] = recent_kit_hits[1] = 0;

    for (unsigned kit = 0; kit < NKIT; kit++) {
        s_drum(NULL, kit, 0, .8f);
        s_drum(NULL, kit, 0, .8f);
    }
    assert(plan_pick(64) == kits); /* Retriggering one lane counts once. */
    for (unsigned kit = 0; kit < NKIT; kit++) s_drum(NULL, kit, 0, .8f);
    assert(plan_pick(16) == kits);
    for (unsigned kit = 0; kit < NKIT; kit++) s_drum(NULL, kit, 1, .8f);

    uint32_t result = plan_pick(64);
    if (scenario < 2) { /* Miami is the only eligible bass in either order. */
        assert((result & kits) && (result & kits) != kits);
        assert((result & bass) == (1u << (scenario ? JB_A : JB_B)));
    } else assert(result == kits); /* Two Chicago owners or a silent bass. */
    assert(!recent_kit_hits[0] && !recent_kit_hits[1]);
    eng_plan = kits;
    assert(plan_pick(256) == kits); /* The first long stretch consumed the hits. */
    for (unsigned kit = 0; kit < NKIT; kit++) {
        s_drum(NULL, kit, 0, .75f);
        s_drum(NULL, kit, 1, .9f);
    }
    return result;
}

unsigned mix_test_current_plan(void) { return eng_plan; }
