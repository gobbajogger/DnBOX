/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef DNBX_S1_WHOLE_EXACT_FILTER_H
#define DNBX_S1_WHOLE_EXACT_FILTER_H
void *exact_voice_filter_new(float rate);
void exact_voice_filter_drop(void *o);
float exact_voice_filter(void *o,float input,float frequency,float resonance,float type,float mix);
#endif
