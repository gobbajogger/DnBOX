/* SPDX-License-Identifier: GPL-3.0-only */
/* XDATA fonts (gfx.c font_live): FONT_S/M/XS/B are one blob in generator order (the region the update
 * loader installs at 0xAD000); FONT_T and FONT_L stay in the app. With the region valid every face draws
 * itself; with a bad CRC / build id (gfx_xdata_ok 0, as xdata_check leaves it) the four XDATA faces draw
 * and measure exactly as FONT_T, and FONT_T / FONT_L are unchanged. Build: -Ibuild/gen (tools/gen_font.py). */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static void lcd_sync(void) {}
static void lcd_blit(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const uint16_t *p)
{ (void)x; (void)y; (void)w; (void)h; (void)p; }
#ifdef __APPLE__
#define __attribute__(x)
#endif
#include "../../firmware/src/gfx.c"
#ifdef __APPLE__
#undef __attribute__
#endif

static const char TXT[] = "DnBOX 0123 BREAK pad 7/8 -?";
static uint16_t shot[240 * 40];

static void draw(const felucca_font_t *f)
{
    cv_begin(240, 40, 0);
    cv_text(2, 4, f, TXT, 0xFFFFu);
    memcpy(shot, cv_px, sizeof shot);
}

int main(void)
{
    const felucca_font_t *const xd[] = {&FONT_S, &FONT_M, &FONT_XS, &FONT_B};
    uint32_t k, at = 0;
    static uint16_t ref[240 * 40];
    /* the blob: S, M, XS, B back to back, its CRC is the build id the header carries */
    for (k = 0; k < 4u; k++) {
        assert(xd[k]->data == FONT_XDATA_BLOB + at);
        at += (uint32_t)(k < 3u ? xd[k + 1]->data - xd[k]->data : FONT_XDATA_LEN - at);
    }
    assert(at == FONT_XDATA_LEN);
    assert(x0x_crc32(FONT_XDATA_BLOB, FONT_XDATA_LEN) == FONT_XDATA_CRC32);
    assert(!(FONT_T.data >= FONT_XDATA_BLOB && FONT_T.data < FONT_XDATA_BLOB + FONT_XDATA_LEN));
    assert(!(FONT_L.data >= FONT_XDATA_BLOB && FONT_L.data < FONT_XDATA_BLOB + FONT_XDATA_LEN));
    /* valid region (the host default): each face is itself */
    assert(gfx_xdata_ok == 1u);
    for (k = 0; k < 4u; k++)
        assert(font_live(xd[k]) == xd[k]);
    draw(&FONT_T);
    memcpy(ref, shot, sizeof ref);
    draw(&FONT_M);
    assert(memcmp(ref, shot, sizeof ref) != 0);           /* M really differs from T */
    /* bad region: the four XDATA faces are FONT_T, pixel for pixel and in width */
    gfx_xdata_ok = 0;
    for (k = 0; k < 4u; k++) {
        assert(font_live(xd[k]) == &FONT_T);
        draw(xd[k]);
        assert(memcmp(ref, shot, sizeof ref) == 0);
        assert(text_w(xd[k], TXT) == text_w(&FONT_T, TXT));
    }
    assert(font_live(&FONT_T) == &FONT_T && font_live(&FONT_L) == &FONT_L);
    puts("XDATA fonts: S/M/XS/B blob in order, CRC = build id; bad region draws FONT_T exactly PASS");
    return 0;
}
