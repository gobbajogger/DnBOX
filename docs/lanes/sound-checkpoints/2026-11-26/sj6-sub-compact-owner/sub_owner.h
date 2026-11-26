/* Host structure witness for original sampled ownership using the existing
 * compact harmonic player. No sample data; Sub receiver owns commit timing. */
#ifndef DNBOX_SUB_OWNER_H
#define DNBOX_SUB_OWNER_H
#include <stdint.h>
typedef struct {
    float phase;
    uint8_t active_key,pending_key,active,pending;
} dnbox_sub_owner;
static inline void dnbox_sub_offer(dnbox_sub_owner *v,uint8_t note,int releasing_or_sounding)
{
    if(v->active && releasing_or_sounding) {
        v->pending_key=note;v->pending=1;
    } else {
        *v=(dnbox_sub_owner){.active_key=note,.active=1};
    }
}
static inline void dnbox_sub_cancel(dnbox_sub_owner *v){ v->pending=0; }
static inline int dnbox_sub_commit(dnbox_sub_owner *v)
{
    if(!v->active || !v->pending)return 0;
    v->active_key=v->pending_key;
    v->pending=0;
    v->phase=0.f;
    return 1;
}
static inline void dnbox_sub_advance(dnbox_sub_owner *v,float acoustic_step)
{
    v->phase+=acoustic_step;
    if(v->phase>=1.f)v->phase-=1.f;
}
#endif
