/* SPDX-License-Identifier: GPL-3.0-only */
#define PWM_FIXTURE_MAIN retained_main
#include "ui_superj6_pwm1_lfo2_test.c"

int main(void)
{
    setup(V_PART, PART_303A);
    select_owner(0,0);
    sound_t previous=proj.sound;
    for (unsigned shape=0;shape<4;++shape) {
        uint8_t cell[ORIGINAL_SOUND_BYTES];
        memcpy(cell, previous.v[T_303][0],sizeof cell);
        cell[31]=(cell[31]&15)|(shape?(8u+shape)<<4:0u);
        CHECK(original_sound_valid(cell));
        CHECK(original_sound_sj6_shape(cell)==shape);
    }
    sound_t unsupported=previous;
    uint8_t *cell=unsupported.v[T_303][0];
    cell[31]=(cell[31]&15)|0x80u;
    CHECK(!original_sound_valid(cell));
    engine_note_lock_capture(1);
    CHECK(!engine_bass_note_capture(proj.arr.lane,0,0,0,&proj.sound));
    engine_note_lock_capture(0);
    proj.arr.lane[0].val[31]=(proj.arr.lane[0].val[31]&15)|0x80u;
    CHECK(engine_bass_note_validate(&proj.arr.lane[0]));
    CHECK(!memcmp(&previous,&proj.sound,sizeof previous));
    puts("PASS exact old composed reader admits four old shapes and rejects S&H in sound and existing note lock, no writes");
}
