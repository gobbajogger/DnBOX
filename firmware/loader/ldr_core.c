/* SPDX-License-Identifier: GPL-3.0-only
 * Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments */
/* Felucca update loader, the portable part (step 2 of the M-UPGRADE update
 * protocol).
 *
 * The host serves the package ("logical image") with cmd 0x30 reads (ota.c).
 * The loader:
 *   1. reads the UFW header and finds flash.bin;
 *   2. checks that the package's app area decrypts with THIS chip's key
 *      (taken from isd_config.ini in the device's own flash head), so a
 *      package for another key is refused before anything is erased;
 *   3. writes ONLY the app area [0x4000, 0x93000), sector by sector, skipping
 *      sectors that are already equal, and verifies each one; the flash head
 *      (SPL, isd_config) is never written; SMK: then the XDATA region
 *      0xAD000..0xAEFFF from the package's xdata.bin entry, if it has one,
 *      then the FITTAB sector 0xFE000..0xFEFFF from fittab.bin, then FDATA (the Synth One
 *      factory pack, the end of sample slot 2) from fdata.bin, each if it has one;
 *   4. asks 0xF0000000 ("success"), then invalidates the update record (RAM
 *      and flash) so the SPL boots the new app, and resets.
 * Power loss during 3: the record is still there, the SPL runs the loader
 * again on the next power-on and the host can resume.
 *
 * Hooks from the platform (loader.c, tests/ldr_test.c):
 *   ldr_fread(off, p, n)  ldr_erase(off)  ldr_prog(off, p, n)   flash, 0 = ok
 *   ldr_record_clear()    forget the RAM update record
 *   ldr_progress(done, total)
 * plus the ota.c hooks (frames, time, idle). */
#define LDR_APP_LO 0x4000u
#if BOARD_SMK37
#include "../hal/fm1_flash_map.h"
/* SMK-37 Pro: app area up to the stock PRCT end; records only in the VM, so ldr_records_drop can never
 * erase BTIF / USRTRIM (0xC0000..0xC1FFF, the key calibration) or anything above */
#define LDR_APP_HI 0x9D000u
#define LDR_REC_LO 0x9D000u
#define LDR_REC_HI 0xC0000u
_Static_assert(LDR_APP_HI <= LDR_REC_LO && LDR_REC_HI <= 0xC0000u && LDR_APP_LO >= 0x4000u,
               "SMK loader windows must stay below BTIF / USRTRIM and above the head");
#else
#define LDR_APP_HI 0x93000u
#define LDR_REC_LO 0x93000u                     /* update records live above the app ... */
#define LDR_REC_HI 0xFC000u                     /* ... and below Felucca's globals */
#endif

static int ldr_fread(uint32_t off, void *p, uint32_t n);
static int ldr_erase(uint32_t off);
static int ldr_prog(uint32_t off, const void *p, uint32_t n);
static void ldr_record_clear(void);
static void ldr_progress(uint32_t done, uint32_t total);

/* SFC cipher: per 32-byte block, key ^ (block offset >> 2) */
static void ldr_sfc(uint8_t *p, uint32_t n, uint32_t base_off, uint32_t key)
{
    uint32_t i, j;
    for (i = 0; i < n; i += 32u) {
        uint32_t k = key ^ ((base_off + i) >> 2);
        for (j = 0; j < 32u && i + j < n; j++) {
            p[i + j] ^= (uint8_t)k;
            k = ((k << 1) ^ (k & 0x8000u ? 0x1021u : 0u)) & 0xFFFFu;
        }
    }
}

/* chip key from the device's flash head: JLFS top entries (ENC 0xFFFF) -> isd_config.ini blob */
static int ldr_chip_key(uint32_t *key)
{
    uint8_t e[32], blob[34];
    uint32_t off, i, sum, k;
    for (off = 32; off < 0x400u; off += 32u) {
        if (ldr_fread(off, e, 32))
            return -1;
        ota_jl_enc(e, 32);
        if (ota_crc16(e + 2, 30, 0) != ota_rd16(e))
            return -2;
        if (!ota_memeq(e + 16, (const uint8_t *)"isd_config.ini", 15)) {
            if (ota_rd16(e + 14))
                return -3;                               /* last entry, not found */
            continue;
        }
        if (ldr_fread(ota_rd32(e + 4), blob, 34) || ota_crc16(blob, 32, 0) != ota_rd16(blob + 32))
            return -4;
        for (i = 0, sum = 0; i < 16u; i++)
            sum += blob[i];
        sum &= 0xFFu;
        sum = sum >= 0xE0u ? 0xAAu : sum <= 0x10u ? 0x55u : sum;
        for (i = 0, k = 0; i < 16u; i++)
            if ((uint32_t)(blob[16 + i] ^ blob[15 - i]) < sum)
                k |= 1u << i;
        *key = k;
        return 0;
    }
    return -3;
}

