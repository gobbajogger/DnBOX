/* SPDX-License-Identifier: GPL-3.0-only */
/* FITTAB (fm1_flash_map.h FL_FITTAB, 0xFE000..0xFEFFF: the Synth One option-B fit table,
 * installed by the update loader) in the production (non-vendor-test) SMK map:
 *   the sector sits exactly between USR (ends 0xFE000) and key_mac (starts 0xFF000);
 *   the app store cannot write it; the loader may write exactly that one 4 KiB sector
 *   and refuses any write or erase that reaches 0xFF000 (off-by-one and two-sector erase);
 *   app/fittab.h returns the blob only for the exact header + CRC-32 + build id, else NULL
 *   (absent). The owner check never touches the sector. */
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "fm1_flash_map.h"
#include "../../firmware/src/app/plat.h"
static uint8_t flash[0x100000], before[0x100000];
static unsigned fittab_erases;
static int st_read(uint32_t off, void *dst, uint32_t n)
{ memcpy(dst, flash + off, n); return 0; }
static int st_erase(uint32_t off)
{
    assert(FL_STORE_OK(off, 4096));
    if (off >= FL_FITTAB_LO && off < FL_FITTAB_HI) fittab_erases++;
    memset(flash + off, 255, 4096); return 0;
}
static int st_prog(uint32_t off, const void *src, uint32_t n)
{ assert(FL_STORE_OK(off, n)); memcpy(flash + off, src, n); return 0; }
#include "../../firmware/src/storage.c"
#include "../../firmware/src/app/fittab.h"

static void put32(uint8_t *p, uint32_t v)
{ p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); p[2] = (uint8_t)(v >> 16); p[3] = (uint8_t)(v >> 24); }

int main(void)
{
    uint32_t s, k, crc;
    static uint8_t region[0x1000], bad[0x1000];
    const uint8_t *blob;

    assert(FL_FITTAB_LO == 0xFE000u && FL_FITTAB_HI == 0xFF000u);
    assert(FL_FITTAB_LO == FL_USRRES_HI && FL_FITTAB_HI == FL_KEYMAC_LO);
    assert(FL_KEYMAC_LO == 0xFF000u && FL_FITTAB_HI - FL_FITTAB_LO == 0x1000u);
    assert(FL_WIN_SAFE(FL_FITTAB_LO, FL_FITTAB_HI));
    assert(FITTAB_BYTES == 1388u && FITTAB_HDR == 16u && FITTAB_MAGIC == FL_FITTAB_MAGIC);

    /* app cannot write the sector; the loader may write exactly that one 4 KiB sector */
    assert(!FL_STORE_OK(FL_FITTAB_LO, 0x1000u) && !FL_RANGE_OK(FL_FITTAB_LO, 0x1000u));
    assert(!FL_GUARD(FL_FITTAB_LO, 0x1000u));
    assert(FL_LDR_RANGE_OK(FL_FITTAB_LO, 0x1000u));
    assert(FL_LDR_RANGE_OK(FL_FITTAB_LO, 1) && FL_LDR_RANGE_OK(0xFEFFFu, 1));
    assert(FL_LDR_RANGE_OK(FL_FITTAB_LO, 0xFFFu));
    assert(!FL_PROTECTED(FL_FITTAB_LO, 0x1000u));

    /* off-by-one at the key_mac boundary, and erase granularity (two sectors / unaligned) */
    assert(!FL_LDR_RANGE_OK(0xFEFFFu, 2));               /* last FITTAB byte + first key_mac byte */
    assert(!FL_LDR_RANGE_OK(0xFF000u, 1));
    assert(!FL_LDR_RANGE_OK(0xFF000u, 0x1000u));
    assert(!FL_LDR_RANGE_OK(FL_FITTAB_LO, 0x1001u));     /* one byte into key_mac */
    assert(!FL_LDR_RANGE_OK(FL_FITTAB_LO, 0x2000u));     /* 8 KiB erase would include key_mac */
    assert(!FL_LDR_RANGE_OK(0xFD000u, 0x2000u));         /* last USR sector + FITTAB */
    assert(!FL_LDR_RANGE_OK(0xFE001u, 0x1000u));         /* unaligned 4 KiB would cross 0xFF000 */
    assert(FL_PROTECTED(0xFEFFFu, 2) && FL_PROTECTED(0xFF000u, 1));
    assert(FL_PROTECTED(FL_FITTAB_LO, 0x2000u));
    assert(FL_PROTECTED(FL_USRRES_LO, 0xA000u));
    assert(!FL_LDR_RANGE_OK(FL_USRRES_LO, 0x1000u));

    /* the store and the owner check never touch FITTAB, USR or key_mac */
    for (unsigned erased = 0; erased < 2; erased++) {
        memset(flash, 0x5A, sizeof flash);
        if (erased) memset(flash + 0xAF000u, 255, 4096);
        memcpy(before, flash, sizeof flash); fittab_erases = 0;
        st_owner_check();
        assert(!fittab_erases);
        for (s = FL_FITTAB_LO; s < 0x100000u; s++) assert(flash[s] == before[s]);
        for (s = FL_USRRES_LO; s < FL_USRRES_HI; s++) assert(flash[s] == before[s]);
    }

    /* header check: present vs absent */
    memset(region, 0xFF, sizeof region);
    for (k = 0; k < FITTAB_BYTES; k++) region[16 + k] = (uint8_t)(k * 37u + 11u);
    crc = x0x_crc32(region + 16, FITTAB_BYTES);
    put32(region, FITTAB_MAGIC); put32(region + 4, FITTAB_BYTES);
    put32(region + 8, crc); put32(region + 12, crc);
    blob = fittab_blob(region, sizeof region, FITTAB_BYTES, crc);
    assert(blob == region + FITTAB_HDR && blob[0] == (uint8_t)(11u));

    memcpy(bad, region, sizeof bad); bad[16 + 400] ^= 1;
    assert(fittab_blob(bad, sizeof bad, FITTAB_BYTES, crc) == 0);          /* flipped bit in the blob */
    assert(fittab_blob(region, sizeof region, FITTAB_BYTES, crc ^ 1u) == 0); /* another build */
    assert(fittab_blob(region, sizeof region, FITTAB_BYTES - 1u, crc) == 0); /* another length */
    memcpy(bad, region, sizeof bad); bad[0] ^= 1;
    assert(fittab_blob(bad, sizeof bad, FITTAB_BYTES, crc) == 0);           /* magic */
    memcpy(bad, region, sizeof bad); put32(bad + 12, crc ^ 2u);
    assert(fittab_blob(bad, sizeof bad, FITTAB_BYTES, crc) == 0);           /* header build id */
    memcpy(bad, region, sizeof bad); put32(bad + 4, 0x2000u);
    assert(fittab_blob(bad, sizeof bad, 0x2000u, crc) == 0);                /* longer than the region */
    memset(bad, 0xFF, sizeof bad);
    assert(fittab_blob(bad, sizeof bad, FITTAB_BYTES, crc) == 0);           /* erased (never installed) */
    assert(fittab_blob(region, 8, FITTAB_BYTES, crc) == 0);
    assert(fittab_blob(0, sizeof region, FITTAB_BYTES, crc) == 0);

    puts("FITTAB region: 0xFE000-0xFEFFF, loader one sector only, key_mac 0xFF000 refused, "
         "header check returns absent PASS");
    return 0;
}
