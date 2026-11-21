/* SPDX-License-Identifier: GPL-3.0-only */
/* Uncompiled C-ready proposal, NOT an implemented or audio-qualified candidate.
 * Apply only in a separately frozen experiment after the owner selects it.
 * Requires the unchanged B chicago_nonlinear() and the following two new floats
 * in s1_dry_voice, zeroed by the existing whole-object reset:
 *
 *     float feedback_lag3, feedback_lag4;
 *
 * Before a substep:
 *   filter_history[3] = y[n-1] (also fourth forward-pole state)
 *   filter_history[4] = y[n-2] (B's existing extra feedback history)
 *   feedback_lag3    = y[n-3]
 *   feedback_lag4    = y[n-4]
 *
 * Hence four output histories require only two extra floats over B. Expected
 * voice size is 160 bytes rather than 152; that layout has NOT been compiled.
 * Retain B's coefficient preparation, physical controls, two held-input
 * substeps, output averaging, oscillator/envelopes, and gain without changes.
 * Use ordinary float operations, f-suffixed constants, -ffp-contract=off, and
 * no fast-math. No libm, double, heap, or mutable globals are introduced.
 */

static float centered_feedback_substep(s1_dry_voice *voice, float input)
{
    float *history = voice->filter_history;
    float A = voice->prepared.pole_retention;
    float R = voice->prepared.controls.resonance * 0.8f;

    if (R == 0.0f) {
        /* The original B audio arithmetic and original five history writes
         * are copied exactly. Two additional lag assignments cannot affect
         * this branch's audio result, including signed-zero behavior.
         */
        float tmp = history[3];
        float nonlinear = chicago_nonlinear(history[4]);
        float v = input - R * nonlinear;
        float z1 = v + A * (history[0] - v);
        float z2 = z1 + A * (history[1] - z1);
        float z3 = z2 + A * (history[2] - z2);
        float z4 = z3 + A * (history[3] - z3);
        voice->feedback_lag4 = voice->feedback_lag3;
        voice->feedback_lag3 = history[4];
        history[0] = z1; history[1] = z2;
        history[2] = z3; history[3] = z4; history[4] = tmp;
        return z4;
    }

    /* q is the current output of the identical forward cascade with feedback
     * set to zero, calculated from old states without committing any state.
     */
    float q1 = input + A * (history[0] - input);
    float q2 = q1 + A * (history[1] - q1);
    float q3 = q2 + A * (history[2] - q2);
    float q = q3 + A * (history[3] - q3);
    float b = 1.0f - A;
    float b2 = b * b;
    float b4 = b2 * b2;

    /* Current y carries weight 1/16. The four old y values carry 4,6,4,1.
     * In real arithmetic y = q - R*b4*phi(d), hence d + c*phi(d) = w.
     */
    float old_weighted = 4.0f * history[3];
    old_weighted = old_weighted + 6.0f * history[4];
    old_weighted = old_weighted + 4.0f * voice->feedback_lag3;
    old_weighted = old_weighted + voice->feedback_lag4;
    float w = 0.0625f * (q + old_weighted);
    float c = (0.0625f * R) * b4;
    float d = w;
    for (unsigned i = 0; i < 6u; ++i)
        d = w - c * chicago_nonlinear(d);

    /* Commit the same stock forward recurrence once, using the solved feedback.
     * New lags must shift BEFORE the existing h[3]/h[4] writes.
     */
    float tmp = history[3];
    float nonlinear = chicago_nonlinear(d);
    float v = input - R * nonlinear;
    float z1 = v + A * (history[0] - v);
    float z2 = z1 + A * (history[1] - z1);
    float z3 = z2 + A * (history[2] - z2);
    float z4 = z3 + A * (history[3] - z3);
    voice->feedback_lag4 = voice->feedback_lag3;
    voice->feedback_lag3 = history[4];
    history[0] = z1; history[1] = z2;
    history[2] = z3; history[3] = z4; history[4] = tmp;
    return z4;
}