/* one 4 KiB sector: erase + program + read back unless it already holds sec (two tries) */
static int ldr_put(uint32_t s, const uint8_t *sec, uint8_t *cur)
{
    uint32_t tries, k;
    if (ldr_fread(s, cur, 0x1000u))
        return -8;
    for (tries = 0; tries < 2u && !ota_memeq(sec, cur, 0x1000u); tries++) {
        if (ldr_erase(s))
            return -9;
        for (k = 0; k < 0x1000u; k += 256u)
            if (ldr_prog(s + k, sec + k, 256))
                return -10;
        if (ldr_fread(s, cur, 0x1000u))
            return -8;
    }
    return ota_memeq(sec, cur, 0x1000u) ? 0 : -11;      /* -11: verify failed twice */
}

#if BOARD_SMK37 && !(defined(X0X_SMC_VENDOR_TEST) && X0X_SMC_VENDOR_TEST)
/* XDATA (fm1_flash_map.h FL_XDATA, plain, outside the app area): the package's "xdata.bin" UFW entry (type 101,
 * exactly the 8 KiB region: tools/build.py xdata_image). write 0: only check the entry (its CRC16 over the whole
 * entry), before the app area's first erase; write 1: then put it in place. No entry (an older package): left
 * as it is (the app then draws FONT_T and says so). */
#define LDR_XD_LO   0xAD000u
#define LDR_XD_HI   0xAF000u
#define LDR_XD_TYPE 101u
_Static_assert(LDR_XD_LO >= LDR_REC_LO && LDR_XD_HI <= LDR_REC_HI, "XDATA inside the loader's VM window");
static int ldr_xdata(const uint8_t *hdr, uint8_t *sec, uint8_t *cur, uint32_t write)
{
    uint32_t i, k, s, off = 0, len = 0, want = 0, crc = 0, nent = ota_rd16(hdr + 8);
    int rc;
    for (i = 0; i < nent; i++) {
        const uint8_t *e = hdr + 0x40 + i * 0x50u;
        if (ota_rd16(e) == LDR_XD_TYPE) {
            want = ota_rd16(e + 4);
            off = ota_rd32(e + 8);
            len = ota_rd32(e + 12);
        }
    }
    if (!off)
        return 0;
    if (len != LDR_XD_HI - LDR_XD_LO)
        return -12;
    for (k = 0; k < len; k += 512u) {
        if (ota_read(off + k, sec, 512))
            return -7;
        crc = ota_crc16(sec, 512, crc);
    }
    if (crc != want)
        return -13;
    for (s = LDR_XD_LO; write && s < LDR_XD_HI; s += 0x1000u) {
        for (k = 0; k < 0x1000u; k += 512u)
            if (ota_read(off + (s - LDR_XD_LO) + k, sec + k, 512))
                return -7;
        if ((rc = ldr_put(s, sec, cur)) != 0)
            return rc;
    }
    return 0;
}
#else
#define ldr_xdata(hdr, sec, cur, write) 0
#endif

#if BOARD_SMK37 && !(defined(X0X_SMC_VENDOR_TEST) && X0X_SMC_VENDOR_TEST)
/* FITTAB (fm1_flash_map.h FL_FITTAB, plain, the 4 KiB spare between USR and key_mac): the package's
 * "fittab.bin" UFW entry (type 102, exactly the 4 KiB sector: tools/build.py fittab_image). write 0:
 * only check the entry (its CRC16 over the whole entry), before the app area's first erase; write 1:
 * then put it in place. No entry (an older package, or stock vendor / mask-ROM rescue): left as it is
 * (the app's fittab_blob / fittab_load then return NULL, absent). Any write or erase that would reach
 * 0xFF000 is refused by FL_LDR_RANGE_OK / FL_PROTECTED. */
#define LDR_FT_LO   0xFE000u
#define LDR_FT_HI   0xFF000u
#define LDR_FT_TYPE 102u
_Static_assert(LDR_FT_LO == 0xFE000u && LDR_FT_HI == 0xFF000u && LDR_FT_HI - LDR_FT_LO == 0x1000u,
               "FITTAB is exactly one sector, immediately below key_mac");
/* One loader-written region [lo, hi) from the package's UFW entry of type typ (exactly hi - lo bytes):
 * write 0 checks the entry's CRC16 only (before the app area's first erase), write 1 puts it in place,
 * sector by sector (ldr_put skips equal sectors and verifies). No entry: 0, the region is left as it is.
 * A wrong length returns elen, a wrong CRC elen - 1. */
