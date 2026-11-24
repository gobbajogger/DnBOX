#define PWM_FIXTURE_MAIN retained_pwm_main
#include "ui_superj6_pwm1_lfo2_test.c"
int main(void)
{
    const char *scene = getenv("SJ6_MAILBOX_RED");
    unsigned v = scene[1] - '0';
    setup(V_PART, PART_303A + v);
    proj.set.bt |= PROJ_BT_CLK_IN;
    select_owner(v, 0);
    CHECK(engine_superj6_knob_position(v, SJ6_A_REFERENCE) == 60);
    if (scene[0] == 'l') {
        engine_note_lock_capture(1);
        CHECK(!engine_bass_note_capture(proj.arr.lane, 0, v, 0, &proj.sound));
        engine_note_lock_capture(0);
        bpart_t *p = &proj.pat[0].bass[v];
        for (unsigned n = 0; n < NSTEPS; ++n) p->step[n].flags = G_REST;
        p->len = 16; p->rate = RATE_16; p->dir = DIR_FWD; p->transpose = 24;
        seq.ppat[TRK_BASS0 + v] = 0;
        p->step[0].flags = G_NOTE;
        p->step[0].note = 45;
        p->step[0].velocity = 100;
        engine_note_lock_refresh();
    }
    CHECK(engine_superj6_knob_turn(v, SJ6_A_REFERENCE, 83, 0));
    engine_bass_edit_hold(1);
    if (scene[0] == 'p') {
        CHECK(!engine_bass_select_preset(v, ORIGINAL_SOUND_JUNO, 1, &proj.sound));
        engine_bass_cancel_preset(v);
    } else {
        seq_start(&seq);
        int active = 0;
        for (unsigned n = 0; n < 2000 && !active; ++n) {
            render(1);
            active = !engine_superj6_knob_turn(v, SJ6_A_REFERENCE, 84 + (n & 1u), 0);
        }
        CHECK(active);
        seq_stop(&seq);
    }
    render(2);
    engine_bass_edit_hold(0);
    render(1);
    CHECK(engine_superj6_knob_position(v, SJ6_A_REFERENCE) == 60);
    return 0;
}
