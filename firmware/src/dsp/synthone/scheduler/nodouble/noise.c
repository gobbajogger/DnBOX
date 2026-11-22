#include "slot.h"

#include <stdlib.h>
#include "soundpipe.h"
S1_POOLED(sp_noise, sp_noise_create, sp_noise_destroy, noise)



int sp_noise_init(sp_data *sp, sp_noise *ns)
{
    ns->amp = 1.f;
    return SP_OK;
}

int sp_noise_compute(sp_data *sp, sp_noise *ns, SPFLOAT *in, SPFLOAT *out)
{
    *out = ((sp_rand(sp) % SP_RANDMAX) / (SP_RANDMAX * 1.f));
    *out = (*out * 2) - 1;
    *out *= ns->amp;
    return SP_OK;
}


