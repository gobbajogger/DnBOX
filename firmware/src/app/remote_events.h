/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
/* Unadmitted current-host protocol profile. No native endpoint is enabled.
 * Semantic changes originate in the main loop or ALNK0 application, not raw
 * MIDI/USB/BT IRQ queue arrival. The eventual receiver must serialize revision
 * access with the existing audio-source mask; this is not a general mutex. */
#if defined(X0X_DNR2_PROBE) && X0X_DNR2_PROBE
void dnbox_source_changed(void);
#else
static inline void dnbox_source_changed(void) {}
#endif
