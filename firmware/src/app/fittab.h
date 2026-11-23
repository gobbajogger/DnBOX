/* SPDX-License-Identifier: GPL-3.0-only */
/* FITTAB region check (FL_FITTAB, 0xFE000..0xFEFFF): the Synth One option-B per-program
 * fit table sits outside the app image, installed by the update loader with a 16 B header
 * (tools/build.py fittab_image), the same layout as XDATA:
 *   +0 magic FITTAB_MAGIC  +4 blob length  +8 zlib CRC-32 of the blob  +12 build id
 * The blob is 1,388 B (694 little-endian uint16 words, one per Synth One program). Synths
 * owns the real content (replace firmware/src/app/fittab_blob.bin; the build writes the
 * matching FITTAB_LEN / FITTAB_CRC32) and the engine fallback. This file is pure: host
 * tests call it. The stock vendor loader and mask-ROM rescue never write the sector, so a
 * missing or foreign header is the normal absent case. */
#ifndef X0X_FITTAB_H
#define X0X_FITTAB_H
#include <stdint.h>
#include "../crc32_nibble.h"

#define FITTAB_MAGIC    0x31425446u             /* "FTB1" */
#define FITTAB_HDR      16u
#define FITTAB_PROGRAMS 694u
#define FITTAB_BYTES    (FITTAB_PROGRAMS * 2u)  /* 1,388 */
#ifdef FL_FITTAB_MAGIC
_Static_assert(FL_FITTAB_MAGIC == FITTAB_MAGIC && FL_FITTAB_HDR == FITTAB_HDR,
               "FITTAB header: flash map and fittab.h agree");
#endif
#ifdef FITTAB_LEN
_Static_assert(FITTAB_LEN == FITTAB_BYTES, "FITTAB_LEN is 694 programs x 2 B");
#endif

static uint32_t fittab_rd32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

/* Pointer to the want_len-byte blob at p + FITTAB_HDR when the header is valid and matches
 * want_len / want_id; otherwise NULL (absent: erased, damaged, or a different build). The
 * caller falls back. Read only at program load, never on the audio path. */
static const uint8_t *fittab_blob(const uint8_t *p, uint32_t room, uint32_t want_len, uint32_t want_id)
{
    uint32_t len;
    if (p == 0 || room < FITTAB_HDR || fittab_rd32(p) != FITTAB_MAGIC)
        return 0;
    len = fittab_rd32(p + 4);
    if (len != want_len || len > room - FITTAB_HDR || fittab_rd32(p + 12) != want_id)
        return 0;
    if (x0x_crc32(p + FITTAB_HDR, len) != fittab_rd32(p + 8) || fittab_rd32(p + 8) != want_id)
        return 0;
    return p + FITTAB_HDR;
}

#if defined(FITTAB_LEN) && defined(FL_XIP) && BOARD_SMK37
/* Live region after fl_plain_window_init: the installed table, or NULL (absent). */
static const uint8_t *fittab_load(void)
{
    return fittab_blob((const uint8_t *)FL_XIP(FL_FITTAB_LO), FL_FITTAB_HI - FL_FITTAB_LO,
                       FITTAB_LEN, FITTAB_CRC32);
}
#endif
#endif
