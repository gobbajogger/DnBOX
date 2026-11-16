/* SPDX-License-Identifier: GPL-3.0-only */
/* Linked only when X0X_SUPERJ6=1. Keeps the parametric resynthesis reachable
 * for a flash measurement. It does not replace the analytic oscillators and
 * it does not play stored samples. */
extern void sj6_resynth_keep(void);

void x0x_superj6_keep(void)
{
    sj6_resynth_keep();
}
