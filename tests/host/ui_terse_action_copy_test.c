/* SPDX-License-Identifier: GPL-3.0-only */
/* Real firmware menu and physical key routes, not fabricated message cards. */
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"

static unsigned copy_failures;
static void text_is(const char *actual, const char *expected)
{
    if (strcmp(actual, expected)) {
        fprintf(stderr, "FAIL copy: got [%s], expected [%s]\n", actual, expected);
        copy_failures++;
    }
}

static void buttons(uint32_t mask)
{
    held_btn = mask;
    ++now_ms;
    input();
}

static void tap(unsigned button)
{
    buttons(1u << button);
    buttons(0);
}

static void pad(unsigned index, int down)
{
    if (down) held_keys |= 1u << WHITE_KEY[index];
    else held_keys &= ~(1u << WHITE_KEY[index]);
    ++now_ms;
    input();
}

static void expect_modal(const char *frame, int act, int arg, const char *title, const char *warning)
{
    CHECK(ui.overlay == O_ASK && ui.ask_act == act && ui.ask_arg == arg);
    text_is(ui.ask_q[0], title);
    text_is(ui.ask_q[1], warning);
    CHECK(tw(&FONT_B, ui.ask_q[0]) <= 222 && tw(&FONT_S, ui.ask_q[1]) <= 222);
    ui_frame();
    shot(frame);
    tap(B_HOME);                          /* physical PATCH: reject */
    CHECK(ui.overlay != O_ASK);
}

static void select_action(int act)
{
    for (unsigned i = 0; i < ui.list_n; i++)
        if (list_rows[i].kind == R_ACT && list_rows[i].a == act) {
            ui.list_sel = (uint8_t)i;
            tap(B_SEL);
            return;
        }
    CHECK(0);
}

static void project_modal(int act, int section, const char *frame, const char *title, const char *warning)
{
    setup(V_HOME, PART_909);
    tap(B_GLO);
    CHECK(ui.view == V_GLO && ui.overlay == O_LIST);
    ui.list_sel = (uint8_t)section;
    tap(B_SEL);
    ui.dirty = 1;                         /* real unsaved load condition */
    select_action(act);
    expect_modal(frame, act, 0, title, warning);
}

static void check_split_content(const int lane[4], int cleared)
{
    /* The opposite pair in the very same slots is not active. */
    CHECK(sm_get(proj.pat[0].drum[PART_909].hit[0], 3) == !cleared);
    CHECK(sm_get(proj.pat[1].drum[PART_808].hit[0], 4) == !cleared);
    CHECK(sm_get(proj.pat[1].drum[PART_909].hit[0], 5));
    CHECK(sm_get(proj.pat[0].drum[PART_808].hit[0], 6));
    CHECK(proj.arr.lane[lane[0]].used == !cleared);
    CHECK(proj.arr.lane[lane[1]].used == !cleared);
    CHECK(proj.arr.lane[lane[2]].used && proj.arr.lane[lane[2]].val[0] == 43);
    CHECK(proj.arr.lane[lane[3]].used && proj.arr.lane[lane[3]].val[0] == 44);
    CHECK(seq.ppat[PART_909] == 0 && seq.ppat[PART_808] == 1);
}

static void clear_active_parts(void)
{
    static const int part[4] = {PART_909, PART_808, PART_909, PART_808};
    static const int pat[4] = {0, 1, 1, 0};
    int lane[4];

    setup(V_HOME, PART_909);
    seq.ppat[PART_909] = 0;
    seq.ppat[PART_808] = 1;
    for (int k = 0; k < 4; k++) {
        sm_set(proj.pat[pat[k]].drum[part[k]].hit[0], k + 3);
        lane[k] = motion_alloc(proj.arr.lane, pat[k], part[k], T_MIX, part[k], MX_LEVEL);
        CHECK(lane[k] >= 0);
        proj.arr.lane[lane[k]].val[0] = (uint8_t)(41 + k);
    }

    tap(B_GLO);
    CHECK(ui.view == V_GLO && ui.overlay == O_LIST);
    ui.list_sel = 4;
    tap(B_SEL);
    select_action(ACT_CLEAR_PAT);
    expect_modal("clear-pattern", ACT_CLEAR_PAT, 0, "CLEAR ALL ACTIVE PARTS", "NOTES + MOTION LOST");
    check_split_content(lane, 0);

    select_action(ACT_CLEAR_PAT);
    tap(B_SEL);                         /* physical SCALE/SEL: confirm */
    CHECK(ui.overlay != O_ASK);
    check_split_content(lane, 1);
}

