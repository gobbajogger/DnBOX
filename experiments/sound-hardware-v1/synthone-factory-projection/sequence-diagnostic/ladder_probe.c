/* SPDX-License-Identifier: GPL-3.0-only */
/* Host-only filter substitution in the original renderer. */
#include "soundpipe.h"

static float step(float *history, float pole, float feedback, float input)
{
    const float delayed = history[3];
    float value = input - feedback * (0.5f * (history[3] + history[4]));
    for (unsigned i = 0; i < 4; ++i) {
        value += pole * (history[i] - value);
        history[i] = value;
    }
    history[4] = delayed;
    return value;
}

int __wrap_sp_moogladder_compute(sp_data *sp, sp_moogladder *p, SPFLOAT *in, SPFLOAT *out)
{
    (void)sp;
    /* Keep original cutoff and resonance, with the candidate coefficient law
     * and recurrence from firmware/src/dsp/s1_dry_voice.c. */
    const float x = p->freq * (1.0f / 22050.0f);
    const float amount = x * (1.56874466f + x * (-1.73140822f +
                 x * (1.55879259f + x * (-0.711601815f + x * 0.118475673f))));
    const float pole = 1.0f - amount;
    const float feedback = p->res * (3.98720139f + x * (3.68178143f - x * 3.93633599f));
    const float first = step(p->delay, pole, feedback, *in);
    const float second = step(p->delay, pole, feedback, *in);
    *out = 0.5f * (first + second);
    return SP_OK;
}
