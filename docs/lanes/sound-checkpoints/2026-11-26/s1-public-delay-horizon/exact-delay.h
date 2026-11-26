#ifndef DNBOX_S1_EXACT_DELAY_HOST_H
#define DNBOX_S1_EXACT_DELAY_HOST_H
#include <stddef.h>
typedef struct dnbox_s1_exact_delay dnbox_s1_exact_delay;
/* Host only; allocate once per stereo owner, and process every sample even
 * when effect mix is zero. No format, RAM, target or performance claim. */
dnbox_s1_exact_delay *dnbox_s1_delay_new(unsigned rate);
void dnbox_s1_delay_free(dnbox_s1_exact_delay *);
void dnbox_s1_delay_reset(dnbox_s1_exact_delay *);
size_t dnbox_s1_delay_history_bytes(const dnbox_s1_exact_delay *);
void dnbox_s1_delay_tap(dnbox_s1_exact_delay *,float time,float fb,float in_l,float in_r,float *out_l,float *out_r);
void dnbox_s1_delay_insert(dnbox_s1_exact_delay *,float time,float fb,float filter_type,float cutoff,float tracking_ratio,
    float mix,float enable,float phaser_l,float phaser_r,float *mixed_l,float *mixed_r);
#endif
