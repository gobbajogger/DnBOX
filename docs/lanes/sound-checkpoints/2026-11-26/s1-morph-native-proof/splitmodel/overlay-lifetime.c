#include <stddef.h>
#include "app/original_runtime.h"
#include "dsp/drum909.h"
/* Illustrative layout only: no firmware design or scheduler edits in this unit. */
#define TAIL 1024u
#define CAP ((TAIL - sizeof(d9_smp_t) - 4u) / sizeof(float))
union witness_work {
    original_runtime_workspace current;
    struct {
        float metal[4][256];
        struct {d9_smp_t next_state; uint32_t ready_and_frames; float pcm[CAP];} sampler;
    } split;
};
_Static_assert(offsetof(union witness_work, split.sampler)==4096,"isolate from live 808 metal lanes");
_Static_assert(sizeof(union witness_work)==sizeof(original_runtime_workspace),"no arena growth");
_Static_assert(sizeof(((union witness_work*)0)->split.sampler)<=TAIL,"remaining exact arena bound");
unsigned char receipt_workspace[sizeof(original_runtime_workspace)];
unsigned char receipt_smp_state[sizeof(d9_smp_t)];
unsigned char receipt_tail_pcm_frames[CAP];
unsigned char receipt_prepared_future[sizeof(((union witness_work*)0)->split.sampler)];
