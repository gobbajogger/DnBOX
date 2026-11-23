/* HOME: hold GLOBE and tap a pattern pad to clear that pattern for the selected
 * part. Hold GLOBE, hold the source pad, then press the target to copy that part.
 * Off HOME the gesture does not run. GLOBE alone still opens settings, and
 * CLEAR PATTERN still clears every part's current pattern. */
#define main x0x_host_main
#include "../../host/x0x_host.c"
#undef main

static int fails;
#define CHECK(c, msg) do { if (!(c)) { fails++; printf("FAIL: %s\n", msg); } } while (0)

static void tick(uint32_t n) { while (n--) run_ms(1); }
static void btn(int b, int down)
{
    if (down) held_btn |= 1u << b;
    else held_btn &= ~(1u << b);
    tick(1);
}
static void key(int k, int down)
{
    if (down) held_keys |= 1u << k;
    else held_keys &= ~(1u << k);
    tick(1);
}
static void tap_btn(int b) { btn(b, 1); btn(b, 0); }
static void pad(int white, int down) { key(WHITE_KEY[white], down); }

static void hit(int pat, int part, int step) { sm_set(proj.pat[pat].drum[part].hit[0], step); }
static int has(int pat, int part, int step) { return sm_get(proj.pat[pat].drum[part].hit[0], step); }
static void note_on(int pat, int bass, int step, int n)
{
    proj.pat[pat].bass[bass].step[step].note = (uint8_t)n;
    proj.pat[pat].bass[bass].step[step].flags = G_NOTE;
}
static int noted(int pat, int bass, int step, int n)
{
    bstep_t *s = &proj.pat[pat].bass[bass].step[step];
    return bstep_gate(s) == G_NOTE && s->note == n;
}
static int lane_of(int pat, int part)
{
    int k;
    for (k = 0; k < NLANE; k++)
        if (proj.arr.lane[k].used && proj.arr.lane[k].pat == pat && proj.arr.lane[k].part == part)
            return k;
    return -1;
}
static int action_row(int act)
{
    int i;
    for (i = 0; i < ui.list_n; i++)
        if (list_rows[i].kind == R_ACT && list_rows[i].a == act)
            return i;
    return -1;
}
static void home(void)
{
    int p;
    set_view(V_HOME);
    ui.overlay = O_NONE;
    ui.pbank = 0;
    ui.part = PART_909;
    seq_chain(&seq, 0, 0);
    for (p = 0; p < NPARTS; p++) {
        seq.ppat[p] = 0;
        seq.pcue[p] = 0xFF;
    }
}
static void expect_msg(const char *s, const char *what)
{
    CHECK(!strcmp(ui.msg, s), what);
    if (strcmp(ui.msg, s))
        printf("     message: \"%s\"\n", ui.msg);
}

