/* SPDX-License-Identifier: GPL-3.0-only */
/* Cold production bass part, before the track mixer. The caller supplies the
 * public control readbacks in ID order for a custom instrument stimulus. */
#include "app/original_runtime.h"
#include "chicago_fx2_compact.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { RATE = 44100, BLOCK = 256 };
static original_runtime_part part;
static original_runtime_workspace workspace;
static original_chicago_tuning tuning;
static original_chicago_fx2_compact fx;

int main(int argc, char **argv)
{
    if (argc < 3) return 2;
    unsigned kind;
    if (!strcmp(argv[1], "miami")) kind = ORIGINAL_SOUND_MIAMI;
    else if (!strcmp(argv[1], "chicago")) kind = ORIGINAL_SOUND_CHICAGO;
    else return 2;
    char *end;
    unsigned long program = strtoul(argv[2], &end, 10);
    unsigned count = original_sound_count(kind);
    if (!*argv[2] || *end || program >= original_sound_factories(kind) ||
        (argc != 3 && (unsigned)argc != 3 + count)) return 2;

    uint8_t cell[ORIGINAL_SOUND_BYTES];
    if (!original_sound_init(cell, kind, (unsigned)program) ||
        !original_chicago_tuning_bind(&tuning, 0, 0, NULL, 0x6d70) ||
        !original_chicago_fx2_compact_init(&fx)) return 1;
    original_chicago_fx2_cold_owner owner = original_chicago_fx2_compact_owner(&fx);
    if (original_runtime_init(&part, cell, 120.f, &tuning, &owner) != ORIGINAL_RUNTIME_OK) return 1;
    for (int i = 3; i < argc; ++i) {
        float value = strtof(argv[i], &end);
        if (!*argv[i] || *end || original_runtime_param(&part, (unsigned)(i - 3), value) != ORIGINAL_RUNTIME_OK)
            return 1;
    }

    /* Interleave control events with the same maximum slice and split points
     * used by the AU capture. The caller transmits only MIDI notes and slice
     * boundaries; this keeps all sound generation inside the production path. */
    unsigned frames = 0, events = 0, cuts = 0;
    unsigned ticks[16], keys[16], velocities[16], boundaries[16];
    int probe_anchor_frames;
    if (scanf("%u%u%u%d", &frames, &events, &cuts, &probe_anchor_frames) != 4 ||
        !frames || frames > 2 * RATE || events > 16 || cuts > 16 ||
        probe_anchor_frames < -1 || probe_anchor_frames > RATE) return 2;
    /* -1 preserves cold production state for every qualification row. The
     * optional diagnostic isolates the missing native timer initialization. */
    if (probe_anchor_frames >= 0) {
        if (kind != ORIGINAL_SOUND_CHICAGO) return 2;
        part.dsp.chicago.allocator.gate_duration = probe_anchor_frames;
    }
    for (unsigned i = 0; i < events; ++i) {
        if (scanf("%u%u%u", &ticks[i], &keys[i], &velocities[i]) != 3 ||
            (i && ticks[i] < ticks[i-1]) || ticks[i] >= frames ||
            keys[i] > 127 || velocities[i] > 127) return 2;
    }
    for (unsigned i = 0; i < cuts; ++i) {
        if (scanf("%u", &boundaries[i]) != 1 || !boundaries[i] || boundaries[i] >= frames ||
            (i && boundaries[i] <= boundaries[i-1])) return 2;
    }
    if (kind == ORIGINAL_SOUND_CHICAGO && original_runtime_all_off(&part) != ORIGINAL_RUNTIME_OK) return 1;
    unsigned event = 0, cut = 0;
    for (unsigned cursor = 0; cursor < frames;) {
        if (original_runtime_prepare(&part, 120.f) != ORIGINAL_RUNTIME_OK) return 1;
        while (event < events && ticks[event] == cursor) {
            if (original_runtime_key(&part, ORIGINAL_INPUT_SEQUENCE, keys[event], velocities[event]) != ORIGINAL_RUNTIME_OK)
                return 1;
            ++event;
        }
        if (cut < cuts && boundaries[cut] == cursor) ++cut;
        unsigned stop = cursor + BLOCK;
        if (stop > frames) stop = frames;
        if (event < events && stop > ticks[event]) stop = ticks[event];
        if (cut < cuts && stop > boundaries[cut]) stop = boundaries[cut];
        float left[BLOCK], right[BLOCK], stereo[BLOCK * 2];
        unsigned n = stop - cursor;
        if (original_runtime_render(&part, &workspace, left, right, n) != ORIGINAL_RUNTIME_OK) return 1;
        for (unsigned i = 0; i < n; ++i) {
            stereo[2 * i] = left[i];
            stereo[2 * i + 1] = right[i];
        }
        if (fwrite(stereo, sizeof(float) * 2, n, stdout) != n) return 1;
        cursor = stop;
    }
    return original_runtime_retire(&part) == ORIGINAL_RUNTIME_OK && !fflush(stdout) ? 0 : 1;
}
