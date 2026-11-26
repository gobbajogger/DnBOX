/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef DNBX_SOURCE_STEREO_HOST_H
#define DNBX_SOURCE_STEREO_HOST_H
void *exact_stereo_new(void);
void exact_stereo_drop(void *ptr);
void exact_stereo(void *ptr,float mono,float pan_hz,float pan_depth,float ph_mix,
                  float ph_bpm,float ph_notch,float ph_fb,float *l,float *r);
#endif
