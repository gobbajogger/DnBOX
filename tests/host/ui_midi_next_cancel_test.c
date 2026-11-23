/* SPDX-License-Identifier: GPL-3.0-only */
/* Real SMC MIDI rewind cancels a queued pattern without repainting its popup. */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"

static uint16_t baseline[240 * 240];

int main(void)
{
    setup(V_HOME, PART_909);
    seq.ppat[PART_909] = 14;
    seq_cue_part(&seq, PART_909, 15);
    fader(0, 0);
    CHECK(smc_ui_active() && seq_cue_of(&seq, PART_909) == 15);
    host_battery_level = 2;
    ui_frame();
    memcpy(baseline, fb, sizeof baseline);

    message(0x90, 0x5B, 127);  /* BLE rewind to the playing pattern: cancel. */
    CHECK(seq_cue_of(&seq, PART_909) == -1);
    uint32_t expiry = ui.msg_until;
    CHECK(expiry > now_ms);
    smc_ui.until = expiry + SMC_UI_MS;
    now_ms = expiry - 1;
    ui_frame();
    CHECK(smc_ui_active());

    /* The complete frame must equal the original plus only the bounded cue. */
    cv_begin(240, HDR_H, C_BLACK);
    memcpy(cv_px, baseline, 240 * HDR_H * sizeof *cv_px);
    cv_text_clip(146, 4, &FONT_T, "NEXT --", C_AMB, 144, 189);
    CHECK(memcmp(cv_px, baseline, 240 * HDR_H * sizeof *cv_px) != 0);
    int header = !memcmp(fb, cv_px, 240 * HDR_H * sizeof *fb);
    int body = !memcmp(fb + 240 * HDR_H, baseline + 240 * HDR_H,
                       (240 - HDR_H) * 240 * sizeof *fb);

    now_ms = expiry;
    ui_frame();
    int restored = smc_ui_active() && !memcmp(fb, baseline, sizeof baseline);
    printf("MIDI NEXT cancellation: bounded header=%d retained body=%d exact expiry=%d\n",
           header, body, restored);
    CHECK(header && body && restored);
    return 0;
}
