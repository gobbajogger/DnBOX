/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef DNBX_SOURCE_MORPH_HOST_H
#define DNBX_SOURCE_MORPH_HOST_H
#include <stdint.h>
int exact_morph_load(void);
float exact_morph_sample(uint32_t *phase,float frequency,float position,int bandlimited);
#endif
