/*
 * Butterworth Filters
 *
 * This code has been extracted from the Csound butterworth filter opcodes
 * It has been modified to work as a Soundpipe module.
 *
 * Original Author(s): Paris Smaragdis, John ffitch
 * Year: 1994
 * Location: Opcodes/butter.c
 *
 */

#include <stdint.h>
#include <stdlib.h>
#include "float_math.h"
#include "soundpipe.h"

int sp_butbp_create(sp_butbp **p)
{
    *p = malloc(sizeof(sp_butbp));
    return SP_OK;
}

int sp_butbp_destroy(sp_butbp **p)
{
    free(*p);
    return SP_OK;
}

int sp_butbp_init(sp_data *sp, sp_butbp *p)
{
    p->istor = 0.f;
    p->sr = sp->sr;
    p->freq = 1000;
    p->bw = 10;
    p->pidsr = S1_PI / sp->sr;
    p->tpidsr = 2.f * S1_PI / sp->sr;
    p->a[6] = p->a[7] = 0.f;
    p->lkf = 0.f;
    p->lkb = 0.f;
    return SP_OK;
}

int sp_butbp_compute(sp_data *sp, sp_butbp *p, SPFLOAT *in, SPFLOAT *out)
{
    SPFLOAT *a = p->a;
    SPFLOAT t, y;

    if (p->bw <= 0.f) {
       *out = 0;
       return SP_OK;
    }

    SPFLOAT bw, fr;
    bw = p->bw;
    fr = p->freq;

    if (bw != p->lkb || fr != p->lkf) {
        SPFLOAT c, d;
        p->lkf = fr;
        p->lkb = bw;
        c = 1.f / s1_tanf(p->pidsr * bw);
        d = 2.f * s1_cosf(p->tpidsr * fr);
        a[1] = 1.f / (1.f + c);
        a[2] = 0.f;
        a[3] = -a[1];
        a[4] = - c * d * a[1];
        a[5] = (c - 1.f) * a[1];
    }
    t = *in - a[4] * a[6] - a[5] * a[7];
    y = t * a[1] + a[2] * a[6] + a[3] * a[7];
    a[7] = a[6];
    a[6] = t;
    *out = y;
    return SP_OK;
}
