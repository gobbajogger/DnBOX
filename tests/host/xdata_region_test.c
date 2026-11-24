/* SPDX-License-Identifier: GPL-3.0-only */
/* XDATA (fm1_flash_map.h FL_XDATA, 0xAD000..0xAEFFF: the FONT_S/M/XS/B bitmaps, installed by the update
 * loader) in the production (non-vendor-test) SMK map:
 *   1. st_owner_check's foreign-store clear stops at 0xAD000: XDATA survives a foreign or untagged store;
 *   2. FL_STORE_OK leaves XDATA out (no app store write can reach it); the loader window still covers it;
 *   5. app/xdata.h accepts only the exact header + blob the app was built with (magic, length, CRC-32,
 *      build id), so gfx.c falls back to FONT_T on anything else. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "fm1_flash_map.h"
#include "../../firmware/src/app/plat.h"
static uint8_t flash[0x100000], before[0x100000];
static unsigned xdata_erases;
static int st_read(uint32_t off, void *dst, uint32_t n)
{ memcpy(dst, flash + off, n); return 0; }
static int st_erase(uint32_t off)
{
    assert(FL_STORE_OK(off, 4096));
    if (off >= FL_XDATA_LO && off < FL_XDATA_HI) xdata_erases++;
    memset(flash + off, 255, 4096); return 0;
}
static int st_prog(uint32_t off, const void *src, uint32_t n)
{ assert(FL_STORE_OK(off, n)); memcpy(flash + off, src, n); return 0; }
#include "../../firmware/src/storage.c"
#include "../../firmware/src/app/xdata.h"

static void put32(uint8_t *p, uint32_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24); }

int main(void)
{
    uint32_t s, k, crc;
    static uint8_t region[0x2000], bad[0x2000];
    /* 2. store window: everything of FL_DATA except XDATA */
    assert(FL_STORE_OK(0x9D000u, 0x10000u));                 /* objects + boot screens 0x9D000..0xACFFF */
    assert(FL_STORE_OK(0xAC000u, 4096) && FL_STORE_OK(0xACFFFu, 1));
    assert(FL_STORE_OK(0xAF000u, 4096));                      /* owner tag */
    for (s = FL_XDATA_LO; s < FL_XDATA_HI; s += 256u) {
        assert(!FL_STORE_OK(s, 256)); assert(!FL_STORE_OK(s, 1)); assert(!FL_RANGE_OK(s, 256));
    }
    assert(!FL_STORE_OK(0xACFFFu, 2)); assert(!FL_STORE_OK(0xAEFFFu, 2)); assert(!FL_STORE_OK(0x9D000u, 0x13000u));
    assert(!FL_GUARD(FL_XDATA_LO, 0x2000u));                  /* not a protected range ... */
    assert(FL_LDR_RANGE_OK(FL_XDATA_LO, 0x2000u));            /* ... and the loader may write it */
    /* 1. a foreign (0x5A) or untagged (erased tag) store: cleared up to 0xAD000, XDATA and all else kept */
    for (unsigned erased = 0; erased < 2; erased++) {
        memset(flash, 0x5A, sizeof flash);
        if (erased) memset(flash + 0xAF000u, 255, 4096);
        memcpy(before, flash, sizeof flash); xdata_erases = 0; st_owner_cleared = 0;
        st_owner_check();
        assert(!xdata_erases && st_owner_cleared);
        for (s = 0x9D000u; s < 0xAD000u; s++) assert(flash[s] == 0xFF);   /* store + boot screens cleared */
        for (s = 0; s < sizeof flash; s++)
            if (!((s >= 0x9D000u && s < 0xAD000u) || (s >= 0xAF000u && s < 0xB0000u))) assert(flash[s] == before[s]);
        assert(st_save(OBJ_SET, "x", 1) == 0);
        { char c; assert(st_load(OBJ_SET, &c, 1) == 1 && c == 'x'); }
    }
    /* 5. the header check */
    memset(region, 0xFF, sizeof region);
    for (k = 0; k < 7682u; k++) region[16 + k] = (uint8_t)(k * 37u + 11u);
    crc = x0x_crc32(region + 16, 7682u);
    put32(region, XDATA_MAGIC); put32(region + 4, 7682u); put32(region + 8, crc); put32(region + 12, crc);
    assert(xdata_valid(region, sizeof region, 7682u, crc));
    memcpy(bad, region, sizeof bad); bad[16 + 4000] ^= 1;      /* a flipped bit in the blob */
    assert(!xdata_valid(bad, sizeof bad, 7682u, crc));
    assert(!xdata_valid(region, sizeof region, 7682u, crc ^ 1u));   /* another build's font set */
    assert(!xdata_valid(region, sizeof region, 7681u, crc));        /* another length */
    memcpy(bad, region, sizeof bad); bad[0] ^= 1;
    assert(!xdata_valid(bad, sizeof bad, 7682u, crc));               /* magic */
    memcpy(bad, region, sizeof bad); put32(bad + 12, crc ^ 2u);
    assert(!xdata_valid(bad, sizeof bad, 7682u, crc));               /* header build id */
    memcpy(bad, region, sizeof bad); put32(bad + 4, 0x10000u);
    assert(!xdata_valid(bad, sizeof bad, 0x10000u, crc));            /* longer than the region */
    memset(bad, 0xFF, sizeof bad);
    assert(!xdata_valid(bad, sizeof bad, 7682u, crc));               /* erased (never installed) */
    assert(!xdata_valid(region, 8, 7682u, crc));
    puts("XDATA region: owner clear stops at 0xAD000, FL_STORE_OK excludes XDATA, header check exact PASS");
    return 0;
}
