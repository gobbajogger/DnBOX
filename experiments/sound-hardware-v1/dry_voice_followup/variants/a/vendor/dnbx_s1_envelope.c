/* SPDX-License-Identifier: GPL-3.0-only */
#include "dnbx_s1_envelope.h"
#include <string.h>

_Static_assert(sizeof(dnbx_s1_envelope_state) == 12, "S1 envelope state budget");

static int unit_range(float value)
{
    /* The two comparisons also reject infinities and NaN. */
    return value >= 0.0f && value <= 1.0f;
}

int dnbx_s1_envelope_configure(dnbx_s1_envelope_params *params,
                             float attack_retention, float decay_retention,
                             float sustain, float release_retention)
{
    if (!params || !unit_range(attack_retention) ||
        !unit_range(decay_retention) || !unit_range(sustain) ||
        !unit_range(release_retention))
        return -1;
    *params = (dnbx_s1_envelope_params){attack_retention, decay_retention,
                                      sustain, release_retention};
    return 0;
}

void dnbx_s1_envelope_reset(dnbx_s1_envelope_state *state)
{
    memset(state, 0, sizeof *state);
}

float dnbx_s1_envelope_sample(dnbx_s1_envelope_state *state,
                            const dnbx_s1_envelope_params *params,
                            unsigned gate)
{
    if (gate != state->previous_gate) {
        state->stage = gate ? DNBX_S1_ENVELOPE_ATTACK : DNBX_S1_ENVELOPE_RELEASE;
        state->retention = gate ? params->attack_retention : params->release_retention;
        state->previous_gate = (uint8_t)gate;
    }
    /* This uses the previous output, before the new sample's recurrence. */
    if (state->stage == DNBX_S1_ENVELOPE_ATTACK && state->level >= 0.99f) {
        state->stage = DNBX_S1_ENVELOPE_DECAY;
        state->retention = params->decay_retention;
    }
    float target = state->stage == DNBX_S1_ENVELOPE_ATTACK ? 1.0f :
                   state->stage == DNBX_S1_ENVELOPE_DECAY ? params->sustain : 0.0f;
    float old_term = state->retention * state->level;
    float target_weight = 1.0f - state->retention;
    float target_term = target_weight * target;
    state->level = old_term + target_term;
    return state->level;
}
