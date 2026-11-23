/* L12: on SEQ the 16 white pads write the 909, 808 and BREAK sequencer steps
 * (tap: on, tap again: off), the pad LEDs show exactly the written steps, the
 * second step page maps the pads to steps 17..32, and while the sequencer
 * plays the playhead pad's LED is inverted on the step the engine is on. */
#define main x0x_host_main
#include "../../host/x0x_host.c"
#undef main

static int fails;
#define CHECK(c, ...) do { if (!(c)) { fails++; printf("FAIL: " __VA_ARGS__); printf("\n"); } } while (0)

static void tick(uint32_t n) { while (n--) run_ms(1); }
static void pad(int white, int down) { int k = WHITE_KEY[white]; if (down) held_keys |= 1u << k; else held_keys &= ~(1u << k); tick(1); }
static void pad_tap(int white) { pad(white, 1); pad(white, 0); }
static uint32_t lights(void) { leds(); return lit_keys; }

static int step_on(int part, int step)
{
    const pattern_t *t = cur_pat();
    if (part == PART_BRK) return (int)((t->brk.steps >> step) & 1u);
    return sm_get(t->drum[part].hit[ui.sel[part]], step);
}
static uint32_t expected(int part)
{
    uint32_t k = 0;
    for (int i = 0; i < 16; i++) {
        int s = part == PART_BRK ? i : step_of(i);
        if (step_on(part, s)) k |= 1u << WHITE_KEY[i];
    }
    return k;
}
static uint32_t white_mask(void)
{
    uint32_t m = 0;
    for (int i = 0; i < 16; i++) m |= 1u << WHITE_KEY[i];
    return m;
}

int main(void)
{
    static const int parts[] = {PART_909, PART_808, PART_BRK};
    boot();
    tick(300);
    proj.set.keyled = 1;
    for (unsigned n = 0; n < sizeof parts / sizeof parts[0]; n++) {
        int part = parts[n], pages = part == PART_BRK ? 1 : 2;
        const char *name = part_name(part);
        seq_stop(&seq);
        tick(50);
        set_view(V_SEQ);
        ui.overlay = O_NONE;
        ui.part = (uint8_t)part;
        if (part < NKIT) { ui.sel[part] = (uint8_t)(n + 1); cur_pat()->drum[part].len = 32; }
        build_pages();
        memset(&proj.pat[seq.ppat[part]], 0, sizeof proj.pat[0]);
        pattern_init(&proj.pat[seq.ppat[part]], 1, 2);
        if (part < NKIT) cur_pat()->drum[part].len = 32;
        for (int page = 0; page < pages; page++) {
            ui.spage = (uint8_t)page;
            CHECK((lights() & white_mask()) == 0, "%s page %d: empty pattern lights no step pad (lit %08x playing %d)", name, page, (unsigned)(lights() & white_mask()), seq.playing);
            for (int i = 0; i < 16; i++) {
                int s = part == PART_BRK ? i : step_of(i);
                CHECK(part == PART_BRK || s == page * 16 + i, "%s: pad %d maps to step %d", name, i, page * 16 + i);
                pad_tap(i);
                CHECK(step_on(part, s), "%s page %d: pad %d writes its step", name, page, i);
                CHECK(lights() & (1u << WHITE_KEY[i]), "%s page %d: pad %d LED lights", name, page, i);
                CHECK(ui.view == V_SEQ, "%s: the pad stays on SEQ", name);
            }
            for (int i = 0; i < 16; i += 2) pad_tap(i);   /* every other step back off */
            for (int i = 0; i < 16; i++) {
                int s = part == PART_BRK ? i : step_of(i);
                CHECK(step_on(part, s) == (i & 1), "%s page %d: second tap on pad %d toggles", name, page, i);
            }
            CHECK((lights() & white_mask()) == expected(part), "%s page %d: LEDs are exactly the steps", name, page);
        }
        if (part < NKIT) {
            int other = part == PART_909 ? PART_808 : PART_909;
            CHECK(!sm_any(cur_pat()->drum[other].hit[ui.sel[part]]), "%s pads leave %s", name, part_name(other));
        }
        /* the sequencer plays what the pads wrote; the playhead LED inverts */
        ui.spage = 0;
        seq_start(&seq);
        uint32_t seen = 0;
        for (int ms = 0; ms < 4000 && __builtin_popcount(seen) < 4; ms++) {
            tick(1);
            int ph = eng_step[part == PART_BRK ? TRK_BRK : TRK_DRUM + part];
            if (ph < 0 || ph >= 16) continue;
            uint32_t want = expected(part) ^ (1u << WHITE_KEY[ph]);
            if ((lights() & white_mask()) == want) seen |= 1u << ph;
        }
        CHECK(seq.playing, "%s: the sequencer plays", name);
        CHECK(__builtin_popcount(seen) >= 4, "%s: the playhead pad LED follows the playing step", name);
        seq_stop(&seq);
    }
    if (fails) { printf("seq-pad-led: %d failure(s)\n", fails); return 1; }
    printf("seq-pad-led: PASS\n");
    return 0;
}
