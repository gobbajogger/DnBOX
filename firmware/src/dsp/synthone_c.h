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
#if defined(S1_HARMONIC_ONLY)
#define DNBOX_S1_PI32V2_OWNER_BYTES 6552u /* expected7972-51*28+8; native assertion must verify */
#else
#define DNBOX_S1_PI32V2_OWNER_BYTES 7972u
#endif
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
 * Pre-FX only. Full app tuning/UI transport and post-FX remain outside this ABI. */
size_t dnbox_s1_owner_bytes(void);
size_t dnbox_s1_owner_alignment(void);
dnbox_s1 *dnbox_s1_init(void *storage,size_t bytes,const dnbox_s1_config *);
#if defined(S1_HARMONIC_ONLY)
/* Experimental harmonic backend; compact MUST be NULL and compact_bytes=0.
 * Same borrowed sine/tuning and serialized ownership contract; not native admission. */
dnbox_s1 *dnbox_s1_init_harmonic(void *storage,size_t bytes,const dnbox_s1_config *);
#endif
#if defined(S1_HARMONIC_ONLY) && defined(S1_CANONICAL_SINE)
/* Explicit canonical original sine. sine MUST be NULL and sine_count=4097;
 * compact MUST remain NULL/0. Generic nonnull-sine API retains its semantics. */
dnbox_s1 *dnbox_s1_init_harmonic_canonical_sine(void *storage,size_t bytes,const dnbox_s1_config *);
#endif
/* All handle operations require a live handle returned by init, or NULL.
 * Retire ends its lifetime. Never render or retire a stale/forged handle. */
void dnbox_s1_retire(dnbox_s1 *);
/* Raw resolved physical target; no dependent source laws applied. */
int dnbox_s1_parameter(dnbox_s1 *,unsigned,float);
/* Original scalar helper laws (clamping, tempo/rate dependencies, portamento).
 * Transient serialized scratch; no resident owner growth and no UI notifications.
 * These are not the complete original app tuning/hold/modwheel/preset frontend.
 * normalized accepts only the six original dependent parameters and [0,1]. */
int dnbox_s1_source_parameter(dnbox_s1 *,unsigned,float);
int dnbox_s1_source_normalized(dnbox_s1 *,unsigned,float);
/* Returns the resolved target, not the instantaneous smoothed sample value.
 * out must be a live writable float disjoint from owner and immutable inputs. */
int dnbox_s1_target(const dnbox_s1 *,unsigned,float *out);
int dnbox_s1_note_on(dnbox_s1 *,int,int);
int dnbox_s1_note_off(dnbox_s1 *,int);
int dnbox_s1_all_off(dnbox_s1 *);
int dnbox_s1_reset(dnbox_s1 *);
int dnbox_s1_render(dnbox_s1 *,unsigned,float *,float *);
#ifdef __cplusplus
}
#endif
#endif
