/* SPDX-License-Identifier: GPL-3.0-only */
/* Synth One factory program pack reader (tools/pack_synthone_factory.py).
 * The pack is position independent: pass its base address (an XIP window,
 * e.g. fm1_xip_ptr(FL_S1P_LO), or a RAM image on the host). The reader uses
 * no heap and no static RAM; its largest stack frame holds 15 code counts. */
#ifndef S1PACK_H
#define S1PACK_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

#define S1PACK_HEADER 32u
#define S1PACK_NAME_MAX 48u           /* name bytes plus NUL */

typedef void (*s1pack_emit_fn)(void *context, unsigned parameter, float physical);

/* Header checks only (magic, version, size, offsets). Returns the program
 * count, or 0. The caller checks the CRC-32 of bytes 32..total with x0x_crc32. */
unsigned s1pack_programs(const uint8_t *pack, uint32_t available);
static inline uint32_t s1pack_u32(const uint8_t *p) { return p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24; }
static inline uint32_t s1pack_total(const uint8_t *pack) { return s1pack_u32(pack + 12); }
static inline uint32_t s1pack_crc(const uint8_t *pack) { return s1pack_u32(pack + 16); }
/* Bank b: name, first program and count. Returns 0 when b is out of range. */
int s1pack_bank(const uint8_t *pack, unsigned bank, const char **name, unsigned *first, unsigned *count);
/* Walks to program p: copies its NUL-terminated name into out (S1PACK_NAME_MAX
 * bytes) and, when emit is set, emits its ordered physical writes (144 for the
 * factory corpus). Returns the number emitted (0 for a name lookup), or -1 for
 * an out-of-range program or a malformed pack. */
int s1pack_read(const uint8_t *pack, unsigned program, char *out, s1pack_emit_fn emit, void *context);
static inline int s1pack_name(const uint8_t *pack, unsigned program, char *out)
{
    return s1pack_read(pack, program, out, 0, 0) == 0;
}
static inline int s1pack_load(const uint8_t *pack, unsigned program, s1pack_emit_fn emit, void *context)
{
    char name[S1PACK_NAME_MAX];
    return emit ? s1pack_read(pack, program, name, emit, context) : -1;
}

#ifdef __cplusplus
}
#endif
#endif
