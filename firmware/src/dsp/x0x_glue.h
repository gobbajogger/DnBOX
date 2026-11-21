/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef X0X_GLUE_H
#define X0X_GLUE_H

#include "fastmath.h"

/* Original 8W8 and 9W9 use the same detector and soft knee. The caller
 * owns the detector and envelope so each kit or bus can recover separately. */
static inline void x0x_glue_prepare(float amount, float *threshold, float *slope, float *makeup)
{
    const float ratio = 2.0f + 3.0f * amount;
    *threshold = 10.0f - 18.0f * amount;
    *slope = 1.0f - 1.0f / ratio;
    *makeup = amount * 2.2f + amount * amount * 6.6f;
}

/* Soft-knee glue gain reduction — one body for mono + stereo (same pattern as
 * master soft_ceiling / djf_gsmooth). */
__attribute__((noinline)) static float x0x_glue_gr(float over, float slope)
{
    if (over >= 3.0f)
        return -over * slope;
    if (over > -3.0f) {
        const float t = over + 3.0f;
        return -(t * t) * (1.0f / 12.0f) * slope;
    }
    return 0.0f;
}

/* One glue sample: update det/env, return linear gain (mono + stereo). */
static inline __attribute__((always_inline)) float x0x_glue_gain(float mag, float thr, float slope, float drel,
                                                    float atk, float rel, float makeup,
                                                    float *det, float *env)
{
    *det = mag > *det ? mag : *det * drel;
    const float in_db = *det > 1e-9f ? 6.0205999f * fm_log2f(*det) : -120.0f;
    const float gr = x0x_glue_gr(in_db - thr, slope);
    *env = gr + (*env - gr) * (gr < *env ? atk : rel);
    return fm_db2lin(*env + makeup);
}

#endif
