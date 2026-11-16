#include "slot.h"

#include <stdlib.h>
#include "soundpipe.h"
S1_POOLED(sp_crossfade, sp_crossfade_create, sp_crossfade_destroy, crossfade)
/* 0.5 is exact in binary32, same bits as the double literal. */





int sp_crossfade_init(sp_data *sp, sp_crossfade *p)
{
    p->pos = 0.5f;
    return SP_OK;
}

int sp_crossfade_compute(sp_data *sp, sp_crossfade *p, SPFLOAT *in1, SPFLOAT *in2, SPFLOAT *out)
{
    *out = *in2 * p->pos + *in1 * (1 - p->pos);
    return SP_OK;
}
