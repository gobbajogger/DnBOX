/* User ask #61 (Guidelines §3, Design Language "Synth SEQ / GEN"): on the keyed R22
 * roll an accent is carried by its own note (full part colour; unaccented notes are
 * dim), a slide is drawn between its note and the next note, a slide out of the last
 * visible step keeps a boundary cue, and neither flag draws anything outside the roll
 * (no separate AC/SL rows). Run on all four synth engines. Mutants: roll_marks_test.sh. */
#define main x0x_host_main
#include "../../host/x0x_host.c"
#undef main

static int fails;
#define CHECK(c, ...) do { if (!(c)) { fails++; printf("FAIL: " __VA_ARGS__); printf("\n"); } } while (0)
static void tick(uint32_t n) { while (n--) run_ms(1); }
static const uint8_t NOTE[16] = {48, 55, 52, 60, 48, 57, 50, 59, 53, 48, 55, 60, 51, 58, 49, 56};
#define ACC_MASK  ((1u << 1) | (1u << 5) | (1u << 9) | (1u << 13))
#define SLIDE_MASK ((1u << 2) | (1u << 6) | (1u << 10) | (1u << 15))
static int sx(int c) { return GX + 13 * c + c / 4; }
static uint16_t px(int x, int y) { return fb[y * 240 + x]; }
static uint16_t shot_a[240 * 240];

static void pattern(int flags_on)
{
    bpart_t *bp = &cur_pat()->bass[0];
    memset(bp->step, 0, sizeof bp->step);
    bp->len = 16; bp->transpose = 24;
    for (int s = 0; s < 16; s++) {
        bp->step[s].note = NOTE[s];
        bp->step[s].flags = G_NOTE;
        if (flags_on && (ACC_MASK >> s & 1u)) bp->step[s].flags |= BS_ACCENT;
        if (flags_on && (SLIDE_MASK >> s & 1u)) bp->step[s].flags |= BS_SLIDE;
    }
    ui.spage = 0; ui.held_step = -1; ui.rec = 0;
    ui_frame(); tick(2); ui_frame();
}

int main(void)
{
    static const char *const NAME[4] = {"CHICAGO", "MIAMI", "SYNTH ONE", "SUPERJ6"};
    boot(); tick(300);
    for (int kind = 0; kind < 4; kind++) {
        seq_stop(&seq); tick(20);
        ui.part = PART_303A;
        if (engine_bass_select_preset(0, kind, 0, &proj.sound) != 0) { CHECK(0, "%s: engine select", NAME[kind]); continue; }
        for (int i = 0; i < 12; i++) { int32_t a[512]; engine_render(a, 256); }
        set_view(V_SEQ); ui.overlay = O_NONE; *page_of(V_SEQ) = 0; build_pages();
        uint16_t col = swap16(part_col()), dimc = swap16(dim(part_col(), 9)), white = swap16(C_WHITE);
        int lo = 127, hi = 0;
        for (int s = 0; s < 16; s++) { if (NOTE[s] < lo) lo = NOTE[s]; if (NOTE[s] > hi) hi = NOTE[s]; }
        lo -= 2; hi += 2; if (hi - lo < 14) hi = lo + 14;
        pattern(0); memcpy(shot_a, fb, sizeof shot_a);
        pattern(1);
        int acc_ok = 0, plain_ok = 0, slide_ok = 0, noslide_ok = 0;
        for (int c = 0; c < 16; c++) {
            int x = sx(c), y = HDR_H + note_y(NOTE[c], lo, hi);
            uint16_t body = px(x + 6, y + 2);
            if (ACC_MASK >> c & 1u) { acc_ok += body == col; CHECK(body == col, "%s step %d: accent is not carried by its note (body %04x, want %04x)", NAME[kind], c + 1, body, col); }
            else { plain_ok += body == dimc; CHECK(body == dimc, "%s step %d: unaccented note body %04x, want dim %04x", NAME[kind], c + 1, body, dimc); }
            /* the gap after the note bar: a slide line, or nothing */
            int ny = c < 15 ? HDR_H + note_y(NOTE[c + 1], lo, hi) : y;
            int y0 = (y < ny ? y : ny), y1 = (y > ny ? y : ny) + 4, w = 0;
            for (int yy = y0; yy <= y1; yy++) for (int xx = x + 12; xx <= x + 13; xx++) w += px(xx, yy) == white;
            if (SLIDE_MASK >> c & 1u) { slide_ok += w >= 2; CHECK(w >= 2, "%s step %d: slide not drawn from its note toward the next (%d px)", NAME[kind], c + 1, w); }
            else { noslide_ok += w == 0; CHECK(w == 0, "%s step %d: a non-slide step draws a slide mark (%d px)", NAME[kind], c + 1, w); }
        }
        /* accent/slide change pixels only inside the roll: no AC/SL rows, no text */
        int outside = 0, inside = 0;
        for (int yy = 0; yy < 240; yy++) for (int xx = 0; xx < 240; xx++) {
            if (fb[yy * 240 + xx] == shot_a[yy * 240 + xx]) continue;
            int in_roll = yy >= HDR_H + ROLL_TOP - 4 && yy <= HDR_H + ROLL_BOT + 5 && xx >= GX - 8;
            if (in_roll) inside++; else outside++;
        }
        CHECK(inside > 0, "%s: accent/slide flags changed nothing on the roll", NAME[kind]);
        CHECK(outside == 0, "%s: accent/slide flags draw %d px outside the roll (separate AC/SL rows?)", NAME[kind], outside);
        printf("%s: accents %d/4, plain %d/12, slides %d/4, clean gaps %d/12, roll px %d, outside %d\n",
               NAME[kind], acc_ok, plain_ok, slide_ok, noslide_ok, inside, outside);
    }
    if (fails) { printf("roll-marks: %d failure(s)\n", fails); return 1; }
    printf("roll-marks: PASS\n");
    return 0;
}
