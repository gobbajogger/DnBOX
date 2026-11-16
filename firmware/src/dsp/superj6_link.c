/* SPDX-License-Identifier: GPL-3.0-only */
/* Linked only when X0X_SUPERJ6=1. Keeps the voice reachable for a flash
 * measurement. Default images do not compile this file. */
#include "sj6_analytic.h"

extern void sj6_resynth_keep(void);
extern void sj6_blocks_keep(void);

void x0x_superj6_keep(void)
{
    Sj6Osc osc;
    Sj6Noise noise;
    sj6_resynth_keep();
    sj6_blocks_keep();
    sj6_osc_reset(&osc);
    osc.increment = 0.01f;
    sj6_osc_saw(&osc, 1.f);
    sj6_osc_restart(&osc);
    osc.increment = 0.01f;
    sj6_osc_triangle(&osc, 1.f);
    sj6_osc_restart(&osc);
    osc.increment = 0.01f;
    sj6_osc_sine(&osc, 1.f);
    sj6_osc_restart(&osc);
    osc.increment = 0.01f;
    sj6_osc_pwm(&osc, 1.f, 0.5f);
    noise.seed = 1;
    sj6_noise_reset(&noise);
    sj6_noise_sample(&noise);
}
