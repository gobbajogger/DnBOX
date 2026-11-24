/* SPDX-License-Identifier: GPL-3.0-only */
/* XDATA region check (FL_XDATA, 0xAD000..0xAEFFF): the FONT_S/M/XS/B bitmaps sit outside the app image,
 * installed by the update loader with a 16 B header (tools/build.py xdata_image):
 *   +0 magic XDATA_MAGIC  +4 blob length  +8 zlib CRC-32 of the blob  +12 build id (FONT_XDATA_CRC32)
 * The app trusts the region only when every field matches the font set it was built with; otherwise
 * gfx.c draws those faces with FONT_T and the UI asks for a reinstall. Pure: host tests call it. */
#ifndef X0X_XDATA_H
#define X0X_XDATA_H
#include <stdint.h>
#include "../crc32_nibble.h"

#define XDATA_MAGIC 0x31445858u                 /* "XXD1" */
#define XDATA_HDR   16u
#ifdef FL_XDATA_MAGIC
_Static_assert(FL_XDATA_MAGIC == XDATA_MAGIC && FL_XDATA_HDR == XDATA_HDR, "XDATA header: flash map and xdata.h agree");
#endif

static uint32_t xdata_rd32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

/* 1: the region at p (room bytes) holds exactly the blob of want_len bytes with build id want_id */
static uint8_t xdata_valid(const uint8_t *p, uint32_t room, uint32_t want_len, uint32_t want_id)
{
    uint32_t len;
    if (room < XDATA_HDR || xdata_rd32(p) != XDATA_MAGIC)
        return 0;
    len = xdata_rd32(p + 4);
    if (len != want_len || len > room - XDATA_HDR || xdata_rd32(p + 12) != want_id)
        return 0;
    return x0x_crc32(p + XDATA_HDR, len) == xdata_rd32(p + 8) && xdata_rd32(p + 8) == want_id;
}
#endif
