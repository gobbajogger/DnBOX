/* L12: on HOME, hold GLOBE + pad clears, and GLOBE + source pad + target pad
 * copies, only the selected instrument, for every part (909, 808, 303A, 303B,
 * BREAK). Every other part of the touched pattern stays byte-identical. */
#define main x0x_host_main
#include "../../host/x0x_host.c"
#undef main

static int fails;
#define CHECK(c, ...) do { if (!(c)) { fails++; printf("FAIL: " __VA_ARGS__); printf("\n"); } } while (0)

static void tick(uint32_t n) { while (n--) run_ms(1); }
static void btn(int b, int down) { if (down) held_btn |= 1u << b; else held_btn &= ~(1u << b); tick(1); }
static void pad(int white, int down) { int k = WHITE_KEY[white]; if (down) held_keys |= 1u << k; else held_keys &= ~(1u << k); tick(1); }

static void home(int part)
{
    int p;
    set_view(V_HOME);
    ui.overlay = O_NONE;
    ui.pbank = 0;
    ui.part = part;
    seq_chain(&seq, 15, 15);
    for (p = 0; p < NPARTS; p++) { seq.ppat[p] = 15; seq.pcue[p] = 0xFF; }
}
/* Give every part of pattern `pat` its own distinct content. */
static void fill(int pat, int seed)
{
    pattern_t *t = &proj.pat[pat];
    sm_set(t->drum[PART_909].hit[0], 1 + seed);
    sm_set(t->drum[PART_808].hit[1], 3 + seed);
    t->drum[PART_909].accent[0] = 0x3u << seed;
    t->drum[PART_808].accent[0] = 0x5u << seed;
    t->bass[0].step[seed].note = (uint8_t)(40 + seed); t->bass[0].step[seed].flags = G_NOTE;
    t->bass[1].step[seed + 1].note = (uint8_t)(50 + seed); t->bass[1].step[seed + 1].flags = G_NOTE;
    t->brk.steps = 0x9u << seed;
    t->brk.slice[seed] = (uint8_t)(2 + seed);
}
static const void *part_ptr(const pattern_t *t, int part)
{
    return part <= PART_808 ? (const void *)&t->drum[part] : part == PART_BRK ? (const void *)&t->brk : (const void *)&t->bass[part - PART_303A];
}
static size_t part_size(int part)
{
    return part <= PART_808 ? sizeof(dpart_t) : part == PART_BRK ? sizeof(brkpart_t) : sizeof(bpart_t);
}
static int same_part(const pattern_t *a, const pattern_t *b, int part)
{
    return !memcmp(part_ptr(a, part), part_ptr(b, part), part_size(part));
}
static int empty_part(const pattern_t *t, int part)
{
    int s;
    if (part <= PART_808) {
        for (s = 0; s < NDRUM; s++) if (sm_any(t->drum[part].hit[s])) return 0;
        return !sm_any(t->drum[part].accent);
    }
    if (part == PART_BRK) {
        for (s = 0; s < 16; s++) if (t->brk.slice[s]) return 0;
        return !t->brk.steps;
    }
    for (s = 0; s < (int)(sizeof t->bass[0].step / sizeof t->bass[0].step[0]); s++) if (bstep_gate(&t->bass[part - PART_303A].step[s]) == G_NOTE) return 0;
    return 1;
}

int main(void)
{
    int part, q;
    boot();
    tick(300);
    for (part = 0; part < NPARTS; part++) {
        pattern_t before, src, dst;
        int cp = part * 3,   /* pads 0..14 of bank 1; pattern 15 plays */
            from = cp + 1, to = cp + 2;
        home(part);
        fill(cp, part % 3); fill(from, 1 + part % 3); fill(to, 2);
        before = proj.pat[cp];
        CHECK(!empty_part(&before, part), "%s: fixture has content", part_name(part));
        btn(B_GLO, 1); pad(cp, 1); pad(cp, 0); btn(B_GLO, 0);
        CHECK(empty_part(&proj.pat[cp], part), "%s: GLOBE+pad clears the selected part", part_name(part));
        for (q = 0; q < NPARTS; q++)
            if (q != part) CHECK(same_part(&proj.pat[cp], &before, q), "%s clear: %s stays", part_name(part), part_name(q));
        CHECK(proj.pat[cp].swing == before.swing, "%s clear: swing stays", part_name(part));
        CHECK(ui.view == V_HOME && ui.overlay == O_NONE, "%s clear stays on HOME", part_name(part));

        home(part);
        src = proj.pat[from]; dst = proj.pat[to];
        btn(B_GLO, 1); pad(from, 1); pad(to, 1);
        if (ui.overlay == O_ASK) { btn(B_SEL, 1); btn(B_SEL, 0); }
        pad(from, 0); pad(to, 0); btn(B_GLO, 0);
        CHECK(same_part(&proj.pat[to], &src, part), "%s: copy writes the selected part", part_name(part));
        CHECK(!memcmp(&proj.pat[from], &src, sizeof src), "%s copy: source pattern intact", part_name(part));
        for (q = 0; q < NPARTS; q++)
            CHECK(same_part(&proj.pat[from], &src, q), "%s copy: source %s intact", part_name(part), part_name(q));
        for (q = 0; q < NPARTS; q++)
            if (q != part) CHECK(same_part(&proj.pat[to], &dst, q), "%s copy: %s stays", part_name(part), part_name(q));
        CHECK(ui.view == V_HOME && ui.overlay == O_NONE, "%s copy stays on HOME", part_name(part));
    }
    if (fails) { printf("home-globe-part-matrix: %d failure(s)\n", fails); return 1; }
    printf("home-globe-part-matrix: PASS (%d parts)\n", NPARTS);
    return 0;
}
