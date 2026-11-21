/* Independent coefficient preparation experiment; no original engine source.
 * Fixture-qualified only; see REPORT.md for arbitrary-control mismatches.
 * Caller validates a finite positive rate and a finite nonnegative duration;
 * caller retains physical duration controls separately from prepared output.
 */
#ifndef S1_COEFFICIENT_REVIEW_CANDIDATE_H
#define S1_COEFFICIENT_REVIEW_CANDIDATE_H
#include "fastmath.h"
static inline float s1_retention_fast(float seconds, float sample_rate, int attack)
{
    float denominator;
    if (seconds == 0.0f)
        return 0.0f; /* Explicit proposal extension, not fixture-qualified. */
    denominator = (sample_rate * seconds) * (attack ? 0.6f : 1.0f);
    return fm_expf(-1.0f / denominator);
}
#endif
