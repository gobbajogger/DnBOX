/* SPDX-License-Identifier: GPL-3.0-only */
/* DBD3 shared preset/BREAK pool, foreground maintenance only. */
#ifndef DNPOOL_H
#define DNPOOL_H
#include "dnbank.h"
#define DNP_SECTOR 4096u
#define DNP_DATA 28u
#define DNP_TOTAL 30u
#define DNP_BYTES (DNP_TOTAL*DNP_SECTOR)
#define DNP_BASE 0xD6000u
#define DNP_CHUNK 256u
/* Callback offsets are RELATIVE to DNP_BASE. Return 0 on success. Program
 * from RAM, complete flash operation, invalidate XIP/cache before returning.
 * Caller serializes ALL access, including sample/project writers and engine
 * preset reads. Guard must be true only inside an exclusive stopped session. */
typedef struct {
    const uint8_t *flash;
    void *ctx;
    int (*erase)(void *,uint32_t);
    int (*write)(void *,uint32_t,const uint8_t *,unsigned);
    int (*held)(void *);
    /* Must check real live + saved project refs. NULL denies destructive changes.
     * new_bank NULL means remove. Neither handle may outlive the call. */
    int (*allow_replace)(void *,const dnbank *,const dnbank *);
    /* Verify current committed slot-2 FSMP extent AND recovery shadow state.
     * Return exact occupied bytes, or UINT32_MAX if uncertain. */
    uint32_t (*sample_extent)(void *);
} dnp_io;
typedef struct {
    dnp_io io;
    uint32_t gen,crc,size,written;
    uint8_t id[8],map[16],count,active;
} dnpool;
typedef struct { uint32_t generation; uint8_t reserved,banks,bank_sectors,free_sectors,contiguous; } dnp_info;
enum {DNP_OK=0,DNP_INVALID=-1,DNP_BUSY=-2,DNP_FULL=-3,DNP_CORRUPT=-4,DNP_IO=-5,DNP_PROTECTED=-6,DNP_STALE=-7};
/* Bind caller-provided session (zero transient state). No flash mutation. */
void dnp_init(dnpool *,const dnp_io *);
/* Explicit format ONLY if entire pool blank. Never format existing legacy data. */
int dnp_format(dnpool *);
int dnp_get_info(dnpool *,dnp_info *);
int dnp_verify(dnpool *);
/* Open exact ID; pointer map refers to active directory, valid only until mutation. */
int dnp_open(dnpool *,const uint8_t id[8],dnbank *);
int dnp_bank_at(dnpool *,unsigned,uint8_t id[8],uint32_t *size,uint32_t *crc);
int dnp_bank_read(dnpool *,const uint8_t id[8],uint32_t generation,uint32_t offset,uint8_t *out,unsigned n);
/* BEGIN allocates unused sectors, does not erase. WRITE sequential <=256 byte
 * chunks (offset aligned except final tail). END validates then commits. */
int dnp_begin(dnpool *,const uint8_t id[8],uint32_t size,uint32_t crc,uint32_t generation);
int dnp_write(dnpool *,uint32_t offset,const uint8_t *,unsigned);
int dnp_end(dnpool *);
void dnp_abort(dnpool *);
int dnp_remove(dnpool *,const uint8_t id[8],uint32_t generation);
/* Reserve BEFORE sample writes; caller must enforce reserved extent, not the
 * theoretical max. Grow relocates banks atomically one sector at a time.
 * Release derives current sample extent from trusted callback, not host data. */
int dnp_reserve_sample(dnpool *,unsigned sectors,uint32_t generation);
int dnp_release_sample(dnpool *,uint32_t generation);
#endif
