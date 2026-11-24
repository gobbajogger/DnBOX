/* SPDX-License-Identifier: GPL-3.0-only */
/* FINAL-GOAL row 7, user 17 Nov 17:30 ("Only for synth seq r22"): every synth
 * SEQ page (Miami, Chicago, SuperJ6, Synth One; stopped and playing) is the
 * approved R22 keyed piano roll: a piano-key column (white keys with inset
 * black keys) at x 5..22, a beat grid on every step boundary with the beat
 * lines brighter, and a playhead column while playing. 0.0.27 (73749f55)
 * drew this only on GEN; plain SEQ showed octave labels on a blank field. */
#define SMC_UI_MAIN unused_smc_ui_main
#include "smc_ui_test.c"
static void audio_ms(unsigned ms){int32_t a[512];unsigned left=ms*44100/1000;while(left){unsigned n=left>256?256:left;engine_render(a,n);left-=n;}}
static int keyed_roll(const char *what, int playing)
{
    memset(cv_tile_valid, 0, sizeof cv_tile_valid);
    memset(blit_hash, 0, sizeof blit_hash);
    ui_frame();
    char name[64]; snprintf(name, sizeof name, "seq-keyed-%s%s", what, playing ? "-play" : "");
    shot(name);
    unsigned white = 0, black_inset = 0, beat = 0, sub = 0, head = 0;
    for (int y = MAIN_Y + 6; y < MAIN_Y + 110; y++) {
        white += fb[y*240 + 20] == swap16(C_GRAY);              /* every key's right half */
        black_inset += fb[y*240 + 8] != swap16(C_GRAY) && fb[y*240 + 20] == swap16(C_GRAY);
        beat += fb[y*240 + col_x(4) - 1] == swap16(C_DIM);
        sub += fb[y*240 + col_x(5) - 1] == swap16(C_LINE) || fb[y*240 + col_x(5) - 1] == swap16(C_GRAY);
        if (playing) for (int c = 0; c < 16; c++) head += fb[y*240 + col_x(c) + 6] == swap16(dim(part_col(), 2));
    }
    int ok = ui.view == V_SEQ && white >= 60 && black_inset >= 10 && beat >= 60 && sub >= 60 && (!playing || head >= 20);
    printf("KEYED_ROLL %s play=%d white=%u black=%u beat=%u sub=%u head=%u %s\n",
           what, playing, white, black_inset, beat, sub, head, ok ? "ok" : "MISSING");
    return ok;
}
int main(void)
{
    static const struct { unsigned bank; const char *name; } banks[] = {
        {ORIGINAL_SOUND_MIAMI, "miami"}, {ORIGINAL_SOUND_CHICAGO, "chicago"},
        {ORIGINAL_SOUND_JUNO, "superj6"}, {ORIGINAL_SOUND_SYNTHONE, "synthone"}};
    int screens = 0;
    for (int owner = 0; owner < 2; owner++)
        for (unsigned b = 0; b < sizeof banks / sizeof *banks; b++) {
            setup(V_PART, PART_303A + owner);
            if (engine_bass_select_preset((unsigned)owner, banks[b].bank, 0, &proj.sound)) continue; /* not in this build */
            for (int i = 0; i < 1024 && engine_bass_preset_pending((unsigned)owner); i++) audio_ms(6);
            set_view(V_SEQ); build_pages();
            char what[32]; snprintf(what, sizeof what, "%s-%d", banks[b].name, owner);
            CHECK(keyed_roll(what, 0)); screens++;
            button_tap(B_PLAY); audio_ms(300);
            CHECK(seq.playing);
            CHECK(keyed_roll(what, 1)); screens++;
            button_tap(B_PLAY); audio_ms(20);
        }
    CHECK(screens >= 8);
    printf("PASS %d synth SEQ screens draw the R22 keyed piano roll\n", screens);
    return 0;
}
