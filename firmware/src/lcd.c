/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments
 * SMK-37 Pro port: Copyright (C) 2026 M. Geisler (genkigenki/makkuro) */
/* ST7789-class 240x240 panel on SPI1: PC9 CLK, PC10 DO, PC7 CS, PC8 D/C,
 * backlight PA2 active low (SMK-37 Pro: see hal/fm1_lcd_hw.h; its 16 RGB pad LEDs are at the end).
 * Polled DMA transfers; every source buffer must be in RAM. A pixel transfer is left running (lcd_busy): the next LCD access, or
 * a write to its source buffer (lcd_sync), waits for it, so the main loop works
 * while the last strip of a frame goes out. */
/* pins and SPI1: hal/fm1_lcd_hw.h */
#ifndef LCD_BAUD
#if BOARD_SMK37
#define LCD_BAUD 4u                /* SMK-37 Pro: 4 = 12 MHz, the stock firmware's rate on its panel and wiring
                                    * (the 60 MHz below is measured on the FM-1 only; X0X_SMK_LCD_BAUD tries
                                    * another) */
#else
#define LCD_BAUD 0u                /* lsb/(BAUD+1): 0 = 60 MHz (~44 MHz effective, measured). Felucca's
                                    * 4 (12 MHz) took 23 ms for a 240x72 strip, longer than the panel's
                                    * 16.7 ms refresh: tearing. ST7789: <= ~62 MHz. The FM-1 has no TE
                                    * line; the panel's scanline can be read (RDSCL on the data wire), but
                                    * timing writes to it did not stop the tearing on hardware. */
#endif
#endif

enum { LCD_WINDOW_STAGE_BYTES = 8u, LCD_CMD_STAGE_BYTES = 16u };
static uint8_t lcd_small[LCD_CMD_STAGE_BYTES] __attribute__((aligned(16)));
_Static_assert(LCD_WINDOW_STAGE_BYTES <= sizeof lcd_small, "CASET and RASET staging overlap");
/* Derive the actual byte count, rejecting oversized payloads at compile time. */
#define LCD_SEQ_DATA(cmd, ...) \
    (cmd), (sizeof((const uint8_t[]){__VA_ARGS__}) + \
    0u * sizeof(char[sizeof((const uint8_t[]){__VA_ARGS__}) <= LCD_CMD_STAGE_BYTES ? 1 : -1])), __VA_ARGS__
static uint8_t lcd_busy;           /* a lcd_data DMA may still run; CS is low */

static void lcd_spin(uint32_t n)
{
    for (volatile uint32_t i = 0; i < n; i++)
        ;
}

static void lcd_sync(void)          /* finish the running transfer (before touching its buffer) */
{
    if (!lcd_busy)
        return;
    fm1_lcd_wait();
    fm1_lcd_deselect();
#if X0X_LCD_TRACE
    lcd_trace_dma_done();
#endif
    lcd_busy = 0;
}

static void lcd_cmd(uint8_t c)
{
    lcd_sync();
    fm1_lcd_send_cmd(c);
    fm1_lcd_wait();
    fm1_lcd_deselect();
}

static void lcd_data(const void *p, uint32_t n)
{
    if (!n)
        return;                    /* SPI_CNT = 0 never completes */
    lcd_sync();
#if X0X_LCD_TRACE
    lcd_trace_dma_begin(n);
#endif
    fm1_lcd_send_data(p, n);
    lcd_busy = 1;                  /* completed by lcd_sync */
}

static void lcd_window(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1)
{
    lcd_sync();
    lcd_small[0] = (uint8_t)(x0 >> 8);
    lcd_small[1] = (uint8_t)x0;
    lcd_small[2] = (uint8_t)(x1 >> 8);
    lcd_small[3] = (uint8_t)x1;
    lcd_cmd(0x2A);
    lcd_data(lcd_small, 4);
    lcd_small[4] = (uint8_t)(y0 >> 8);       /* own bytes: the x transfer may still read [0..3] */
    lcd_small[5] = (uint8_t)y0;
    lcd_small[6] = (uint8_t)(y1 >> 8);
    lcd_small[7] = (uint8_t)y1;
    lcd_cmd(0x2B);
    lcd_data(lcd_small + 4, 4);
    lcd_cmd(0x2C);
}

