/* SPDX-License-Identifier: GPL-3.0-only */
#include "s1_sub_osc.h"
#include "soundpipe.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>

int main(void)
{
    enum { FRAMES = 88200 };
    static float table[4097];
    sp_data sp = {0};
    sp.sr = 44100;
    sp_ftbl ft = {0};
    ft.tbl = table;
    sp_ftbl_init(&sp, &ft, 4096);
    sp_gen_sine(&sp, &ft);
    const int notes[] = {0, 24, 48, 54, 60, 69, 84, 96, 127};
    const unsigned velocities[] = {0u, 36u, 93u, 127u};
    const uint32_t starts[] = {0u, 1u, 0x7fffffu, 0x800000u, 0x800001u, 0xffffffu};
    const float levels[] = {0.f, .25f, 1.f};
    unsigned frames = 0, square_mismatches = 0, phase_mismatches = 0, edge_cases = 0;
    double sine_abs_error = 0., total_error = 0., total_reference = 0.;
    for (unsigned n=0; n<sizeof notes/sizeof notes[0]; ++n)
    for (unsigned o=0; o<2; ++o)
    for (unsigned v=0; v<sizeof velocities/sizeof velocities[0]; ++v)
    for (unsigned depth=0; depth<sizeof levels/sizeof levels[0]; ++depth)
    for (unsigned sq=0; sq<2; ++sq) {
        s1_sub_osc candidate = {0};
        sp_osc reference = {0};
        sp_osc_init(&sp, &reference, &ft, 0.f);
        const float hz = (float)(440. * exp2((notes[n] - 69) / 12.));
        const float ratio = .9375f + (float)(n % 4u) * .125f;
        s1_sub_pitch(&candidate, hz, ratio, o);
        reference.freq = hz * (ratio / (2.f * (1.f + (float)o)));
        if (reference.freq > 22050.f) reference.freq = 22050.f;
        const float velocity = (float)velocities[v] / 127.f;
        reference.amp = velocity * velocity;
        for (unsigned i=0; i<FRAMES; ++i) {
            if (i < sizeof starts/sizeof starts[0]) {
                reference.lphs = candidate.phase = starts[i];
                edge_cases++;
            }
            float expected;
            sp_osc_compute(&sp, &reference, NULL, &expected);
            expected = sq ? (expected > 0.f ? levels[depth] : -levels[depth]) :
                            expected * (3.f * levels[depth]);
            const float observed = s1_sub_sample(&candidate, reference.amp, levels[depth], sq);
            assert(isfinite(observed));
            phase_mismatches += candidate.phase != (uint32_t)reference.lphs ||
                                candidate.step != (uint32_t)reference.inc;
            if (sq) square_mismatches += expected != observed;
            else {
                const double error = (double)observed - expected;
                if (fabs(error) > sine_abs_error) sine_abs_error = fabs(error);
                total_error += error * error;
                total_reference += (double)expected * expected;
            }
            ++frames;
        }
    }
    const double relative_rms = sqrt(total_error / total_reference);
    printf("{\"frames\":%u,\"forced_phase_edges\":%u,\"phase_mismatches\":%u,"
           "\"square_mismatches\":%u,\"sine_max_abs_error\":%.12g,"
           "\"sine_relative_rms_error\":%.12g,\"state_bytes_per_voice\":%zu}\n",
           frames,edge_cases,phase_mismatches,square_mismatches,sine_abs_error,relative_rms,
           sizeof(s1_sub_osc));
    return phase_mismatches || square_mismatches || sine_abs_error > 0.00001 ||
           relative_rms > 0.000001;
}
