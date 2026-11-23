/* SPDX-License-Identifier: GPL-3.0-only */
#define X0X_UI_PERF 0
#define SMC_UI_MAIN retained_fixture_main
#include "smc_ui_test.c"

int main(void)
{
    int about = 0, batt = 0;
    setup(V_HOME, PART_909);
    global_index();
    for (int g = 0; g < 6; ++g) {
        global_section(g);
        for (unsigned n = 0; n < ui.list_n; ++n) {
            pref_t r = list_rows[n];
            CHECK(r.kind != R_ACT || (r.a != ACT_PERF && r.a != ACT_PERF_TEST));
            about += r.kind == R_ACT && r.a == ACT_ABOUT;
            batt += r.kind == R_BATT;
        }
    }
    CHECK(about == 1 && batt == 1);
    CHECK(pref_desc(PR(R_ACT, ACT_PERF, 0, 0)) == NULL);
    CHECK(pref_desc(PR(R_ACT, ACT_PERF_TEST, 0, 0)) == NULL);
    CHECK(pref_desc(PR(R_STALLS, 0, 0, 0)) == NULL);
    run_action(ACT_PERF, 0);
    run_action(ACT_PERF_TEST, 0);
    CHECK(ui.view == V_HOME && !perf_testing());
    const char *help[HELP_LINES];
    for (int n = 0, count = help_lines(B_GLO, help); n < count; ++n)
        CHECK(strstr(help[n], "PERFORMANCE") == NULL);
    puts("stock no-perf: no benchmark or stalls route; About and Battery retained");
    return 0;
}