int main(void)
{
    setup(V_SEQ, PART_909);
    buttons(1u << B_SEQ);
    now_ms += HELP_HOLD;
    input();
    CHECK(ui.help == B_SEQ + 1);
    const char *lines[HELP_LINES];
    int n = help_lines(B_SEQ, lines), i;
    for (i = 0; i < n; i++)
        if (!strncmp(lines[i], "SEQ + " UI_CAP_SEL ": ", sizeof("SEQ + " UI_CAP_SEL ": ") - 1)) break;
    CHECK(i == n);
    ui_frame(); shot("seq-help");
    buttons(0);
    unsigned lane = ui_step_lane;
    buttons(1u << B_SEQ);
    buttons((1u << B_SEQ) | (1u << B_SEL));
    buttons(1u << B_SEQ);
    buttons(0);
    CHECK(ui.view == V_SEQ && ui_step_lane == (lane + 1) % UI_LANE_COUNT);

    setup(V_HOME, PART_808);
    tap(B_ARP);
    CHECK(ui.view == V_HOME && ui.overlay == O_NONE);
    text_is(ui.msg, "TB-3PO UNAVAILABLE");
    ui_frame(); shot("arp-inactive");
    setup(V_HOME, PART_303A);
    tap(B_ARP);
    CHECK(ui.view == V_GEN);               /* allowed route remains available */

    clear_active_parts();
    project_modal(ACT_RESET, 4, "reset-project", "RESET PROJECT", "SAMPLES KEPT; EDITS LOST");
    project_modal(ACT_LOAD, 4, "load-project", "LOAD SAVED PROJECT", "DISCARD UNSAVED EDITS");

    setup(V_SONG, PART_909);
    tap(B_SEL);
    CHECK(ui.overlay == O_LIST);
    song()->len = 2;
    sm_set(proj.pat[2].drum[PART_909].hit[0], 3);
    select_action(ACT_SONG_CLR);
    expect_modal("clear-song", ACT_SONG_CLR, 0, "CLEAR SONG", "BARS REMOVED; PATTERNS KEPT");
    CHECK(song()->len == 2 && sm_get(proj.pat[2].drum[PART_909].hit[0], 3));
    select_action(ACT_SONG_CLR);
    tap(B_SEL);                         /* physical SCALE: confirm */
    CHECK(song()->len == 0 && sm_get(proj.pat[2].drum[PART_909].hit[0], 3));

    setup(V_HOME, PART_909);
    buttons(1u << B_SAVE);
    buttons((1u << B_SAVE) | (1u << B_REC));
    buttons(1u << B_SAVE);
    buttons(0);
    expect_modal("clear-part", ACT_CLEAR_PART, PART_909, "CLEAR 909", "PATTERN 1");

    setup(V_HOME, PART_909);
    for (unsigned p = 0; p < NPARTS; p++) seq.ppat[p] = 0;
    sm_set(proj.pat[1].drum[PART_909].hit[0], 0);
    buttons(1u << B_SAVE);
    pad(1, 1); pad(1, 0);
    buttons(0);
    expect_modal("paste-all-over", ACT_COPY, 1, "PASTE OVER P2", "PART DATA REPLACED");
    CHECK(sm_get(proj.pat[1].drum[PART_909].hit[0], 0));

    const int selected[] = {PART_909, PART_808, PART_BRK, PART_303A};
    const char *shots[] = {"paste-part-over", "paste-part-808", "paste-part-break", "paste-part-bass"};
    for (unsigned i=0; i<sizeof(selected)/sizeof(selected[0]); i++) {
        const int part = selected[i];
        setup(V_HOME, part);
        for (unsigned p = 0; p < NPARTS; p++) seq.ppat[p] = 0;
        if (part_is_drum(part))
            sm_set(proj.pat[1].drum[part].hit[0], 0);
        else if (part == PART_BRK)
            proj.pat[1].brk.steps = 1;
        else {
            proj.pat[1].bass[0].step[0].note = 60;
            proj.pat[1].bass[0].step[0].flags = G_NOTE;
        }
        char warning[28];
        snprintf(warning, sizeof(warning), "%s PART REPLACED", part_name(part));
        buttons(1u << B_GLO);
        pad(0, 1); pad(1, 1); pad(1, 0); pad(0, 0);
        buttons(0);
        expect_modal(shots[i], ACT_COPY, 1, "PASTE OVER P2", warning);
        CHECK(part_used(&proj.pat[1], part));
    }

    printf("Terse action copy: 12 real routes, %u copy mismatches\n", copy_failures);
    return copy_failures ? 1 : 0;
}
