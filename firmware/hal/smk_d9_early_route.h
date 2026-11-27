/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
#include <stdint.h>
typedef struct { volatile uint32_t live_id, button_epoch; uint32_t last_id; } d9_lease;
typedef struct {
    uint64_t down_before, down_after;
    uint32_t button_epoch, pad_notes, button_now;
    unsigned debounce, encoder_pending, other_pending;
} d9_postscan;
typedef struct { uint32_t lease_id; uint8_t physical, voice, velocity; } d9_admit;
static inline uint32_t d9_epoch_next(uint32_t n) { return n == UINT32_MAX ? n : n+1u; }
static inline void d9_lease_close(d9_lease *l) { l->live_id=0; }
static inline void d9_lease_open(d9_lease *l,int safe,uint32_t ack,uint32_t now)
{
    d9_lease_close(l);
    if (!safe || ack != now || now == UINT32_MAX || l->last_id == UINT32_MAX) return;
    l->last_id=d9_epoch_next(l->last_id);
    if (l->last_id==UINT32_MAX) return;
    l->button_epoch=now;
    __asm__ volatile("" ::: "memory");
    l->live_id=l->last_id;
}
static inline int d9_scan_admit(const d9_lease *l,const d9_postscan *s,unsigned k,unsigned velocity,
                                const uint8_t black[11],d9_admit *out)
{
    uint32_t id=l->live_id;
    if (!id || s->button_epoch==UINT32_MAX || s->button_epoch!=l->button_epoch ||
        s->button_now || s->debounce || s->encoder_pending || s->other_pending || s->pad_notes ||
        k<5u || k>=32u || !velocity || velocity>127u) return 0;
    if (s->down_before || s->down_after!=(UINT64_C(1)<<k)) return 0;
    unsigned v=0;
    while (v<11u && black[v]!=k-5u) ++v;
    if (v==11u) return 0;
    *out=(d9_admit){id,k,v,velocity};
    return 1;
}
