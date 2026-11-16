/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef DNBOX_SJ6_CONTROLS_H
#define DNBOX_SJ6_CONTROLS_H
#include <stddef.h>
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif
#define DNBOX_SJ6_NATIVE_COUNT 218u
#define DNBOX_SJ6_PAGE_COUNT 8u
#define DNBOX_SJ6_BLOB_BYTES 888u
/* Stable statuses for this new API only. No old engine/schema ID is reused. */
typedef enum {DNBOX_CTL_OK=0,DNBOX_CTL_BAD_ARGUMENT=1,DNBOX_CTL_BAD_ID=2,
 DNBOX_CTL_BAD_BLOB=3,DNBOX_CTL_NOT_ADMITTED=4} dnbox_control_status;
typedef struct {
 uint16_t native_id; uint16_t default_is_host_reference; uint32_t default_bits;
 const char *identifier,*units,*law;
} dnbox_sj6_control;
/* Caller must keep all supplied spans valid. These cold serialized helpers do
 * not activate/apply DSP and are not intended for the audio callback. Derived
 * default words are host references, not certified Apple platform pow results. */
const dnbox_sj6_control *dnbox_sj6_control_at(unsigned page,unsigned slot);
const char *dnbox_sj6_page_name(unsigned page);
dnbox_control_status dnbox_sj6_binding_status(unsigned native_id);
/* New standalone little-endian SJ61/version1/count218/payload872/CRC32 format.
 * NOT an extension or reinterpretation of existing 32-byte sound/project cells.
 * Explicit future app admission/migration is required before storing it there. */
dnbox_control_status dnbox_sj6_blob_encode(void*,size_t,const float*,size_t);
dnbox_control_status dnbox_sj6_blob_get(const void*,size_t,unsigned,float*);
dnbox_control_status dnbox_sj6_blob_set(void*,size_t,unsigned,float);
typedef struct {
 uint16_t native_id,flags;uint32_t minimum_bits,default_bits,maximum_bits;
 const char *identifier,*units;
} dnbox_s1_control;
/* flags bit0=dependent source setter; bit1=post-note FX domain. No generic linear
 * normalized mapping is promised. Actual renderer currently pre-FX host only. */
const dnbox_s1_control *dnbox_s1_control_at(unsigned native_id);
#ifdef __cplusplus
}
#endif
#endif
