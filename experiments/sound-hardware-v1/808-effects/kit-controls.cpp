/* SPDX-License-Identifier: GPL-3.0-only */
#include "sc808_shape.h"
#include "sc808_fx.h"
extern "C" {
#include "drum808.h"
}
#include <algorithm>
#include <cmath>
#include <cstdio>

int main()
{
    static drum808_t kit;
    unsigned passed = 0;
    double worst = 0.0;
    for (int mode = 0; mode <= 7; ++mode)
    for (int pot = 0; pot <= 127; ++pot)
    for (int level : {0, 31, 89, 127})
    for (int with_glue = 0; with_glue <= 1; ++with_glue) {
        drum808_init(&kit);
        int comp = with_glue ? pot : 0;
        drum808_set(&kit, D8_KIT, 0, level);
        drum808_set(&kit, D8_KIT, D8_KIT_DIST, mode);
        drum808_set(&kit, D8_KIT, D8_KIT_DRIVE, pot);
        drum808_set(&kit, D8_KIT, D8_KIT_GLUE, comp);
        float lstate[2] = {}, rstate[2] = {};
        sc808_glue_t glue = {};
        double energy = 0, error = 0;
        float drive = 10.f * (pot / 127.f);
        float vol = level / 127.f;
        for (int base = 0; base < 512; base += 32) {
            float l[32], r[32], rv[32], dl[32], want_l[32], want_r[32];
            for (int i = 0; i < 32; ++i) {
                int t = base + i;
                float x = (t > 120 && t < 293) ? 0.f : ((t * 53) % 251 - 125) * 0.019f;
                float y = (t > 120 && t < 293) ? 0.f : ((t * 73) % 249 - 124) * 0.011f;
                /* The lane accumulator normally applies kit.vol before the joining stage. */
                l[i] = x * kit.vol;
                r[i] = y * kit.vol;
                rv[i] = 0.25f * kit.vol;
                dl[i] = -0.5f * kit.vol;
                if (mode) {
                    x = sc808_shape_st(x, drive, mode - 1, lstate);
                    y = sc808_shape_st(y, drive, mode - 1, rstate);
                }
                if (comp) {
                    float peak = std::max(std::fabs(x), std::fabs(y));
                    float detected = sc808_glue_tick(&glue, peak, comp / 127.f, 44100.f);
                    float gain = peak ? detected / peak : 1.f;
                    x *= gain;
                    y *= gain;
                }
                want_l[i] = x * vol;
                want_r[i] = y * vol;
            }
            drum808_kit_process(&kit, l, r, rv, dl, 32);
            for (int i = 0; i < 32; ++i) {
                if (rv[i] != 0.25f * vol || dl[i] != -0.5f * vol) return 1;
                double el = l[i] - want_l[i], er = r[i] - want_r[i];
                energy += (double)want_l[i] * want_l[i] + (double)want_r[i] * want_r[i];
                error += el * el + er * er;
            }
        }
        double ratio = energy ? std::sqrt(error / energy) : std::sqrt(error);
        if (!std::isfinite(ratio) || ratio > 0.001) {
            std::fprintf(stderr, "mode %d drive %d volume %d glue %d relative error %.9g\n",
                         mode, pot, level, comp, ratio);
            return 2;
        }
        worst = std::max(worst, ratio);
        ++passed;
    }
    std::printf("{\"cases\":%u,\"frames_per_case\":512,\"worst_relative_rms\":%.9g,\"limit\":0.001}\n",
                passed, worst);
    return 0;
}
