#pragma once
#include <stdint.h>
#include <stddef.h>
#include "soundpipe.h"
#ifdef __cplusplus
extern "C" {
#endif
/* S1 APP-lifecycle history adapter, NOT general Soundpipe vdelay replacement.
 * Caller owns initialized readable metadata, borrowed live writable storage and
 * sp_data context, adapter state, full physical storage, readable input and
 * writable output spans are pairwise disjoint and live for every call. The
 * compute API rejects these overlaps before any state/storage/output mutation.
 * Structural validation is not pointer
 * authentication. No allocation. Only44100/48000/96000Hz diagnostic domains.
 * Keeps original10s VIRTUAL ring/index/arithmetic. Reads unavailable older
 * history reject transactionally; they are NOT silently synthesized as zero.
 * Cold/reset unvisited virtual positions are proven +0 and read as +0.
 * Extended HOT port restore without FX clear is not admitted: it can expose
 * almost10s-old samples. Source AU restores only after init/reset.
 */
typedef struct {sp_vdelay v;uint32_t physical_words,physical_left,written,magic;float admitted_max_delay;}s1_history_delay;
size_t s1_history_words(int rate,float max_delay);
int s1_history_init(s1_history_delay*,int rate,float*storage,size_t words,float max_delay);
int s1_history_compute(sp_data*,s1_history_delay*,const float*input,float*output);
int s1_history_reset(s1_history_delay*);
#ifdef __cplusplus
}
#endif
