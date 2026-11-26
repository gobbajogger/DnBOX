/* SPDX-License-Identifier: GPL-3.0-only */
#include "mb_join.h"
/* A valid nonzero request generation and single accepted invocation are
 * guarantees of the enclosing mailbox. Disallow wrap at the caller. */
int mb_decline(const join_word *cancel,join_word *declined,uint32_t generation) {
#if defined(DNBX_PI32)
    if(*cancel!=generation) return 0;
    *declined=generation;
#else
    if(atomic_load_explicit(cancel,memory_order_acquire)!=generation) return 0;
    atomic_store_explicit(declined,generation,memory_order_release);
#endif
    /* Actual HAL publishes matching DONE with a barrier after return. */
    return 1;
}

enum join_result mb_poll(const join_word *done,const join_word *declined,uint32_t generation) {
#if defined(DNBX_PI32)
    if(*done!=generation) return HOLD_OWNER;
    __asm__ volatile("csync" ::: "memory");
    const uint32_t denied=*declined;
#else
    if(atomic_load_explicit(done,memory_order_acquire)!=generation) return HOLD_OWNER;
    const uint32_t denied=atomic_load_explicit(declined,memory_order_relaxed);
#endif
    return denied==generation ? RENDER_LOCAL : USE_REMOTE;
}