/* Fill and dirty tiles use the same single SPI DMA. Both writers call
 * lcd_sync before touching this memory. Neither retains its data afterward. */
#include "app/lcd_tiles.h"
static uint16_t lcd_txbuf[X0X_LCD_MAX_DMA_BYTES / 2u];
_Static_assert(sizeof lcd_txbuf >= 240u * sizeof(uint16_t), "LCD fill scanline must fit");

static void lcd_fill(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint16_t c)
{
    uint32_t i, n, k, sw = (uint16_t)((c >> 8) | (c << 8));
    if (!w || !h || x >= 240u || y >= 240u)
        return;
    if (x + w > 240u)
        w = 240u - x;
    if (y + h > 240u)
        h = 240u - y;
    lcd_sync();
    k = 240u / w * w;                       /* whole rows per transfer (narrow fills: one DMA) */
    for (i = 0; i < k; i++)
        lcd_txbuf[i] = (uint16_t)sw;
    lcd_window(x, y, x + w - 1u, y + h - 1u);
    for (n = w * h; n; n -= k) {
        if (k > n)
            k = n;
        lcd_data(lcd_txbuf, k * 2u);
    }
}

static void lcd_blit(uint32_t x, uint32_t y, uint32_t w, uint32_t h, const uint16_t *px)
{
    if (!w || !h)
        return;
    lcd_window(x, y, x + w - 1u, y + h - 1u);
#if X0X_LCD_TRACE
    lcd_trace_pixels(w * h * 2u);
#endif
    lcd_data(px, w * h * 2u);
}

#if BOARD_SMK37
/* SMK-37 Pro ST7789V 240x240: the stock v016 table with a SWRESET in
 * front (stock relies on the power-on reset) and NORON instead of its DISPOFF + 50 ms;
 * lcd_init sends DISPON after the clear. Same format as below. */
static const uint8_t LCD_SEQ[] = {
    0x01, 0,                     /* SWRESET */
    LCD_SEQ_DATA(0x00, 150),
    0x11, 0,                     /* SLPOUT */
    LCD_SEQ_DATA(0x00, 120),
    LCD_SEQ_DATA(0x36, 0x00),               /* MADCTL: top-left origin, RGB order */
    LCD_SEQ_DATA(0x3A, 0x55),               /* COLMOD: RGB565 */
    LCD_SEQ_DATA(0xB2, 0x0C, 0x0C, 0x00, 0x33, 0x33),
    LCD_SEQ_DATA(0xB7, 0x35),
    LCD_SEQ_DATA(0xBB, 0x32),
    LCD_SEQ_DATA(0xC2, 0x01),
    LCD_SEQ_DATA(0xC3, 0x15),
    LCD_SEQ_DATA(0xC4, 0x20),
    LCD_SEQ_DATA(0xC6, 0x0F),
    LCD_SEQ_DATA(0xD0, 0xA4, 0xA1),
    LCD_SEQ_DATA(0xE0, 0xD0, 0x08, 0x0E, 0x09, 0x09, 0x05, 0x31, 0x33, 0x48, 0x17, 0x14, 0x15, 0x31, 0x34),
    LCD_SEQ_DATA(0xE1, 0xD0, 0x08, 0x0E, 0x09, 0x09, 0x05, 0x31, 0x33, 0x48, 0x17, 0x14, 0x15, 0x31, 0x34),
    0x21, 0,                     /* INVON: IPS panel */
    0x13, 0,                     /* NORON */
};
#else
/* ST7789V init: reset, then 16-bit colour, scan direction and IPS inversion;
 * voltage and gamma stay at the defaults. Format: cmd, n, n data bytes;
 * cmd 0x00 = wait (data byte: ~ms). */
