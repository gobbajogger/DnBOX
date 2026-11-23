/* SPDX-License-Identifier: GPL-3.0-only */
#define SMC_UI_MAIN retained_fixture
#include "smc_ui_test.c"

static uint16_t pixel(int x, int y) { return swap16(cv_px[y * cv_w + x]); }

int main(void)
{
    uint16_t bg = RGB(9, 18, 24), ink = RGB(255, 240, 224);
    cv_begin(40, 40, bg);
    cv_line(2, 4, 17, 4, ink);
    cv_line(23, 18, 23, 2, ink);
    for (int x = 0; x < 40; x++) {
        CHECK(pixel(x, 4) == (x == 23 || (x >= 2 && x <= 17) ? ink : bg));
        CHECK(pixel(x, 3) == (x == 23 ? ink : bg));
        CHECK(pixel(x, 5) == (x == 23 ? ink : bg));
    }
    for (int slope = 1; slope < 10; slope += 4) {
        uint16_t plain[40 * 40];
        cv_begin(40, 40, bg);
        cv_line(4, 5, 13, 5 + slope, ink);
        memcpy(plain, cv_px, sizeof plain);
        cv_begin(40, 40, bg);
        cv_line(13, 5 + slope, 4, 5, ink);
        CHECK(!memcmp(plain, cv_px, sizeof plain));
        CHECK(pixel(4, 5) == ink);
        CHECK(pixel(13, 5 + slope) == ink);
        int partial = 0;
        for (unsigned i = 0; i < sizeof plain / sizeof plain[0]; i++)
            partial += plain[i] != swap16(bg) && plain[i] != swap16(ink);
        CHECK(partial > 0);
        cv_begin(40, 40, bg);
        cv_line(5, 4, 5 + slope, 13, ink);
        for (int y = 0; y < 40; y++)
            for (int x = 0; x < 40; x++)
                if (slope != 9 || (x > 5 && x < 12 && y > 6 && y < 13))
                    CHECK(cv_px[y * 40 + x] == plain[x * 40 + y]);
    }
    printf("SPATIAL LINE coverage, axes, reversal and transpose checks=%u PASS\n", checks);
}
