/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments */
/* Small-canvas renderer (no full framebuffer). Draw text/lines into
 * an off-screen strip, then blit it in one DMA transfer. Pixels are stored
 * byte-swapped (the panel takes RGB565 big-endian). */
typedef struct { int8_t top, bot; } capm_t;

typedef struct {               /* proportional, see tools/gen_font.py */
    uint8_t h;
    uint8_t pad;               /* bitmap starts this many pixels left of the pen */
    uint8_t first, last;
    const uint8_t *adv;        /* advance per glyph */
    const uint8_t *bw;         /* bitmap width per glyph (starts FONT_PAD left of the pen) */
    const uint16_t *off;       /* glyph offset, optionally with advance in its upper five bits */
    const uint8_t *data;
    capm_t cap;                /* rows containing at least half-covered H (or 0) pixels */
} felucca_font_t;
/* FONT_S/M/XS/B bitmaps live outside the app, in the XDATA flash region (FL_XDATA, 0xAD000: tools/build.py
 * links .xdata.font there and installs it with a 16 B header). Host builds keep them in memory. */
#if defined(__clang__) && !defined(X0X_HOST) && BOARD_SMK37 && !defined(X0X_XDATA_IN_APP)
#define FONT_XDATA_SECTION __attribute__((section(".xdata.font"), used))
#define GFX_XDATA_OK0 0u           /* firmware: FONT_T until xdata_check() has read the header */
#else
#define GFX_XDATA_OK0 1u
#endif
#include "felucca_font.h"
#include "app/xdata.h"
#ifdef FONT_XDATA_LEN
static uint8_t gfx_xdata_ok = GFX_XDATA_OK0;   /* 0: draw XDATA faces with FONT_T (bad/missing region) */
/* the face actually drawn: an XDATA face falls back to FONT_T (always in the app) when the region failed */
static inline const felucca_font_t *font_live(const felucca_font_t *f)
{
    if (!gfx_xdata_ok && f->data >= FONT_XDATA_BLOB && f->data < FONT_XDATA_BLOB + FONT_XDATA_LEN)
        return &FONT_T;
    return f;
}
#if !GFX_XDATA_OK0
/* the region's header, linked at FL_XDATA_LO ahead of the blob (app.ld .xdata; tools/build.py checks it) */
static const uint32_t FONT_XDATA_HDR[4] __attribute__((section(".xdata.0hdr"), used, aligned(4))) =
    { XDATA_MAGIC, FONT_XDATA_LEN, FONT_XDATA_CRC32, FONT_XDATA_CRC32 };
/* boot, after fl_plain_window_init (the region reads as plaintext from then on): trust XDATA or not */
static void xdata_check(void)
{
    gfx_xdata_ok = xdata_valid((const uint8_t *)FL_XIP(FL_XDATA_LO), FL_XDATA_HI - FL_XDATA_LO,
                               FONT_XDATA_LEN, FONT_XDATA_CRC32);
}
#else
static void xdata_check(void) {}
#endif
#else
#define font_live(f) (f)
static uint8_t gfx_xdata_ok = 1;
static void xdata_check(void) {}
#endif

static inline uint32_t font_offset(const felucca_font_t *f, uint32_t gi)
{
#ifdef FONT_INDEX16
    return f->off[gi] & 2047u;
#else
    return f->off[gi];
#endif
}
static inline uint32_t font_advance(const felucca_font_t *f, uint32_t gi)
{
#ifdef FONT_INDEX16
    return f->off[gi] >> 11;
#else
    return f->adv[gi];
#endif
}

/* Null bw denotes a generation-verified advance+padding width. Zero-advance
 * blank glyphs retain zero width. Other font families keep their exact table. */
static inline uint32_t font_bitmap_width(const felucca_font_t *f, uint32_t gi)
{
    uint32_t advance = font_advance(f,gi);
    return f->bw ? f->bw[gi] : advance ? advance + 2u * f->pad : 0u;
}

/* One strip, not a 240x240 frame. The panel is three strips that are never
 * drawn at the same time: header 20, main area 2*69 = 138, knob row 82
 * (20+138+82 = 240). The tallest is the main area. The strip used to be 144
 * rows; the extra 6 moved into the knob row and were still reserved here
 * (240 * 6 * 2 = 2880 bytes of the pool). */
