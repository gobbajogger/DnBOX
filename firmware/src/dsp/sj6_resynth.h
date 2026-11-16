/* SPDX-License-Identifier: GPL-3.0-only */
/* Parametric resynthesis of a recorded zone. No sample memory.
 *
 * Each partial has its own attack, decay, peak, sustain and phase.
 * A nonzero loop_len means the sustain increment is an integer number of
 * cycles in that many samples, so the loop closes exactly.
 * Noise is white, or one-pole filtered when noise_pole is nonzero.
 * The private 12-record set is not in this tree.
 */
#ifndef SJ6_RESYNTH_H
#define SJ6_RESYNTH_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

enum { SJ6_PARTIALS = 16, SJ6_SINE_N = 256 };

typedef struct Sj6Partial {
    uint32_t inc;
    uint32_t phase0;
    uint16_t attack;
    uint16_t decay;
    int16_t peak;
    int16_t sustain;
} Sj6Partial;

typedef struct Sj6Voice {
    char name[24];
    uint32_t loop_len;
    uint16_t drift_len;
    int16_t drift_depth;
    int16_t noise_peak;
    int16_t noise_sustain;
    uint16_t noise_pole;
    uint32_t noise_seed;
    Sj6Partial partial[SJ6_PARTIALS];
} Sj6Voice;

typedef struct Sj6State {
    uint32_t phase[SJ6_PARTIALS];
    uint32_t drift;
    uint32_t noise;
    int32_t nstate;
    uint32_t frame; /* samples already rendered; a later block continues the envelope */
} Sj6State;

typedef struct Sj6Cost {
    uint64_t muls, adds, shifts, branches;
} Sj6Cost;

void sj6_state_init(Sj6State *s, const Sj6Voice *v);
int32_t sj6_sine_at(uint32_t phase);
void sj6_render(const Sj6Voice *v, Sj6State *s, int32_t *out, int n, Sj6Cost *cost);
void sj6_resynth_keep(void);
unsigned sj6_standin_count(void);
const Sj6Voice *sj6_standin(unsigned index);

#ifdef __cplusplus
}
#endif
#endif
