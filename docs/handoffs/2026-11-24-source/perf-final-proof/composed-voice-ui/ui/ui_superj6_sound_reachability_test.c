/* SPDX-License-Identifier: GPL-3.0-only */
#define PWM_FIXTURE_MAIN retained_pwm_main
#include "ui_superj6_pwm1_lfo2_test.c"
static void select_detent(int dir)
{
    enc_acc[EN_SELECT] = dir;
    now_ms += 300;
    input();
}
static void tap(unsigned b)
{
    held_btn = 1u << b; input();
    now_ms += 50;
    held_btn = 0; input();
}
static void verify_query(void)
{
    for (int i=0; i<=pg.n; ++i) {
        page_query_t q = {.page=i};
        describe_pages(0, &q, ui.view, ui.part, ui.mixsel, ui.sel);
        CHECK(q.n == pg.n);
        if (i < pg.n) {
            CHECK(!strcmp(pg.title[i], q.title));
            CHECK(!memcmp(pg.r[i], q.r, sizeof q.r));
        }
    }
}
static void owner_path(unsigned v)
{
    setup(V_PART, PART_303A + v);
    select_owner(v, 0);
    build_pages();
    CHECK(pg.n == 15);
    verify_query();
    const unsigned start = cur_page();
    unsigned seen = 0;
    for (unsigned n=0; n<15; ++n) {
        const unsigned i = cur_page();
        CHECK(!(seen & (1u << i)));
        seen |= 1u << i;
        CHECK(i == (start+n)%15u);
        CHECK(strcmp(pg.title[i], "IMPORTED"));
        select_detent(1);
    }
    CHECK(seen == 0x7fff && cur_page() == (int)start);
    while (cur_page()) select_detent(-1);
    select_detent(-1);
    CHECK(cur_page() == 14 && !strcmp(pg.title[14], "SOUND"));
    CHECK(!strcmp(pg.title[13], "ARPEGGIATOR") && !strcmp(pg.title[12], "TUNING"));
    CHECK(pg.r[14][0].kind == R_BENGINE && pg.r[14][0].b == v);
    CHECK(pg.r[14][1].kind == R_BPRESET && pg.r[14][1].b == v);
    CHECK(pg.r[14][2].kind == R_BSTATUS && pref_read_only(pg.r[14][2]));
    CHECK(pg.r[14][3].kind == R_NONE);
    tap(B_SEL);
    CHECK(ui.overlay == O_LIST);
    tap(B_HOME);
    CHECK(ui.view == V_PART && ui.overlay == O_NONE && cur_page() == 14);
    select_detent(1);
    CHECK(cur_page() == 0);
    select_detent(-1);
    CHECK(cur_page() == 14);
    enc_acc[EN_K1+1] = 1; /* actual K2, first to second compact built-in */
    now_ms += 300; input();
    for (unsigned i=0; i<1024 && engine_bass_preset_pending(v); ++i) render(1);
    CHECK(engine_bass_requested_preset(v) == 1 && !engine_bass_preset_pending(v));
    CHECK(cur_page() == 14 && !strcmp(pg.title[cur_page()], "SOUND"));

    engine_note_lock_capture(1);
    CHECK(!engine_bass_note_capture(proj.arr.lane, 0, v, 0, &proj.sound));
    engine_note_lock_capture(0);
    CHECK(!engine_bass_note_validate(&proj.arr.lane[0]));
    engine_note_lock_refresh();
    CHECK(cur_page() == 14);
    verify_query();
    select_detent(1);
    CHECK(cur_page() == 0);
    select_detent(-1);
    CHECK(cur_page() == 14);
    printf("PASS owner=%u real 15-page SELECT wrap and backward wrap, overlay EXIT, SOUND K2, note-lock context\n", v);
}
int main(void)
{
    printf("descriptor bytes: pg=%zu sizeof(pref_t)=%zu query=%zu cap=%u\n",
           sizeof pg, sizeof(pref_t), sizeof(page_query_t), (unsigned)MAXPAGES);
    owner_path(0);
    owner_path(1);
    /* Exercise every stock view and visible part at least in initial engine
     * state. The descriptor query cannot expose a page lost by materialization. */
    for (int view=V_HOME; view<=V_GLO; ++view) {
        for (int part=PART_909; part<=PART_303B; ++part) {
            setup(view,part);
            verify_query();
        }
    }
    printf("PASS exact query/materialized pages and slots: %u checks\n", checks);
    return 0;
}
