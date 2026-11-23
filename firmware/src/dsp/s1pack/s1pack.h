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
uint32_t s1pack_total(const uint8_t *pack);
uint32_t s1pack_crc(const uint8_t *pack);
/* Bank b: name, first program and count. Returns 0 when b is out of range. */
int s1pack_bank(const uint8_t *pack, unsigned bank, const char **name, unsigned *first, unsigned *count);
/* Copies program p's NUL-terminated name (out holds S1PACK_NAME_MAX bytes). */
int s1pack_name(const uint8_t *pack, unsigned program, char *out);
/* Emits program p's ordered physical writes (144 for the factory corpus).
 * Returns the number emitted, or -1 on a malformed pack. */
int s1pack_load(const uint8_t *pack, unsigned program, s1pack_emit_fn emit, void *context);

#ifdef __cplusplus
}
#endif
#endif
