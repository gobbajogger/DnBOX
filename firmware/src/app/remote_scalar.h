/* SPDX-License-Identifier: GPL-3.0-only */
#pragma once
/* Source-authoritative scalar transaction, not a transport or catalog.
 * Main-loop/single-producer entry only. The existing audio-source mask is
 * not a general multi-thread or multi-producer mutex.
 * bank is -1 for fixed targets, or exact Chicago/Miami bank for T_303.
 * 0: rejected without mutation. 2: stored target and one audio command queued.
 * This never means audio-applied. Caller still owns protocol revision and
 * endpoint admission; no endpoint is enabled by providing this entry point. */
int dnbox_remote_scalar_set(unsigned target, unsigned voice, unsigned parameter,
                            int bank, unsigned value);