#ifndef X0X_UI_SMALL_CANVAS
#define X0X_UI_SMALL_CANVAS 0
#endif
#if X0X_UI_SMALL_CANVAS
#define CV_ROWS 82u
#else
#define CV_ROWS 138u
#endif
#define CV_MAX (240u * CV_ROWS)
_Static_assert(CV_ROWS >= 82u, "screen strip shorter than the knob row");
static uint16_t cv_px[CV_MAX] __attribute__((section(".pool")));
static uint32_t cv_w, cv_h;
#if X0X_UI_SMALL_CANVAS
static uint32_t cv_row_base;
#else
#define cv_row_base 0u
#endif
static int32_t cv_oy;            /* y offset for graph drawing */
static int32_t cv_y0, cv_y1;     /* X0X: the rows drawing may touch (cv_band) */

#define RGB(r, g, b) ((uint16_t)((((r) >> 3) << 11) | (((g) >> 2) << 5) | ((b) >> 3)))
#include "ui_theme.h"
#define C_BLACK UI_N_BASE
#define C_WHITE UI_N_WHITE           /* what is being touched / where we are */
/* The screen's five steps, darkest to brightest, plus white; picked as THEME in GLOBAL. */
typedef struct {
    const char *name;
    uint16_t c[5];
} palette_t;
/* X0X: the first three steps are neutral greys in every theme (structure: rules, empty cells,
 * labels); only the two brightest carry the theme's hue (values, highlights). Colour on the
 * screen then means something: a part, a value, an alarm. */
#define NEUTRALS RGB(40,56,61), RGB(128,146,148), RGB(156,170,168)
static const palette_t PALETTES[] = {
    {"GREEN", {NEUTRALS, RGB(80, 220, 120), RGB(150, 255, 175)}},
    {"AMBER", {NEUTRALS, RGB(232, 136, 30), RGB(255, 184, 70)}},
    {"CYAN", {NEUTRALS, RGB(60, 180, 235), RGB(150, 225, 255)}},
    {"RED", {NEUTRALS, RGB(232, 76, 60), RGB(255, 132, 112)}},
    {"MONO", {NEUTRALS, RGB(196, 196, 202), RGB(236, 236, 240)}},
    {"LIGHT", {RGB(180,193,180), RGB(80,97,77), RGB(64,81,62), RGB(130,84,14), RGB(36,91,43)}},
};
#define NPALETTES (sizeof(PALETTES) / sizeof(PALETTES[0]))
static uint16_t pal[5];
#define C_LINE pal[0]                /* 1 rules, separators */
#define C_DIM pal[1]                 /* 2 inactive, empty steps, units */
#define C_GRAY pal[2]                /* 3 labels */
#define C_AMB pal[3]                 /* 4 secondary text */
#define C_HI pal[4]                  /* 5 values, curves */

static void palette_set(uint32_t i)
{
    uint32_t k;
    ui_theme_light = i % NPALETTES == 5u;
    for (k = 0; k < 5u; k++)
        pal[k] = PALETTES[i % NPALETTES].c[k];
}

static inline uint16_t swap16(uint32_t c) { return (uint16_t)(((c >> 8) & 0xFFu) | ((c & 0xFFu) << 8)); }

/* Exact for every RGB565 blend numerator (0..63*15). */
static inline uint32_t cv_div15(uint32_t x) { return (x * 0x8889u) >> 19; }

static void cv_begin(uint32_t w, uint32_t h, uint16_t bg)
{
    uint32_t i, n;
    uint32_t fill = swap16(bg);
    if (w * h > CV_MAX)
        h = CV_MAX / w;
    lcd_sync();                     /* the last blit may still read cv_px */
#if X0X_UI_SMALL_CANVAS
    cv_row_base = 0;
#endif
    cv_w = w;
    cv_h = h;
    cv_oy = cv_y0 = 0;
    cv_y1 = (int32_t)h;
    n = w * h;
    i = 0;
    if (n && ((uintptr_t)cv_px & 3u))
        cv_px[i++] = (uint16_t)fill;
    fill |= fill << 16;
    /* Copy object bytes: the host unity build strips type attributes. */
    for (; i + 1u < n; i += 2u)
        __builtin_memcpy(__builtin_assume_aligned(cv_px + i, 4), &fill, sizeof fill);
    if (i < n)
        cv_px[i] = (uint16_t)fill;
}

/* X0X: draw the next things as if the canvas were rows y0 .. y0+h-1 alone (one band of a
 * canvas that holds several, sent in one transfer so the bands change on screen together) */
static void cv_band(int32_t y0, int32_t h)
{
    cv_oy = cv_y0 = y0;
    cv_y1 = y0 + h;
#if X0X_UI_SMALL_CANVAS
    if (cv_y0 < (int32_t)cv_row_base) cv_y0 = (int32_t)cv_row_base;
    if (cv_y1 > (int32_t)(cv_row_base + cv_h)) cv_y1 = (int32_t)(cv_row_base + cv_h);
#endif
}

