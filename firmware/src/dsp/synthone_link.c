/* SPDX-License-Identifier: GPL-3.0-only */
/* Linked only when X0X_SYNTHONE=1. Keeps the no-double Synth One closure
 * reachable for a flash/pool/RAM measurement. It does not render audio. */
extern void s1_scheduler_keep(void);
extern void s1_pool_keep(void);
extern float sp_midi2cps(float);

void x0x_synthone_keep(void)
{
    volatile float hz;
    s1_scheduler_keep();
    s1_pool_keep();
    hz = sp_midi2cps(60.f);
    (void)hz;
}
