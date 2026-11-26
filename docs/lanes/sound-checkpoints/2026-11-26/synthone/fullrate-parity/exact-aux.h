/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef DNBX_SOURCE_AUX_HOST_H
#define DNBX_SOURCE_AUX_HOST_H
void *exact_aux_new(void);
void exact_aux_drop(void *state);
float exact_aux_sample(void *state,float subfreq,float fmfreq,float amp,float square,
                       float subvol,float fmidx,float fmvol);
#endif