#if X0X_UI_SMALL_CANVAS
/* Raster in logical coordinates, backed by just one contiguous window. */
static void cv_window(uint32_t w, uint32_t logical_h, uint32_t y0, uint32_t rows, uint16_t bg)
{
    if (y0 > logical_h) y0 = logical_h;
    if (rows > logical_h - y0) rows = logical_h - y0;
    cv_begin(w, rows, bg);
    cv_row_base = y0;
    cv_y0 = (int32_t)y0;
    cv_y1 = (int32_t)(y0 + cv_h);
}
#endif

static void cv_blit(uint32_t x, uint32_t y) { lcd_blit(x, y, cv_w, cv_h, cv_px); }

/* canvas rows r0 .. cv_h-1 only, to screen row y + r0 */
static void cv_blit_from(uint32_t x, uint32_t y, uint32_t r0)
{
    if (r0 < cv_h)
        lcd_blit(x, y + r0, cv_w, cv_h - r0, cv_px + r0 * cv_w);
}

static inline void cv_pset(int32_t x, int32_t y, uint16_t c)
{
    y += cv_oy;
    if ((uint32_t)x < cv_w && y >= cv_y0 && y < cv_y1)
        cv_px[((uint32_t)y - cv_row_base) * cv_w + (uint32_t)x] = swap16(c);
}

static void cv_rect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t c)
{
    int32_t x1 = x + w, y1 = y + h + cv_oy, i;
    uint16_t sc = swap16(c);
    y += cv_oy;                             /* clipped once, then filled row by row */
    if (x < 0)
        x = 0;
    if (y < cv_y0)
        y = cv_y0;
    if (x1 > (int32_t)cv_w)
        x1 = (int32_t)cv_w;
    if (y1 > cv_y1)
        y1 = cv_y1;
    for (; y < y1; y++)
        for (i = x; i < x1; i++)
            cv_px[((uint32_t)y - cv_row_base) * cv_w + (uint32_t)i] = sc;
}

#if defined(__clang__) && !defined(X0X_HOST)
__attribute__((minsize, noinline))
#endif
static void px_blend(int32_t x, int32_t y, uint16_t c, uint32_t a)
{
    uint16_t *q;
    uint32_t bg;
    y += cv_oy;
    if ((uint32_t)x >= cv_w || y < cv_y0 || y >= cv_y1 || !a)
        return;
    q = &cv_px[((uint32_t)y - cv_row_base) * cv_w + (uint32_t)x];
    if (a >= 16u) {
        *q = swap16(c);
        return;
    }
    bg = swap16(*q);
    *q = swap16((uint32_t)(((((c >> 11) * a + (bg >> 11) * (16u - a)) >> 4) << 11) |
                           (((((c >> 5) & 63u) * a + ((bg >> 5) & 63u) * (16u - a)) >> 4) << 5) |
                           (((c & 31u) * a + (bg & 31u) * (16u - a)) >> 4)));
}

static int cv_abs(int value) { return value < 0 ? -value : value; }

/* Integer 1 px strokes. Keep axis-aligned pixels solid. On a slope, the
 * projected filter footprint spans at most three pixels on the minor axis. */
#if defined(__clang__) && !defined(X0X_HOST)
__attribute__((minsize, noinline))
#endif
static void cv_line(int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint16_t c)
{
    int dx = x1 - x0, dy = y1 - y0;
    if (!dx || !dy) {
        cv_rect(dx < 0 ? x1 : x0, dy < 0 ? y1 : y0,
                cv_abs(dx) + 1, cv_abs(dy) + 1, c);
        return;
    }
    int steep = cv_abs(dy) > cv_abs(dx);
    int major = steep ? y0 : x0, end = steep ? y1 : x1;
    int minor = steep ? x0 : y0, last = steep ? x1 : y1;
    if (major > end) {
        int m = major;
        major = end;
        end = m;
        m = minor;
        minor = last;
        last = m;
    }
    int n = end - major;
    int advance = ((last - minor) * 65536) / n;
    int falloff = 16 - cv_abs(advance) / 16384;
    int p = minor * 65536;
    for (int i = 0; i <= n && i < 2000; i++, p += advance) {
        int at = i == n ? last : (p + 32768) >> 16;
        int delta = i == n ? 0 : p - at * 65536;
        for (int off = -1; off <= 1; off++) {
            int alpha = 16 - ((cv_abs(delta - off * 65536) * falloff + 32768) >> 16);
            if (alpha > 0)
                px_blend(steep ? at + off : major + i, steep ? major + i : at + off, c, (uint32_t)alpha);
        }
    }
}