int main(void)
{
    pattern_t keep4, keep10, keep11, keep17;
    int row, lane909, lane808, src_lane;
    uint8_t epoch;
    char pasted[48];

    boot();
    tick(300);
    home();

    hit(0, PART_909, 8);
    hit(2, PART_909, 1);
    hit(2, PART_808, 4);
    proj.pat[2].drum[PART_909].accent[0] = 0x3;
    proj.pat[2].drum[PART_808].accent[0] = 0x5;
    note_on(2, 0, 0, 40);
    proj.pat[2].brk.steps = 0x4;
    proj.pat[2].brk.slice[1] = 3;
    proj.pat[2].swing = 61;
    hit(4, PART_909, 6);
    hit(4, PART_808, 2);
    note_on(4, 0, 0, 50);
    hit(11, PART_909, 3);
    hit(11, PART_808, 5);
    lane909 = motion_alloc(proj.arr.lane, 2, PART_909, T_MIX, PART_909, MX_LEVEL);
    lane808 = motion_alloc(proj.arr.lane, 2, PART_808, T_MIX, PART_808, MX_LEVEL);
    CHECK(lane909 >= 0 && lane808 >= 0, "motion lanes allocate");
    proj.arr.lane[lane909].val[0] = 33;
    proj.arr.lane[lane808].val[0] = 44;
    src_lane = motion_alloc(proj.arr.lane, 4, PART_808, T_MIX, PART_808, MX_LEVEL);
    CHECK(src_lane >= 0, "copy source lane allocates");
    proj.arr.lane[src_lane].val[0] = 41;

    epoch = seq.rec_epoch;
    seq.rec_bass[0].active = 1;
    seq.rec_bass[0].pat = 4;
    seq.rec_bass[0].first = seq.rec_bass[0].last = 0;
    seq.rec_bass[0].entered = 1;
    seq.rec_bass[0].slot = 1000;
    ui.dirty = 0;
    keep11 = proj.pat[11];

    btn(B_GLO, 1);
    pad(2, 1);
    pad(2, 0);
    CHECK(!has(2, PART_909, 1), "GLOBE+pad clears the selected part in that pattern");
    CHECK(!sm_any(proj.pat[2].drum[PART_909].hit[0]), "cleared 909 row is empty");
    CHECK(proj.pat[2].drum[PART_909].accent[0] == 0, "cleared 909 accent is empty");
    CHECK(has(2, PART_808, 4), "other part in the target pattern stays");
    CHECK(proj.pat[2].drum[PART_808].accent[0] == 0x5, "other part accent stays");
    CHECK(noted(2, 0, 0, 40), "303 in the target pattern stays");
    CHECK(proj.pat[2].brk.steps == 0x4 && proj.pat[2].brk.slice[1] == 3, "break in the target pattern stays");
    CHECK(proj.pat[2].swing == 61, "swing stays with the existing part clear");
    CHECK(!memcmp(&proj.pat[11], &keep11, sizeof keep11), "other patterns stay");
    CHECK(has(4, PART_909, 6) && has(4, PART_808, 2), "the pattern under the recorder keeps its drums");
    CHECK(has(0, PART_909, 8), "the playing pattern stays");
    CHECK(seq.ppat[PART_909] == 0 && seq.ppat[PART_808] == 0, "the gesture does not change the playing pattern");
    CHECK(seq_cue_of(&seq, PART_909) < 0, "the gesture does not cue a pattern");
    CHECK(seq.chain_a == seq.chain_b, "the gesture does not start a chain");
    CHECK(ui.view == V_HOME && ui.overlay == O_NONE, "clear stays on HOME");
    CHECK(ui.dirty == 1, "clear marks the project dirty");
    expect_msg("CLEARED 909 P3", "clear names the part and pattern");
    CHECK(!seq.rec_bass[0].active, "an active recording is finished before the clear");
    CHECK(seq.rec_epoch == (uint8_t)(epoch + 1), "recording epoch advances");
    CHECK(noted(4, 0, 0, 50), "finishing the recording does not clear another pattern");
    CHECK(!proj.arr.lane[lane909].used, "the cleared part's motion lane is released");
    CHECK(proj.arr.lane[lane808].used && proj.arr.lane[lane808].val[0] == 44, "the other part's lane stays");
    btn(B_GLO, 0);
    CHECK(ui.view == V_HOME, "GLOBE used with a pad does not open settings");

    CHECK(project_save() == 0, "cleared pattern saves");
    hit(2, PART_909, 1);
    CHECK(project_load() == 0, "saved project loads");
    CHECK(!has(2, PART_909, 1), "load keeps the cleared part empty");
    CHECK(has(2, PART_808, 4), "load keeps the other part");
    CHECK(lane_of(2, PART_909) < 0, "load keeps the cleared lane gone");
    CHECK(lane_of(2, PART_808) >= 0, "load keeps the other lane");
    home();

    set_view(V_FX);
    ui.overlay = O_NONE;
    ui.msg[0] = 0;
    keep4 = proj.pat[4];
    btn(B_GLO, 1);
    pad(4, 1);
    pad(4, 0);
    CHECK(!memcmp(&proj.pat[4], &keep4, sizeof keep4), "GLOBE+pad off HOME does not clear");
    CHECK(ui.view == V_FX && ui.overlay == O_NONE, "off HOME the pad does not leave the page");
    CHECK(strncmp(ui.msg, "CLEARED", 7) != 0, "off HOME there is no clear toast");
    btn(B_GLO, 0);
    CHECK(ui.view == V_GLO && ui.overlay == O_LIST, "GLOBE release off HOME still opens settings");
    tap_btn(B_GLO);
    CHECK(ui.view == V_FX, "GLOBE again still closes settings");

    set_view(V_SEQ);
    ui.part = PART_909;
    ui.overlay = O_NONE;
    for (row = 0; row < NPARTS; row++)
        seq.ppat[row] = 0;
    keep4 = proj.pat[3];
    btn(B_GLO, 1);
    pad(3, 1);
    pad(3, 0);
    CHECK(!memcmp(&proj.pat[3], &keep4, sizeof keep4), "SEQ does not treat the pad as a pattern clear");
    CHECK(ui.view == V_SEQ, "SEQ stays up while GLOBE is held");
    btn(B_GLO, 0);

    home();
    ui.overlay = O_NONE;
    ui.msg[0] = 0;
    tap_btn(B_GLO);
    CHECK(ui.view == V_GLO && ui.overlay == O_LIST, "GLOBE alone opens settings");
    CHECK(strncmp(ui.msg, "CLEARED", 7) != 0, "GLOBE alone does not clear");
    tap_btn(B_GLO);
    CHECK(ui.view == V_HOME && ui.overlay == O_NONE, "GLOBE again closes settings");

    home();
    ui.part = PART_808;
    {
        uint8_t swing = proj.pat[1].swing;
        pattern_t drums = proj.pat[1];
        btn(B_GLO, 1);
    pad(4, 1);
    pad(1, 1);
    CHECK(has(1, PART_808, 2), "copy writes the selected part into the target pattern");
    CHECK(!has(1, PART_909, 6), "copy does not bring other parts along");
    CHECK(!has(1, PART_909, 1) && !noted(1, 0, 0, 40), "copy does not touch other parts already considered");
    CHECK(has(4, PART_808, 2) && has(4, PART_909, 6), "copy leaves the source pattern intact");
    CHECK(proj.pat[1].swing == swing, "808 copy does not move swing");
    CHECK(!memcmp(&proj.pat[1].drum[PART_909], &drums.drum[PART_909], sizeof drums.drum[PART_909]),
          "copy leaves the other drum part");
    CHECK(!memcmp(proj.pat[1].bass, drums.bass, sizeof drums.bass), "copy leaves both bass parts");
    CHECK(!memcmp(&proj.pat[1].brk, &drums.brk, sizeof drums.brk), "copy leaves the break");
    {
        int copied = motion_find(proj.arr.lane, 1, PART_808, T_MIX, PART_808, MX_LEVEL);
        CHECK(copied >= 0 && proj.arr.lane[copied].val[0] == 41, "copy takes the selected part's lane");
        CHECK(proj.arr.lane[src_lane].used && proj.arr.lane[src_lane].pat == 4, "source lane stays on the source");
    }
    expect_msg("808 PASTED TO P2", "copy names the selected part");
    CHECK(ui.view == V_HOME && ui.overlay == O_NONE, "empty-target copy does not ask");
    pad(4, 0);
    pad(1, 0);
    CHECK(has(4, PART_808, 2), "releasing the source after a copy does not clear it");
    btn(B_GLO, 0);
    CHECK(ui.view == V_HOME, "GLOBE used to copy does not open settings");
    }

    note_on(5, 0, 2, 48);
    note_on(6, 0, 2, 60);
    hit(6, PART_909, 3);
    ui.part = PART_303A;
    btn(B_GLO, 1);
    pad(5, 1);
    pad(6, 1);
    CHECK(ui.overlay == O_ASK && ui.ask_act == ACT_COPY && ui.ask_arg == 6, "occupied part asks before paste");
    char expected[28];
    snprintf(expected, sizeof(expected), "%s PART REPLACED", part_name(PART_303A));
    CHECK(!strncmp(ui.ask_q[1], expected, sizeof(expected)), "paste names the selected bass part and whole-part replacement");
    CHECK(noted(6, 0, 2, 60) && has(6, PART_909, 3), "the question does not paste yet");
    btn(B_HOME, 1);
    btn(B_HOME, 0);
    CHECK(ui.overlay == O_NONE, "HOME cancels the part paste");
    CHECK(noted(6, 0, 2, 60), "cancel keeps the target part");
    pad(5, 0);
    pad(6, 0);
    pad(5, 1);
    pad(6, 1);
    CHECK(ui.overlay == O_ASK, "the occupied target asks again");
    btn(B_SEL, 1);
    btn(B_SEL, 0);
    CHECK(noted(6, 0, 2, 48), "confirm pastes the selected part");
    CHECK(has(6, PART_909, 3), "confirm leaves the other part");
    CHECK(noted(5, 0, 2, 48), "confirm leaves the source");
    put_i(put_s(put_s(pasted, part_name(PART_303A)), " PASTED TO P"), 7);
    expect_msg(pasted, "confirm names the selected part");
    pad(5, 0);
    pad(6, 0);
    btn(B_GLO, 0);

    home();
    ui.part = PART_BRK;
    proj.pat[7].brk.steps = 0x5;
    proj.pat[7].brk.slice[0] = 2;
    hit(7, PART_909, 9);
    btn(B_GLO, 1);
    pad(7, 1);
    pad(7, 0);
    CHECK(proj.pat[7].brk.steps == 0 && proj.pat[7].brk.slice[0] == 0, "GLOBE+pad clears the break");
    CHECK(has(7, PART_909, 9), "break clear leaves the drums");
    expect_msg("CLEARED BREAK P8", "break clear names BREAK");
    btn(B_GLO, 0);

    ui.pbank = 1;
    ui.part = PART_909;
    hit(16, PART_909, 2);
    hit(17, PART_909, 5);
    keep17 = proj.pat[17];
    btn(B_GLO, 1);
    pad(0, 1);
    pad(0, 0);
    CHECK(!has(16, PART_909, 2), "the second pattern bank clears P17");
    CHECK(!memcmp(&proj.pat[17], &keep17, sizeof keep17), "the next pattern in that bank stays");
    expect_msg("CLEARED 909 P17", "bank clear names P17");
    btn(B_GLO, 0);

    /* Two new pads in one scan have no earlier source, so this is not a copy. */
    home();
    ui.part = PART_808;
    hit(12, PART_808, 1);
    hit(13, PART_808, 2);
    btn(B_GLO, 1);
    held_keys |= 1u << WHITE_KEY[12];
    held_keys |= 1u << WHITE_KEY[13];
    tick(1);
    held_keys &= ~(1u << WHITE_KEY[12]);
    held_keys &= ~(1u << WHITE_KEY[13]);
    tick(1);
    CHECK(has(12, PART_808, 1), "a same-scan pair does not clear the earlier pad");
    CHECK(!has(13, PART_808, 1), "a same-scan pair does not copy onto the later pad");
    CHECK(!has(13, PART_808, 2), "a same-scan pair clears only the last new pad");
    CHECK(strncmp(ui.msg, "808 PASTED", 10) != 0, "a same-scan pair does not paste");
    btn(B_GLO, 0);

    home();
    hit(0, PART_909, 1);
    hit(0, PART_808, 3);
    note_on(0, 0, 1, 42);
    proj.pat[0].brk.steps = 0x2;
    for (row = 0; row < NPARTS; row++)
        seq.ppat[row] = 0;
    memset(&proj.pat[3], 0, sizeof proj.pat[3]);
    pattern_init(&proj.pat[3], 1, 2);
    btn(B_SAVE, 1);
    tick(300);
    pad(3, 1);
    pad(3, 0);
    CHECK(has(3, PART_909, 1) && has(3, PART_808, 3), "SAVE+pad still copies every part");
    CHECK(noted(3, 0, 1, 42) && proj.pat[3].brk.steps == 0x2, "SAVE+pad still copies bass and break");
    expect_msg("PARTS PASTED TO P4", "all-parts paste keeps its toast");
    btn(B_SAVE, 0);

    for (row = 0; row < NPARTS; row++)
        seq.ppat[row] = 9;
    hit(9, PART_909, 1);
    hit(9, PART_808, 2);
    note_on(9, 0, 0, 45);
    note_on(9, 1, 0, 47);
    proj.pat[9].brk.steps = 0x8;
    hit(10, PART_909, 7);
    keep10 = proj.pat[10];
    set_view(V_GLO);
    open_global();
    global_section(4);
    row = action_row(ACT_CLEAR_PAT);
    CHECK(row >= 0, "Global menu still contains CLEAR PATTERN");
    ui.list_sel = (uint8_t)row;
    list_enter();
    CHECK(ui.overlay == O_ASK, "CLEAR PATTERN still asks");
    CHECK(!strcmp(ui.ask_q[0], "CLEAR ALL ACTIVE PARTS") && !strcmp(ui.ask_q[1], "NOTES + MOTION LOST"),
          "CLEAR PATTERN names all active parts and their notes and motion");
    button_tap(B_SEL);
    CHECK(!has(9, PART_909, 1) && !has(9, PART_808, 2), "CLEAR PATTERN clears both drum parts");
    CHECK(!noted(9, 0, 0, 45) && !noted(9, 1, 0, 47), "CLEAR PATTERN clears both bass parts");
    CHECK(proj.pat[9].brk.steps == 0, "CLEAR PATTERN clears the break");
    CHECK(!memcmp(&proj.pat[10], &keep10, sizeof keep10), "CLEAR PATTERN leaves other patterns");
    expect_msg("PATTERN CLEARED", "CLEAR PATTERN keeps its toast");
    CHECK(help_fits(), "GLOBE help text fits the existing card");

    printf("home_globe_part: %s\n", fails ? "FAIL" : "ok");
    return fails != 0;
}
