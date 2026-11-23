/* User ask #60 (guidelines §3): on the 909/808 SEQ grid the playhead cell lights
 * white only on the real hit. A rest, a muted track or a chance-zero step at the
 * playhead must not impersonate one. Negative: on 14c6df7b (white on every
 * written step at the playhead) the rest-free mute and chance-zero checks fail. */
#define main x0x_host_main
#include "../../host/x0x_host.c"
#undef main

static int fails;
#define CHECK(c, ...) do { if (!(c)) { fails++; printf("FAIL: " __VA_ARGS__); printf("\n"); } } while (0)
static void tick(uint32_t n) { while (n--) run_ms(1); }

/* white pixels of the playhead flash box in grid column c, BD row (voice 0) */
static int flash_px(int c)
{
    int n = 0, x0 = col_x(c) + 2, y0 = HDR_H + 0 * DROW + 1 + 2;
    for (int y = y0; y < y0 + DROW - 6; y++)
        for (int x = x0; x < x0 + 8; x++)
            n += fb[y * 240 + x] == swap16(C_WHITE);
    return n;
}

/* play until the playhead is on step s; render a frame mid-step; white pixels there */
static int lit_on(int kit, int s)
{
    for (int ms = 0; ms < 6000; ms++) {
        tick(1);
        if (eng_step[TRK_DRUM + kit] == s) {
            tick(20);
            if (eng_step[TRK_DRUM + kit] != s) continue;
            ui_frame();
            return flash_px(s);
        }
    }
    CHECK(0, "kit %d: the playhead never reached step %d", kit, s);
    return -1;
}

int main(void)
{
    boot();
    tick(300);
    for (int kit = 0; kit < NKIT; kit++) {
        const char *name = kit ? "808" : "909";
        seq_stop(&seq); tick(50);
        set_view(V_SEQ); ui.overlay = O_NONE; ui.part = (uint8_t)kit; ui.sel[kit] = 0; ui.spage = 0;
        memset(&proj.pat[seq.ppat[kit]], 0, sizeof proj.pat[0]);
        pattern_init(&proj.pat[seq.ppat[kit]], 1, 2);
        dpart_t *d = &cur_pat()->drum[kit];
        d->len = 16;
        sm_set(d->hit[0], 0); sm_set(d->hit[0], 4); sm_set(d->hit[0], 8);   /* 2, 6: rests */
        int lane = motion_alloc(proj.arr.lane, seq.ppat[kit], kit, T_PROB, kit << 4, 0);
        CHECK(lane >= 0, "%s: a chance lane", name);
        if (lane >= 0) { memset(proj.arr.lane[lane].val, MOT_NONE, NSTEPS); proj.arr.lane[lane].val[4] = 0; }
        seq.vmute &= ~(1u << (kit * NDRUM));
        build_pages();
        seq_start(&seq);
        int hit = lit_on(kit, 0), rest = lit_on(kit, 2), chance0 = lit_on(kit, 4), again = lit_on(kit, 8);
        CHECK(hit >= 20, "%s: a real hit at the playhead lights (%d px)", name, hit);
        CHECK(again >= 20, "%s: the next real hit lights (%d px)", name, again);
        CHECK(rest == 0, "%s: a rest at the playhead stays dark (%d px)", name, rest);
        CHECK(chance0 == 0, "%s: a chance-zero step does not impersonate a hit (%d px)", name, chance0);
        seq.vmute |= 1u << (kit * NDRUM);
        for (int ms = 0; ms < 6000 && eng_step[TRK_DRUM + kit] == 8; ms++) tick(1);   /* a fresh pass over step 8 */
        int muted = lit_on(kit, 8);
        CHECK(muted == 0, "%s: a muted track's written step stays dark (%d px)", name, muted);
        seq.vmute &= ~(1u << (kit * NDRUM));
        seq_stop(&seq);
        if (lane >= 0) proj.arr.lane[lane].used = 0;
    }
    if (fails) { printf("seq-hit-light: %d failure(s)\n", fails); return 1; }
    printf("seq-hit-light: PASS\n");
    return 0;
}
