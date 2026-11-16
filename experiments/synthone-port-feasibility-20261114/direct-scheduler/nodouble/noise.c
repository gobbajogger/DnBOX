#include <stdlib.h>
#include "soundpipe.h"

int sp_noise_create(sp_noise **ns)
{
    *ns = malloc(sizeof(sp_noise));
    return SP_OK;
}

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

int sp_noise_destroy(sp_noise **ns)
{
    free(*ns);
    return SP_OK;
}
