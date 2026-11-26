/* SPDX-License-Identifier: GPL-3.0-only */
#include "join.h"
#define TAG(epoch,phase) (((epoch)<<2)|(phase))
#if defined(DNBX_PI32)
static inline uint32_t acquire(const join_word *w) {
    uint32_t v=*w;
    __asm__ volatile("csync" ::: "memory");
    return v;
}
static inline void release(join_word *w,uint32_t v) {
    __asm__ volatile("csync" ::: "memory");
    *w=v;
}
#else
#define acquire(w) atomic_load_explicit((w),memory_order_acquire)
#define release(w,v) atomic_store_explicit((w),(v),memory_order_release)
#endif

/* Called after the original platform DONE/join. Even a terminal owner
 * acknowledgment does not itself let another platform job reuse fn/arg. */
int join_issue(join_word *request,const join_word *reply,uint32_t generation) {
    uint32_t old=acquire(request)>>2;
    uint32_t ack=acquire(reply);
    if(generation==0 || generation>=1u<<30 || generation<=old) return 0;
    if(old && ack!=TAG(old,COMPLETED) && ack!=TAG(old,DECLINED)) return 0;
    release(request,TAG(generation,RUN));
    return 1;
}

void join_cancel(join_word *request,uint32_t generation) {
    if(generation && acquire(request)==TAG(generation,RUN))
        release(request,TAG(generation,CANCEL));
}

enum join_result join_poll(const join_word *reply,uint32_t generation) {
    uint32_t ack=acquire(reply);
    if(!generation) return HOLD_OWNER;
    if(ack==TAG(generation,COMPLETED)) return USE_REMOTE;
    if(ack==TAG(generation,DECLINED)) return RENDER_LOCAL;
    return HOLD_OWNER;
}

/* One invocation per admitted platform job. Worker maintains no waits on C0
 * mix state. Stale/nonvalid calls do not touch owner or publish old replies.
 * Cancellation is not accepted after sampling RUN: C0 must await COMPLETED. */
void join_work(const join_word *request,join_word *reply,uint32_t generation,
               void (*render_only)(void *),void *owner) {
    uint32_t state=acquire(request);
    if(!generation || (acquire(reply)>>2)>=generation) return;
    if(state==TAG(generation,CANCEL)) {
        release(reply,TAG(generation,DECLINED));
    } else if(state==TAG(generation,RUN)) {
        render_only(owner);
        release(reply,TAG(generation,COMPLETED));
    }
}
