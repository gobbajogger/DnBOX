#ifndef DNBX_HOST_SJ6_NORMAL_CUTOFF_H
#define DNBX_HOST_SJ6_NORMAL_CUTOFF_H
#include <math.h>
/* Older original's Normal target 0x1003a6448–6538, not the OTA recipient.
 * base/strength/keytrack/velocity_amount are post-receiver linear/physical
 * inputs. This does not infer original control IDs or raw mappings. */
static inline double sj6_normal_cutoff(float base,float strength,float velocity_amount,
                                       float velocity,float env,float keytrack,float pitch)
{
    const float headroom=22050.f-base;
    const float scale=fmaf(velocity_amount,velocity,1.f-velocity_amount);
    const float range=(strength*scale)*env;
    double fc=(double)base+(double)(headroom*range);
    if(pitch>130.80999755859375f){
        const float relative=pitch/130.80999755859375f;
        const float exponent=fmaxf(1.f/(12.f-log2f(relative)),1.f/24.f);
        const float delta=powf(relative,exponent)-1.f;
        fc+=(double)(((keytrack*delta)*headroom)/3.3299999237060547f);
    }
    return fmin(fc,22050.0);
}
#endif