/* glyph index of a character: lower case folds to upper case when the font
 * has none, anything missing (and C1 controls) draws as '?' */
static uint32_t glyph(const felucca_font_t *f, uint32_t ch)
{
    if (ch >= 'a' && ch <= 'z' && f->last < 'a')
        ch -= 32u;
    if (ch < f->first || ch > f->last || (ch >= 127u && ch < 160u))
        ch = '?';
    return ch - f->first;
}

#if defined(FONT_ALPHA_CROP) || defined(FONT_LZ_CROP)
/* Internal normalized crop descriptor, consumed by both the painter and its tests. */
static uint32_t font_crop_read(const uint8_t **gp)
{
    const uint8_t *d=*gp;
#ifdef FONT_ALPHA_CROP
    uint32_t n=(uint32_t)d[0]|((uint32_t)d[1]<<8);
    d+=2;
    if(n!=65535u) {
        *gp=d;
        return (n&3u)|((n&60u)<<4)|((n&1984u)<<6)|((n&63488u)<<7);
    }
#endif
    *gp=d+3;
    return d[0]|((uint32_t)d[1]<<8)|((uint32_t)d[2]<<16);
}
#endif

/* text in colour c, each pixel's coverage blended over what is already on the canvas (an
 * anti-aliased face on a coloured box gets no dark fringe); returns the end x */
static int32_t cv_text_clip(int32_t x, int32_t y, const felucca_font_t *f, const char *s, uint16_t c, int32_t clip_left, int32_t clip_right)
{
    uint32_t r = c >> 11, g = (c >> 5) & 63u, b = c & 31u;
    uint16_t sc = swap16(c);
    f = font_live(f);
    for (; *s; s++) {
        uint32_t gi = glyph(f, (uint8_t)*s), gx, gy, w, height = f->h;
        uint32_t crop_x = 0, crop_y = 0;
#if defined(FONT_ALPHA_CROP)
        uint8_t above[32] = {0};
        uint32_t code_bits=0, code_left=0;
#elif defined(FONT_LZ_CROP)
        uint8_t history[256];
        uint32_t written = 0, left = 0, distance = 0, byte = 0;
#elif defined(FONT_PACKBITS)
        uint32_t left = 0, repeated = 0, byte = 0;
#else
        uint32_t bpr;
#endif
        const uint8_t *gd;
        gd = f->data + font_offset(f,gi);
#if defined(FONT_ALPHA_CROP) || defined(FONT_LZ_CROP)
        uint32_t rectangle = font_crop_read(&gd);
        crop_x = rectangle & 63u;
        crop_y = (rectangle >> 6) & 63u;
        w = (rectangle >> 12) & 63u;
        height = rectangle >> 18;
#else
        w = font_bitmap_width(f, gi);
#ifndef FONT_PACKBITS
        bpr = (w + 1u) / 2u;
#endif
#endif
        for (gy = 0; gy < height; gy++)
            for (gx = 0; gx < w; gx++) {
                uint32_t a;
#if defined(FONT_ALPHA_CROP)
                /* Per-glyph bitstream; exact alpha uses the already decoded previous
                 * row only. Generated codebooks cover all values in at most 7 bits. */
                if (code_left < 7u) {
                    code_bits=(code_bits<<8)|*gd++;
                    code_left+=8u;
                }
                uint32_t peek=(code_bits>>(code_left-7u))&127u;
                uint32_t entry=FONT_ALPHA_PREFIX[above[gx]][peek>>3];
                if (entry&128u) entry=FONT_ALPHA_SUB[entry&127u][peek&7u];
                code_left-=entry>>4;
                a=entry&15u;
                above[gx]=(uint8_t)a;
#elif defined(FONT_LZ_CROP)
                /* Generated streams never reference before their own glyph;
                 * the ring is bounded and needs no clear or shared state. */
                if (!(gx & 1u)) {
                    if (!left) {
                        uint32_t token = *gd++;
                        distance = token & 128u ? *gd++ : 0;
                        left = (token & 127u) + (distance ? 3u : 1u);
                    }
                    byte = distance ? history[(written - distance) & 255u] : *gd++;
                    history[written++ & 255u] = (uint8_t)byte;
                    left--;
                }
                a = byte;
#elif defined(FONT_PACKBITS)
                /* Decode one byte per pixel pair; odd-width rows discard their
                 * padding nibble exactly as the original bitmap reader did. */
                if (!(gx & 1u)) {
                    if (!left) {
                        uint32_t token = *gd++;
                        left = (token & 127u) + 1u;
                        repeated = token & 128u;
                        if (repeated)
                            byte = *gd++;
                    }
                    if (!repeated)
                        byte = *gd++;
                    left--;
                }
                a = byte;
#else
                a = gd[gy * bpr + gx / 2u];
#endif
                int32_t px = x - f->pad + (int32_t)(gx + crop_x), py = y + (int32_t)(gy + crop_y) + cv_oy;
#ifndef FONT_ALPHA_CROP
                a = (gx & 1u) ? (a & 15u) : (a >> 4);
#endif
                if (!a || px < clip_left || px >= clip_right || (uint32_t)px >= cv_w || py < cv_y0 || py >= cv_y1)
                    continue;
                if (a == 15u) {
                    cv_px[((uint32_t)py - cv_row_base) * cv_w + (uint32_t)px] = sc;
                } else {
                    uint32_t bg = swap16(cv_px[((uint32_t)py - cv_row_base) * cv_w + (uint32_t)px]);
                    uint32_t br = bg >> 11, bgg = (bg >> 5) & 63u, bb = bg & 31u;
                    uint32_t nr = cv_div15(r * a + br * (15u - a)), ng = cv_div15(g * a + bgg * (15u - a)),
                             nb = cv_div15(b * a + bb * (15u - a));
                    cv_px[((uint32_t)py - cv_row_base) * cv_w + (uint32_t)px] = swap16((nr << 11) | (ng << 5) | nb);
                }
            }
        x += font_advance(f,gi);
    }
    return x;
}

