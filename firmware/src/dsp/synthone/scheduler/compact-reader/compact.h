// SPDX-License-Identifier: GPL-3.0-only
#ifndef S1_COMPACT_WORDS_H
#define S1_COMPACT_WORDS_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
/* Borrowed immutable compact-v1 bytes; no ownership, allocation or cache.
 * Caller supplies live mapped/readable bytes for byte_count and an aligned,
 * writable binding. Payload is byte-aligned and may have a nonzero view offset.
 * Binding and payload must be disjoint. Do not forge/mutate a successful
 * binding or its payload, and do not race reads with changes/free/reuse.
 * Neither uintptr span checks nor the tag prove OS mapping or authenticity. */
typedef struct s1c_binding { const unsigned char *bytes; size_t byte_count; uint32_t format_tag; } s1c_binding;
/* Full cold structural/topology/finite-word admission, at most 4MiB.
 * Failure preserves the whole binding. Source/content identity still requires
 * the independent exact input/container digest: finite wrong words can admit. */
int s1c_bind(s1c_binding *,const void *,size_t);
/* Shallow tag/header/span check only; not renewed full admission or a hash. */
int s1c_binding_valid(const s1c_binding *);
/* Logical rows [0,51], samples [0,4096]; sample4096 is the exact +0 guard.
 * Fixed aliases:48->44,49->45; rows50/51 map to physical48/49.
 * Output is an aligned writable uint32_t disjoint from the WHOLE binding and
 * payload. Failure leaves output unchanged; successful read preserves all
 * binary32 bits, including signed zero. Output may not alias borrowed bytes.
 * Out-of-range unsigned coordinates are rejected, not wrapped. */
int s1c_read(const s1c_binding *,unsigned,unsigned,uint32_t *);
#ifdef __cplusplus
}
#endif
#endif
