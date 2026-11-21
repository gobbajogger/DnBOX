/* SPDX-License-Identifier: GPL-3.0-only */
#ifndef DNBX_S1_ENVELOPE_H
#define DNBX_S1_ENVELOPE_H

#include <stdint.h>

/* Independently derived from recorded S1 scalar observations. This component
 * does not import an original engine or change the SJ6 AHDSHR. It is not yet
 * connected to the production Synth One renderer.
 *
 * Coefficients are prepared outside this component; they are NOT seconds.
 * A stage latches its retention on entry. Sustain is a live target. Keep the
 * physical controls separately. Target coefficient math and whole-instrument
 * fidelity require their own qualification.
 */
typedef struct {
    float attack_retention, decay_retention, sustain, release_retention;
} dnbx_s1_envelope_params;

typedef struct {
    float level, retention;
    uint8_t stage, previous_gate, reserved[2];
} dnbx_s1_envelope_state;

enum {
    DNBX_S1_ENVELOPE_IDLE = 0,
    DNBX_S1_ENVELOPE_ATTACK = 1,
    DNBX_S1_ENVELOPE_DECAY = 2,
    DNBX_S1_ENVELOPE_RELEASE = 3
};

/* Accept finite coefficients/target in [0,1]. On failure return -1 without
 * changing *params. A null params pointer is rejected. Valid values return 0.
 * Endpoint coefficients describe the recurrence, not additional evidence for
 * an original physical-control endpoint or voice-retirement policy. */
int dnbx_s1_envelope_configure(dnbx_s1_envelope_params *params,
                             float attack_retention, float decay_retention,
                             float sustain, float release_retention);

/* Caller owns independent state per amplitude/filter envelope and note.
 * Reset initializes it to zero and is required before first use.
 * State must be nonnull. */
void dnbx_s1_envelope_reset(dnbx_s1_envelope_state *state);

/* One sample, with edits/events applied before the call. Preconditions: state
 * was reset, params was configured successfully, gate is 0 or 1, pointers are
 * valid/disjoint and this state has one writer. No allocation or transcendental
 * work. Compile without fast math or FMA contraction. Release tails are not
 * truncated; this API does not declare a voice silent/retired. */
float dnbx_s1_envelope_sample(dnbx_s1_envelope_state *state,
                            const dnbx_s1_envelope_params *params,
                            unsigned gate);

#endif
