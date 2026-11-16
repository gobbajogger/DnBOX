/*
 * Osc
 *
 * This code has been extracted from the Csound opcode "oscili".
 * It has been modified to work as a Soundpipe module.
 *
 * Original Author(s): Barry Vercoe, John FFitch, Robin Whittle
 * Year: 1991
 * Location: OOps/ugens2.c
 *
 */

#include "slot.h"

#include <stdlib.h>
#include "float_math.h"
#include "soundpipe.h"
S1_POOLED(sp_osc, sp_osc_create, sp_osc_destroy, osc)





int sp_osc_init(sp_data *sp, sp_osc *osc, sp_ftbl *ft, SPFLOAT iphs)
{
    osc->freq = 440.f;
    osc->amp = 0.2f;
    osc->tbl = ft;
    osc->iphs = s1_fabsf(iphs);
    osc->inc = 0;
    if (osc->iphs >= 0){
        osc->lphs = ((int32_t)(osc->iphs * SP_FT_MAXLEN)) & SP_FT_PHMASK;
    }

    return SP_OK;
}

int sp_osc_compute(sp_data *sp, sp_osc *osc, SPFLOAT *in, SPFLOAT *out)
{
    sp_ftbl *ftp;
    SPFLOAT amp, cps, fract, v1, v2, *ft;
    int32_t phs, lobits;
    int32_t pos;
    SPFLOAT sicvt = osc->tbl->sicvt;

    ftp = osc->tbl;
    lobits = osc->tbl->lobits;
    amp = osc->amp;
    cps = osc->freq;
    phs = osc->lphs;
    ft = osc->tbl->tbl;
    
    osc->inc = s1_lrintf(cps * sicvt);

    fract = ((phs) & ftp->lomask) * ftp->lodiv;
    pos = phs>>lobits;
    v1 = *(ft + pos);
    v2 = *(ft + ((pos + 1) % ftp->size));
    *out = (v1 + (v2 - v1) * fract) * amp;
    phs += osc->inc;
    phs &= SP_FT_PHMASK;

    osc->lphs = phs;
    return SP_OK;
}
