/*
 * Moogladder
 *
 * This code has been extracted from the Csound opcode "Moogladder".
 * It has been modified to work as a Soundpipe module.
 *
 * Original Author(s): Victor Lazzarini
 * Year: 2001
 * Location: Opcodes/newfils.c
 */

#include "slot.h"

#include <stdint.h>
#include <stdlib.h>
#include "float_math.h"
#include "soundpipe.h"
S1_POOLED(sp_moogladder, sp_moogladder_create, sp_moogladder_destroy, moog)

#define SPFLOAT2LONG(x) lrintf(x)

/* Same branches as the Csound tanh. The |x|<0.5 arm is exact.
 * The middle arm uses the firmware tanh (absolute error 3e-7).
 * Audio samples times THERMAL stay in the exact arm. */
static float my_tanh(float x)
{
    float sign = 1.f;
    if (x < 0.f) {
        sign = -1.f;
        x = -x;
    }
    if (x >= 4.f)
        return sign;
    if (x < 0.5f)
        return x * sign;
    return sign * s1_tanhf(x);
}





int sp_moogladder_init(sp_data *sp, sp_moogladder *p){
    p->istor = 0.f;
    p->res = 0.4f;
    p->freq = 1000;
    int i;

    if (p->istor == 0.f) {
      for (i = 0; i < 6; i++)
        p->delay[i] = 0.f;
      for (i = 0; i < 3; i++)
        p->tanhstg[i] = 0.f;
      p->oldfreq = 0.f;
      p->oldres = -1.f;     /* ensure calculation on first cycle */
    }
    return SP_OK;
}

int sp_moogladder_compute(sp_data *sp, sp_moogladder *p, SPFLOAT *in, SPFLOAT *out){
    SPFLOAT freq = p->freq;
    SPFLOAT res = p->res;
    SPFLOAT res4;
    SPFLOAT *delay = p->delay;
    SPFLOAT *tanhstg = p->tanhstg;
    SPFLOAT stg[4], input;
    SPFLOAT acr, tune;
#define THERMAL (0.000025f) /* 1/40000 transistor thermal voltage */
    int     j, k;

    if (res < 0) res = 0;

    if (p->oldfreq != freq || p->oldres != res) {
        SPFLOAT f, fc, fc2, fc3, fcr;
        p->oldfreq = freq;
        /* sr is half the actual filter sampling rate  */
        fc =  (SPFLOAT)(freq/sp->sr);
        f  =  0.5f*fc;
        fc2 = fc*fc;
        fc3 = fc2*fc;
        /* frequency & amplitude correction  */
        fcr = 1.8730f*fc3 + 0.4955f*fc2 - 0.6490f*fc + 0.9988f;
        acr = -3.9364f*fc2 + 1.8409f*fc + 0.9968f;
        tune = (1.f - s1_expf(-((2.f * S1_PI)*f*fcr))) / THERMAL;   /* filter tuning  */
        p->oldres = res;
        p->oldacr = acr;
        p->oldtune = tune;
    } else {
        res = p->oldres;
        acr = p->oldacr;
        tune = p->oldtune;
    }
    res4 = 4.f*(SPFLOAT)res*acr;

    /* oversampling  */
    for (j = 0; j < 2; j++) {
        /* filter stages  */
        input = *in - res4 /*4.0*res*acr*/ *delay[5];
        delay[0] = stg[0] = delay[0] + tune*(my_tanh(input*THERMAL) - tanhstg[0]);
        for (k = 1; k < 4; k++) {
          input = stg[k-1];
          stg[k] = delay[k]
            + tune*((tanhstg[k-1] = my_tanh(input*THERMAL))
                    - (k != 3 ? tanhstg[k] : my_tanh(delay[k]*THERMAL)));
          delay[k] = stg[k];
        }
        /* 1/2-sample delay for phase compensation  */
        delay[5] = (stg[3] + delay[4])*0.5f;
        delay[4] = stg[3];
    }
    *out = (SPFLOAT) delay[5];
    return SP_OK;
}
