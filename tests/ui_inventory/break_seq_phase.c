/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN smc_fixture_main
#include "../host/smc_ui_test.c"

static void button(unsigned mask)
{
    now_ms += 40;
    held_btn = mask;
    input();
}

static void tap(unsigned key)
{
    button(1u << key);
    button(0);
}

int main(void)
{
    setup(V_HOME, PART_909);
    for (int attempt = 0; ui.part != PART_BRK && attempt < NPARTS; ++attempt) {
        now_ms += 100;
        enc_acc[EN_ALGO] = 1;
        input();
    }
    tap(B_SEQ);
    CHECK(ui.part == PART_BRK && ui.view == V_SEQ && ui.overlay == O_NONE);
    cur_pat()->brk.steps = 0;
    cur_pat()->brk.rate = RATE_16;

#ifdef SHIPPING_026
    const int lanes = 1, top = 95, bottom = 106;
#else
    const int lanes = 4, top = KNOB_Y + 69, bottom = KNOB_Y + 82;
#endif
    unsigned stationary = 0;
    for (int lane = 0; lane < lanes; ++lane) {
        uint16_t previous_pixels[13 * 240];
        const size_t bytes = (bottom - top) * 240 * sizeof *previous_pixels;
        now_ms += 60;
        ui_frame();
        char name[32];
        snprintf(name, sizeof name, "lane-%d-stopped", lane);
        shot(name);
        tap(B_PLAY);
        for (int block = 0; block < 7; ++block) audio_block();
        int previous = -1;
        for (int sample = 0; sample < 16; ++sample) {
            for (int block = 0; block < 250 && eng_step[TRK_BRK] == previous; ++block)
                audio_block();
            int phase = eng_step[TRK_BRK];
            CHECK(seq.playing && phase == sample);
            now_ms += 60;
            ui_frame();
            leds();
            CHECK(lit_keys & (1u << WHITE_KEY[phase]));
            if (previous >= 0) {
                CHECK(!(lit_keys & (1u << WHITE_KEY[previous])));
                stationary += !memcmp(previous_pixels, fb + top * 240, bytes);
            }
            CHECK(cur_pat()->brk.steps == 0);
            memcpy(previous_pixels, fb + top * 240, bytes);
            snprintf(name, sizeof name, "lane-%d-phase-%02d", lane, phase);
            shot(name);
            previous = phase;
        }
        tap(B_PLAY);
        for (int block = 0; block < 7; ++block) audio_block();
        CHECK(!seq.playing);
        leds();
        for (int step = 0; step < 16; ++step) CHECK(!(lit_keys & (1u << WHITE_KEY[step])));
#ifndef SHIPPING_026
        if (lane + 1 < lanes) {
            button(1u << B_SEQ);
            button((1u << B_SEQ) | (1u << B_SEL));
            button(1u << B_SEQ);
            button(0);
            CHECK(ui_step_lane == lane + 1);
        }
#endif
    }
    printf("BREAK SEQ: %d lanes, %u stationary transitions, %u checks\n", lanes, stationary, checks);
    return stationary != 0;
}
