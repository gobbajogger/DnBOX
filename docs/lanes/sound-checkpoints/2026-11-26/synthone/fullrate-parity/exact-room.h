/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef DNBOX_EXACT_ROOM_HOST_H
#define DNBOX_EXACT_ROOM_HOST_H
/* Host sound-parity only, unrestricted histories. Own exactly one context per part. */
enum { ROOM_INPUT_COMP=0, ROOM_WET_COMP, ROOM_MASTER_COMP };
void *exact_room_new(float sr);
void exact_room_drop(void *state);
void exact_room_hp(void *s, float frequency, float *l, float *r);
void exact_room_comp(void *s, unsigned which, float ratio, float threshold,
                     float attack, float release, float makeup, float *l, float*r);
void exact_room_return(void *s, float fb, float l, float r, float *wl, float *wr);
#endif