static int32_t cv_text(int32_t x, int32_t y, const felucca_font_t *f, const char *s, uint16_t c)
{ return cv_text_clip(x,y,f,s,c,0,(int32_t)cv_w); }

static int32_t text_w(const felucca_font_t *f, const char *s)
{
    int32_t w = 0;
    f = font_live(f);
    for (; *s; s++)
        w += font_advance(f,glyph(f, (uint8_t)*s));
    return w;
}

/* one-shot: text in a box, cleared to black, blitted */
static void draw_text_box(uint32_t x, uint32_t y, uint32_t w, const felucca_font_t *f, const char *s,
                          uint16_t c, int align)
{
    int32_t tw = text_w(f, s), tx = 0;
    cv_begin(w, f->h, C_BLACK);
    if (align == 1)
        tx = ((int32_t)w - tw) / 2;
    else if (align == 2)
        tx = (int32_t)w - tw;
    cv_text(tx, 0, f, s, c);
    cv_blit(x, y);
    lcd_sync();                     /* one-shots (boot, crash, UBOOT, update) finish here */
}

/* One static frame, using the existing validated still-picture record. */
#include "app/boot_screen.h"
static void draw_boot_screen(const uint8_t *pay)
{
    char title[13], subtitle[22];
    int custom = pay && pay[0] == BOOT_KIND_CUSTOM;
    int picture = custom && (pay[1] & BOOT_F_PIC);
    boot_lines(pay,title,subtitle);
#if X0X_UI_SMALL_CANVAS
    for (int band=0; band<3; ++band) {
        cv_begin(240,80,C_BLACK);
        if (picture) for (unsigned y=band?(unsigned)band*80u-20u:0u; y<BOOT_PIC_H && y+20u<(unsigned)(band+1)*80u; ++y)
            for (unsigned x=0; x<BOOT_PIC_W; ++x)
                if (boot_pix(pay+BOOT_PAY_FIXED,x,y)) cv_pset((int)x,(int)y+20-band*80,C_HI);
        cv_blit(0,(uint32_t)band*80u);
    }
#else
    for (int band=0; band<2; ++band) {
        cv_begin(240,120,C_BLACK);
        if (picture) for (unsigned y=0; y<BOOT_PIC_H; ++y)
            for (unsigned x=0; x<BOOT_PIC_W; ++x)
                if (boot_pix(pay+BOOT_PAY_FIXED,x,y)) cv_pset((int)x,(int)y+20-band*120,C_HI);
        cv_blit(0,(uint32_t)band*120u);
    }
#endif
    /* B supports lowercase; M deliberately contains capitals/numbers only. */
    draw_text_box(0,picture?137:96,240,&FONT_B,title,C_HI,1);
    draw_text_box(0,picture?164:134,240,&FONT_S,subtitle,C_GRAY,1);
    if (!custom) draw_text_box(0,154,240,&FONT_S,"BREAK",C_DIM,1);
}
