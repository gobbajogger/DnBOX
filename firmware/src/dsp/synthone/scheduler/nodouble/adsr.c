#include "slot.h"

#include <stdlib.h>
#include "exact.h"
#include "soundpipe.h"
S1_POOLED(sp_adsr, sp_adsr_create, sp_adsr_destroy, adsr)

enum { CLEAR, ATTACK, DECAY, SUSTAIN, RELEASE };





int sp_adsr_init(sp_data *sp, sp_adsr *p)
{
    p->atk = 0.1f;
    p->dec = 0.1f;
    p->sus = 0.5f;
    p->rel = 0.3f;
    p->timer = 0;
    p->a = 0;
    p->b = 0;
    p->y = 0;
    p->x = 0;
    p->prev = 0;
    p->atk_time = p->atk * sp->sr;
    p->mode = CLEAR;
    return SP_OK;
}

static SPFLOAT tau2pole(sp_data *sp, sp_adsr *p, SPFLOAT tau)
{
    return s1_adsr_pole(tau, sp->sr);
}

static SPFLOAT adsr_filter(sp_data *sp, sp_adsr *p)
{
    p->y = p->b * p->x  + p->a * p->y;
    return p->y;
}

int sp_adsr_compute(sp_data *sp, sp_adsr *p, SPFLOAT *in, SPFLOAT *out)
{
    SPFLOAT pole;
    if(p->prev < *in && p->mode != DECAY) {
        p->mode = ATTACK;
        p->timer = 0;
        /* quick fix: uncomment if broken */
        /* pole = tau2pole(sp, p, p->atk * 0.75); */
        /* p->atk_time = p->atk * sp->sr * 1.5; */
        pole = tau2pole(sp, p, s1_round_mul(p->atk, 0x3fe3333333333333ull));
        p->atk_time = p->atk * sp->sr;
        p->a = pole;
        p->b = 1 - pole;
    } else if(p->prev > *in) {
        p->mode = RELEASE;
        pole = tau2pole(sp, p, p->rel);
        p->a = pole;
        p->b = 1 - pole;
    }

    p->x = *in;
    p->prev = *in;

    switch(p->mode) {
        case CLEAR:
            *out = 0;
            break;
        case ATTACK:
            p->timer++;
            *out = adsr_filter(sp, p);
            /* quick fix: uncomment if broken */
            /* if(p->timer > p->atk_time) { */
            if(s1_f_gt_d(*out, 0x3fefae147ae147aeull)) {
                p->mode = DECAY;
                pole = tau2pole(sp, p, p->dec);
                p->a = pole;
                p->b = 1 - pole;
            }
            break;
        case DECAY:
        case RELEASE:
            p->x *= p->sus;
            *out = adsr_filter(sp, p);
        default:
            break;
    }

    return SP_OK;
}