static const uint8_t LCD_SEQ[] = {
    0x01, 0,                     /* SWRESET */
    LCD_SEQ_DATA(0x00, 150),
    0x11, 0,                     /* SLPOUT */
    LCD_SEQ_DATA(0x00, 120),
    LCD_SEQ_DATA(0x3A, 0x55),               /* COLMOD: RGB565 */
    LCD_SEQ_DATA(0x36, 0x00),               /* MADCTL: top-left origin, RGB order */
    0x21, 0,                     /* INVON: IPS panel */
    0x13, 0,                     /* NORON */
};
#endif
#undef LCD_SEQ_DATA

static void lcd_init(void)
{
    uint32_t r, x;
    fm1_lcd_hw_init();
    lcd_spin(2000000u);
    for (r = 0; r < sizeof LCD_SEQ; r += 2u + LCD_SEQ[r + 1u]) {
        if (LCD_SEQ[r] == 0x00u) {                       /* pseudo command: wait */
            lcd_spin(LCD_SEQ[r + 2u] * 25000u);          /* ~1 ms per unit (lcd_spin(3000000) ~ 120 ms) */
            continue;
        }
        lcd_cmd(LCD_SEQ[r]);
        for (x = 0; x < LCD_SEQ[r + 1u]; x++)
            lcd_small[x] = LCD_SEQ[r + 2u + x];
        if (LCD_SEQ[r + 1u])
            lcd_data(lcd_small, LCD_SEQ[r + 1u]);
    }
    fm1_lcd_baud(LCD_BAUD);
    lcd_fill(0, 0, 240, 240, 0);
    lcd_cmd(0x29);
}

#if BOARD_SMK37
/* The 16 RGB pads (SMK-37 Pro): WS2812-class LEDs on SPI1 group B (PC10), one SPI byte per
 * LED bit (0xF0 = 1, 0xC0 = 0, MSB first, GRB), 250 zero bytes of reset first (~300 us; the
 * stock firmware's 36 bytes = 43 us are too short for newer WS2812 parts: a glitch on PC10 at the
 * pin switch then counts as a first bit and shifts the chain by one, so a lit pad turns its chain
 * neighbour green). The frame goes out between LCD transfers (LCD CS high), blocking for ~0.8 ms;
 * padled_flush() only sends when a colour changed. */
#define PADLED_N 16u
#define PADLED_LATCH 250u
static uint8_t padled_buf[PADLED_LATCH + PADLED_N * 24u] SMK_BIG;   /* (zeroed at boot: the latch stays 0) */
static uint32_t padled_rgb[PADLED_N];
static uint8_t padled_dirty = 1;
/* hardware pad (0..7 top row, 8..15 bottom row, left to right; smk_input.h SMK_PAD_OF_WHITE) -> position in
 * the chain (stock table) */
static const uint8_t PADLED_SLOT[PADLED_N] = {0, 15, 14, 13, 12, 11, 10, 9, 1, 2, 3, 4, 5, 6, 7, 8};

static void padled_set(uint32_t pad, uint8_t r, uint8_t g, uint8_t b)
{
    uint32_t c = (uint32_t)g << 16 | (uint32_t)r << 8 | b;
    if (pad < PADLED_N && padled_rgb[pad] != c) {
        padled_rgb[pad] = c;
        padled_dirty = 1;
    }
}

/* a76: the GRB colour of a hardware pad in the last frame sent (padled_buf), for ED_INLOG */
static uint32_t padled_sent(uint32_t pad)
{
    const uint8_t *o = padled_buf + PADLED_LATCH + PADLED_SLOT[pad % PADLED_N] * 24u;
    uint32_t v = 0, i;
    for (i = 0; i < 24u; i++)
        v = v << 1 | (o[i] == 0xF0u);
    return v;
}

static void padled_flush(void)
{
    uint32_t p, i;
    if (!padled_dirty)
        return;
    padled_dirty = 0;
    for (p = 0; p < PADLED_N; p++) {
        uint8_t *o = padled_buf + PADLED_LATCH + PADLED_SLOT[p] * 24u;
        for (i = 0; i < 24u; i++)
            o[i] = (padled_rgb[p] >> (23u - i)) & 1u ? 0xF0u : 0xC0u;
    }
    lcd_sync();
    fm1_padled_send(padled_buf, sizeof padled_buf);
    fm1_lcd_wait();
    fm1_padled_done();
}
#endif
