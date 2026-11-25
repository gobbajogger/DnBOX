/* SPDX-License-Identifier: GPL-3.0-only */
#include "s1_pan_port.h"
float physical_select(unsigned detent) { return s1_pan_free_select(detent); }
unsigned physical_reverse(float hz) { return s1_pan_free_position(hz); }
uint32_t physical_phase_update(float rate,float scale) { return s1_pan_q24_increment(rate,scale); }
