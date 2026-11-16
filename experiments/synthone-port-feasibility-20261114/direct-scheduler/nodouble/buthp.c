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
S1_POOLED(sp_buthp, sp_buthp_create, sp_buthp_destroy, buthp)

/* Filter loop */

static int sp_butter_filter(SPFLOAT *in, SPFLOAT *out, SPFLOAT *a)
{
    SPFLOAT t, y;
    t = *in - a[4] * a[6] - a[5] * a[7];
    y = t * a[1] + a[2] * a[6] + a[3] * a[7];
    a[7] = a[6];
    a[6] = t;
    *out = y;
    return SP_OK;
}






int sp_buthp_init(sp_data *sp, sp_buthp *p)
{
    p->istor = 0.f;
    p->sr = sp->sr;
    p->freq = 1000;
    s1_butbp_consts(sp->sr, &p->pidsr, 0);
    if (p->istor==0.f) {
        p->a[6] = p->a[7] = 0.f;
        p->lkf = 0.f;
    }
    return SP_OK;
}

int sp_buthp_compute(sp_data *sp, sp_buthp *p, SPFLOAT *in, SPFLOAT *out)
{
    if (p->freq <= 0.f)     {
      *out = 0;
      return SP_OK;
    }

    if (p->freq != p->lkf)      {
      p->lkf = p->freq;
      s1_buthp_coeffs(p->pidsr, p->lkf, p->a);
    }
    sp_butter_filter(in, out, p->a);
    return SP_OK;
}
