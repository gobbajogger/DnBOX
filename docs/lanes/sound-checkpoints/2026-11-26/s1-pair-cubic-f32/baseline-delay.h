#ifndef SOURCE_S1_DELAY_HOST_H
#define SOURCE_S1_DELAY_HOST_H
typedef struct source_s1_exact_delay source_s1_exact_delay;
/* Host only; allocate once per stereo owner, and process every sample even
 * when effect mix is zero. No format, RAM, target or performance claim. */
source_s1_exact_delay *source_s1_delay_new(unsigned rate);
void source_s1_delay_free(source_s1_exact_delay *);
void source_s1_delay_reset(source_s1_exact_delay *);
void source_s1_delay_tap(source_s1_exact_delay *,float time,float fb,float in_l,float in_r,float *out_l,float *out_r);
void source_s1_delay_insert(source_s1_exact_delay *,float time,float fb,float filter_type,float cutoff,float tracking_ratio,
    float mix,float enable,float phaser_l,float phaser_r,float *mixed_l,float *mixed_r);
#endif
