/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef DNBOX_SYNTHONE_C_H
#define DNBOX_SYNTHONE_C_H
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif
typedef struct dnbox_s1 dnbox_s1;
/* Pinned pi32v2 pre-FX allocation contract; compilation verifies both values.
 * Other architectures must query owner_bytes/alignment instead. No FX included. */
#define DNBOX_S1_PI32V2_OWNER_BYTES 7972u
#define DNBOX_S1_PI32V2_OWNER_ALIGNMENT 4u
typedef struct {
    const void *compact; size_t compact_bytes;
    const float *sine; size_t sine_count;
    const float *frequency_bands; /* exactly13 floats */
    const float *parameters; /* exactly150 resolved physical values */
    const float *tuning; /* exactly128 frequencies */
    int sample_rate;
} dnbox_s1_config;
/* Single serialized owner; every pointer must denote its declared live span.
 * Cold init needs unoccupied aligned storage. Input data must not overlap it;
 * compact/sine/tuning remain borrowed and immutable until retire. No heap.
 * This is pre-FX only, NOT the original dependent-parameter UI frontend. */
size_t dnbox_s1_owner_bytes(void);
size_t dnbox_s1_owner_alignment(void);
dnbox_s1 *dnbox_s1_init(void *storage,size_t bytes,const dnbox_s1_config *);
/* All handle operations require a live handle returned by init, or NULL.
 * Retire ends its lifetime. Never render or retire a stale/forged handle. */
void dnbox_s1_retire(dnbox_s1 *);
int dnbox_s1_parameter(dnbox_s1 *,unsigned,float);
int dnbox_s1_note_on(dnbox_s1 *,int,int);
int dnbox_s1_note_off(dnbox_s1 *,int);
int dnbox_s1_all_off(dnbox_s1 *);
int dnbox_s1_reset(dnbox_s1 *);
int dnbox_s1_render(dnbox_s1 *,unsigned,float *,float *);
#ifdef __cplusplus
}
#endif
#endif