static int ldr_region(const uint8_t *hdr, uint8_t *sec, uint8_t *cur, uint32_t write,
                      uint32_t typ, uint32_t lo, uint32_t hi, int elen)
{
    uint32_t i, k, s, off = 0, len = 0, want = 0, crc = 0, nent = ota_rd16(hdr + 8);
    int rc;
    for (i = 0; i < nent; i++) {
        const uint8_t *e = hdr + 0x40 + i * 0x50u;
        if (ota_rd16(e) == typ) {
            want = ota_rd16(e + 4);
            off = ota_rd32(e + 8);
            len = ota_rd32(e + 12);
        }
    }
    if (!off)
        return 0;
    if (len != hi - lo)
        return elen;
    for (k = 0; k < len; k += 512u) {
        if (ota_read(off + k, sec, 512))
            return -7;
        crc = ota_crc16(sec, 512, crc);
    }
    if (crc != want)
        return elen - 1;
    for (s = lo; write && s < hi; s += 0x1000u) {
        for (k = 0; k < 0x1000u; k += 512u)
            if (ota_read(off + (s - lo) + k, sec + k, 512))
                return -7;
        if ((rc = ldr_put(s, sec, cur)) != 0)
            return rc;
    }
    return 0;
}
#define ldr_fittab(hdr, sec, cur, write) ldr_region(hdr, sec, cur, write, LDR_FT_TYPE, LDR_FT_LO, LDR_FT_HI, -14)
/* FDATA (fm1_flash_map.h FL_FDATA, plain, the end of sample slot 2): the Synth One factory pack from the
 * package's "fdata.bin" entry (type 103, exactly FL_FDATA_SIZE bytes: tools/build.py fdata_image). Same
 * rules as FITTAB: a wrong length -16, a wrong CRC -17, no entry leaves the region (the app then finds no
 * pack and Synth One plays INIT). Slot 2 ends at FL_FDATA_LO (app/slots.c); the installers back up and
 * restore a slot-2 sample that the shorter slot no longer holds before this runs. */
#define LDR_FD_TYPE 103u
#if FL_FDATA_SIZE
_Static_assert(FL_FDATA_LO % 0x1000u == 0u && FL_FDATA_HI == 0xF4000u && FL_FDATA_LO >= 0xC2000u,
               "FDATA is whole sectors at the end of the sample slots, below the stock USR resources");
#define ldr_fdata(hdr, sec, cur, write) ldr_region(hdr, sec, cur, write, LDR_FD_TYPE, FL_FDATA_LO, FL_FDATA_HI, -16)
#else
#define ldr_fdata(hdr, sec, cur, write) 0
#endif
#else
#define ldr_fittab(hdr, sec, cur, write) 0
#define ldr_fdata(hdr, sec, cur, write) 0
#endif

static int ldr_session(void)
{
    static uint8_t hdr[0x400], sec[0x1000], cur[0x1000];
    uint32_t i, k, fl_off, fl_size, ota_off, ota_len, key, s, done = 0;
    int rc;
    /* 1. UFW header + entry list (ota.c) */
    if ((rc = ota_ufw(hdr, &fl_off, &fl_size, &ota_off, &ota_len)) != 0)
        return rc;
    if (!fl_off || fl_size < LDR_APP_HI)
        return -3;
    /* 2. the package's app area must decrypt with this chip's key */
    if ((rc = ldr_chip_key(&key)) != 0)
        return -40 + rc;
    if (ota_read(fl_off + LDR_APP_LO, sec, 32))
        return -5;
    ldr_sfc(sec, 32, 0, key);
    if (ota_crc16(sec + 2, 30, 0) != ota_rd16(sec))
        return -6;                                       /* package for another chip key */
    /* the XDATA, FITTAB and FDATA entries, if any, must be whole before anything is erased */
    if ((rc = ldr_xdata(hdr, sec, cur, 0)) != 0)
        return rc;
    if ((rc = ldr_fittab(hdr, sec, cur, 0)) != 0)
        return rc;
    if ((rc = ldr_fdata(hdr, sec, cur, 0)) != 0)
        return rc;
    /* 3. app area, sector by sector */
    for (s = LDR_APP_LO; s < LDR_APP_HI; s += 0x1000u) {
        for (k = 0; k < 0x1000u; k += 512u)
            if (ota_read(fl_off + s + k, sec + k, 512))
                return -7;
        if ((rc = ldr_put(s, sec, cur)) != 0)
            return rc;
        done += 0x1000u;
        ldr_progress(done, LDR_APP_HI - LDR_APP_LO);
    }
    /* 3b. XDATA, FITTAB then FDATA after the app: power loss here resumes from the record like the app area */
    if ((rc = ldr_xdata(hdr, sec, cur, 1)) != 0)
        return rc;
    if ((rc = ldr_fittab(hdr, sec, cur, 1)) != 0)
        return rc;
    if ((rc = ldr_fdata(hdr, sec, cur, 1)) != 0)
        return rc;
    /* 4. finish: the host confirms, the record goes, the SPL boots the app */
    for (i = 0; i < 4u; i++)
        if (!ota_read(0xF0000000u, sec, 8))
            break;
    ldr_record_clear();
    for (s = LDR_REC_HI; s > LDR_REC_LO; s -= 0x1000u) {   /* flash records: 4K boundary - 256 */
        uint8_t r[80];
        if (ldr_fread(s - 0x100u, r, 80))
            break;
        if (ota_rd16(r + 6) == 0x5441u && ota_rd16(r) && ota_rd16(r) == ota_crc16(r + 2, 78, 0))
            ldr_erase(s - 0x1000u);
    }
    return 0;
}
