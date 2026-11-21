/* SPDX-License-Identifier: GPL-3.0-only */
#include "../../../experiments/sound-hardware-v1/synthone-widen/widen.h"
enum { S1_WIDEN_FIRST = 1365 };
_Static_assert(sizeof original_fx[0].arena >= 3 * S1_WIDEN_FIRST, "inactive Chicago span");
_Static_assert(sizeof original_bass[0].dsp.synthone.widen_tail == S1_WIDEN_BYTES - 3 * S1_WIDEN_FIRST,
               "complete 50 ms history");
#ifdef X0X_WIDEN_PROBE
extern void s1_widen_observe(int part, float input, float tail, float target, float applied, float result);
extern void s1_widen_cold(int part);
#endif

static int s1_widen_ready(int v)
{
    return original_bass[v].kind == ORIGINAL_SOUND_SYNTHONE &&
           original_bass[v].status == ORIGINAL_RUNTIME_OK && !original_fx[v].selected;
}

static void s1_widen_adopt(int v, int rc)
{
    if (rc != ORIGINAL_RUNTIME_OK || !s1_widen_ready(v)) return;
    /* Cold runtime init cleared this part's tail, index and smoothed gain. */
    memset(&original_fx[v].arena, 0, 3u * S1_WIDEN_FIRST);
#ifdef X0X_WIDEN_PROBE
    s1_widen_cold(v);
#endif
}

static void s1_widen_render(int v, float *right, unsigned frames)
{
    if (!s1_widen_ready(v)) return;
    s1_dry_part *p = &original_bass[v].dsp.synthone;
    unsigned index = p->widen_frame;
    float wet = p->widen_current;
    const float target = p->widen_target;
    for (unsigned i = 0; i < frames; ++i) {
        float input = right[i];
        float delay = s1_widen_tap((uint8_t *)&original_fx[v].arena, S1_WIDEN_FIRST,
                                   p->widen_tail, &index, input);
        /* Source Soundpipe port.c, fixed INIT half-time .1 s at 44100 Hz.
         * c2 = (float)pow(.5, (float)(1/44100.) / .1f), c1 = 1.f-c2. */
        wet = 0x1.49a000p-13f * target + 0x1.ffeb66p-1f * wet;
        if (wet != 0.f) right[i] = wet * delay + (1.f - wet) * input;
#ifdef X0X_WIDEN_PROBE
        s1_widen_observe(v, input, delay, target, wet, right[i]);
#endif
    }
    p->widen_frame = index;
    p->widen_current = wet;
}
