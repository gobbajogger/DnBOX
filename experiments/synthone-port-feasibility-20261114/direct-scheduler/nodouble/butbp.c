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

#include "slot.h"

#include <stdint.h>
#include <stdlib.h>
#include "exact.h"
#include "soundpipe.h"
S1_POOLED(sp_butbp, sp_butbp_create, sp_butbp_destroy, butbp)





int sp_butbp_init(sp_data *sp, sp_butbp *p)
{
    p->istor = 0.f;
    p->sr = sp->sr;
    p->freq = 1000;
    p->bw = 10;
    s1_butbp_consts(sp->sr, &p->pidsr, &p->tpidsr);
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
        p->lkf = fr;
        p->lkb = bw;
        s1_butbp_coeffs(p->pidsr, p->tpidsr, bw, fr, a);
    }
    t = *in - a[4] * a[6] - a[5] * a[7];
    y = t * a[1] + a[2] * a[6] + a[3] * a[7];
    a[7] = a[6];
    a[6] = t;
    *out = y;
    return SP_OK;
}
