/* SPDX-License-Identifier: GPL-3.0-only */
/* Full sequencer, project store/load, engine and BREAK voice. A constant
 * ADPCM plateau gives each pinned slice the same signal for gain comparison. */
#define SMC_UI_MAIN inherited_smc_fixture
#include "smc_ui_test.c"

static uint8_t loop_data[4096];
static int32_t audio[512];

/* A player holds a BREAK step and turns K3 with SEQ held. The audio
 * measurements below use the project saved through that input path. */
static void turn_hit_velocity(int step, int delta)
{
    held_keys = 1u << WHITE_KEY[step];
    ++now_ms;
    input();
    CHECK(held_any_step() == step);
    held_btn = 1u << B_SEQ;
    ++now_ms;
    input();
    enc_acc[EN_ALGO] = delta;
    ++now_ms;
    input();
    held_btn = 0;
    ++now_ms;
    input();
    held_keys = 0;
    ++now_ms;
    input();
    CHECK(step_lane_has_note(PART_BRK, 0, step));
}

static unsigned peak_on_step(int pattern, int step)
{
    unsigned blocks = 0;
    while (seq.ppat[PART_BRK] != pattern || !seq.t[TRK_BRK].started ||
           seq.t[TRK_BRK].pos != step) {
        CHECK(++blocks < 500);
        engine_render(audio, 256);
    }
    engine_render(audio, 256);  /* let the outgoing voice finish its 88-sample fade */
    eng_peak[PART_BRK] = 0;
    engine_render(audio, 256);
    CHECK(seq.t[TRK_BRK].pos == step);
    printf("pattern=%d step=%d next-velocity=%u slice=%d Q15=%u\n", pattern, step,
           (unsigned)seq.brk_velocity, engine_brk_slice(), (unsigned)eng_peak[PART_BRK]);
    return eng_peak[PART_BRK];
}

/* The same packed target may exist in two patterns. Clearing a lane also has
 * to work when the FIFO is filled with note releases that cannot be evicted. */
static void clear_while_playing(unsigned full, unsigned half, unsigned quarter)
{
    int playing = motion_alloc(proj.arr.lane, 0, PART_BRK, T_VEL, PART_BRK << 4, 0);
    int other = motion_alloc(proj.arr.lane, 1, PART_BRK, T_VEL, PART_BRK << 4, 0);
    CHECK(playing >= 0 && other >= 0);
    lane_t *l = &proj.arr.lane[playing];
    l->val[5] = l->val[6] = 64;
    l->val[7] = l->val[8] = l->val[9] = 32;
    proj.arr.lane[other].val[6] = 1;
    CHECK(peak_on_step(0, 5) == half);
    engine_motion_clear(other);
    unsigned after_other = peak_on_step(0, 6);
    CHECK(peak_on_step(0, 7) == quarter);
    for (int n = 0; n < 1024; n++) engine_bass_off(0);
    engine_brk_live(32, 1);  /* the full FIFO must reject this press */
    engine_motion_clear(playing);
    unsigned old_voice = peak_on_step(0, 7);
    unsigned next_voice = peak_on_step(0, 8);
    CHECK(engine_brk_held() == 0);
    printf("clear other pattern=%u, current tail=%u, next hit after full FIFO=%u\n",
           after_other, old_voice, next_voice);
    CHECK(after_other == half);
    CHECK(old_voice == quarter);
    CHECK(next_voice == full);
    engine_brk_live(32, 1);
    engine_render(audio, 256);
    CHECK(engine_brk_held() & 1u);  /* the drained FIFO accepts the same press */
    engine_brk_live(32, 0);
    engine_render(audio, 256);
}

int main(void)
{
    setup(V_SEQ, PART_BRK);
    int sample_slot = engine_brk_nslots();
    memset(loop_data, 0x44, sizeof loop_data);
    slot_nz[0] = 1;
    slot_data[0][0] = loop_data;
    slot_n[0][0] = sizeof loop_data * 2;
    slot_rate[0][0] = 22050;
    brkpart_t *bp = &proj.pat[0].brk;
    bp->slot_a = bp->slot_b = (uint8_t)sample_slot;
    bp->steps = 0xffffu;
    memset(bp->slice, 2, sizeof bp->slice);  /* each step starts on the same decoded plateau */
    bp->set[BRK_COMPLEX] = 0;
    proj.pat[1].brk = *bp;
    ui_step_lane = UI_LANE_VELOCITY;
    turn_hit_velocity(1, -63);
    turn_hit_velocity(2, -127);
    turn_hit_velocity(4, -95);
    turn_hit_velocity(15, -127);
    int lane = motion_find(proj.arr.lane, 0, PART_BRK, T_VEL, PART_BRK << 4, 0);
    CHECK(lane >= 0);
    lane_t *l = &proj.arr.lane[lane];
    l->val[0] = 127;  /* older projects can store unity explicitly */
    proj.sound.v[T_MIX][PART_BRK][MX_LEVEL] = 75;  /* keep the unity signal inside the Q15 meter range */
    proj.set.bt |= PROJ_BT_CLK_IN;
    CHECK(project_validate_for(&proj) == 0);
    CHECK(project_save() == 0);
    memset(l, 0, sizeof *l);
    CHECK(project_load() == 0);
    CHECK(l->used && l->val[0] == 127 && l->val[1] == 64 && l->val[2] == 0 &&
          l->val[3] == MOT_NONE && l->val[4] == 32 && l->val[15] == 0);
    engine_apply_sound(&proj.sound);
    engine_brk_loops();
    CHECK(engine_brk_nslots() == sample_slot + 1);
    for (int n = 0; n < 10; n++) engine_render(audio, 256); /* apply queued project settings */
    seq_start(&seq);

    unsigned full = peak_on_step(0, 0);
    unsigned half = peak_on_step(0, 1);
    CHECK(full > 1000 && full < 32767);
    CHECK(abs((int)(half * 127) - (int)(full * 64)) <= 127);
    CHECK(peak_on_step(0, 2) == 0);
    CHECK(peak_on_step(0, 3) == full);   /* sparse hole restores the legacy hit */
    unsigned quarter = peak_on_step(0, 4);
    CHECK(abs((int)(quarter * 127) - (int)(full * 32)) <= 127);

    seq_cue_part(&seq, TRK_BRK, 1);
    CHECK(peak_on_step(0, 15) == 0);
    CHECK(peak_on_step(1, 0) == full);  /* no velocity lane in the queued pattern */
    seq_cue_part(&seq, TRK_BRK, 0);
    CHECK(peak_on_step(0, 2) == 0);

    /* Undo/load can replace a complete lane table before the audio boundary.
     * Remove the applied lane in that order, then disable motion as well. */
    memset(l, 0, sizeof *l);
    engine_motion_reset();
    engine_motion_enable(0);
    CHECK(peak_on_step(0, 3) == full);
    CHECK(seq.brk_velocity == 127);
    engine_motion_enable(1);
    CHECK(peak_on_step(0, 4) == full);
    clear_while_playing(full, half, quarter);
    printf("PASS sequenced BREAK velocity: saved/reloaded, sparse and pattern restore, "
           "lane replacement, motion disable; Q15 peaks 127=%u 64=%u 32=%u 0=0\n",
           full, half, quarter);
    return 0;
}
