/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef DNBX_COOPERATIVE_JOIN_H
#define DNBX_COOPERATIVE_JOIN_H
#include <stdint.h>
#if defined(DNBX_PI32)
typedef volatile uint32_t join_word;
#else
#include <stdatomic.h>
typedef _Atomic uint32_t join_word;
#endif

/* Exactly one publication word per writer. Place on the respective existing
 * mailbox lanes, not together across cores. Nonzero generation occupies upper 30 bits. */
enum join_phase { RUN=1, CANCEL=2, COMPLETED=1, DECLINED=2 };
enum join_result { HOLD_OWNER, USE_REMOTE, RENDER_LOCAL };
int join_issue(join_word *request,const join_word *reply,uint32_t generation);
void join_cancel(join_word *request,uint32_t generation);
enum join_result join_poll(const join_word *reply,uint32_t generation);
void join_work(const join_word *request,join_word *reply,uint32_t generation,
               void (*render_only)(void *),void *owner);
#endif
