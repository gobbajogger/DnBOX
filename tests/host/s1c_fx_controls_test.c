/* SPDX-License-Identifier: GPL-3.0-only */
/* Synth One compact FX: every effect control moves the output the right way (behaviour, not an
 * exact match). The INIT program plays one note (0.3 s) and its tail through the device part;
 * each control is rendered at a low and a high setting and a direction metric is compared. */
#include "app/s1c_part.h"
#include "dsp/s1c/s1c_front.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

enum { SR = 44100, N = SR * 2, NOTE = 128 * 103 };   /* 0.3 s, on a block edge */
static s1_dry_part part;
static _Alignas(8) uint8_t arena[S1C_ARENA_BYTES];
static float L[N], R[N];

static void render(unsigned id, float value, unsigned id2, float value2)
{
    s1_part_init(&part);
    s1_part_bind(&part, arena);
    s1c *e = &part.e;
    static const unsigned off[] = {S1C_P_DLON, S1C_P_RVON, S1C_P_PHMIX, S1C_P_PANA, S1C_P_WIDEN};
    for (unsigned i = 0; i < sizeof off / sizeof off[0]; ++i) s1c_front_set(e, off[i], 0.0f);
    s1c_front_set(e, S1C_P_BCSR, 48000.0f);
    s1c_front_set(e, 29 /* amp release */, s1c_front_clamp(29, 0.0f));
    if (id2) s1c_front_set(e, id2, value2);
    if (id2 == S1C_P_RVON) s1c_front_set(e, S1C_P_RVMIX, 0.5f);   /* audible reverb for size and lo-cut */
    s1c_front_set(e, id, value);
    s1_part_note(&part, 60, 100);
    for (unsigned at = 0; at < N; at += 128) {
        if (at == NOTE) s1_part_note(&part, 60, 0);
        s1_part_render(&part, L + at, R + at, 128);
    }
}
static double energy(unsigned a, unsigned b)
{ double e = 0; for (unsigned i = a; i < b; ++i) e += (double)L[i] * L[i] + (double)R[i] * R[i]; return e / (b - a) + 1e-30; }
static double tail(void) { return energy(NOTE + SR / 5, N); }        /* after the dry release */
static double side(void) { double e = 0; for (unsigned i = 0; i < N; ++i) { double d = L[i] - R[i]; e += d * d; } return e / N + 1e-30; }
static double panvar(void)
{   double m = 0, v = 0; unsigned n = 0;
    for (unsigned b = 0; b + 512 <= NOTE; b += 512, ++n) { double l = 0, r = 0;
        for (unsigned i = b; i < b + 512; ++i) { l += fabs(L[i]); r += fabs(R[i]); }
        double x = (l - r) / (l + r + 1e-30); m += x; v += x * x; }
    m /= n; return v / n - m * m + 1e-30; }
static unsigned first_echo(void)   /* first sample after the dry note where the output rises again */
{   double pk = 0; for (unsigned i = 0; i < NOTE; ++i) if (fabs(L[i]) > pk) pk = fabs(L[i]);
    for (unsigned i = NOTE + SR / 50; i < N; ++i) if (fabs(L[i]) > 0.2 * pk) return i;
    return N; }

static double diff_from(const float *ref)
{ double e = 0; for (unsigned i = 0; i < N; ++i) { double d = L[i] - ref[i]; e += d * d; } return e / N + 1e-30; }

static int fails;
static void check(const char *name, double lo, double hi, int up)
{
    int ok = up ? hi > lo * 1.05 : hi < lo * 0.95;
    printf("s1c fx control %-16s low %.3g high %.3g (%s) %s\n", name, lo, hi, up ? "up" : "down", ok ? "ok" : "FAIL");
    fails += !ok;
}

int main(void)
{
    static float ref[N];
    double a, b;
    render(S1C_P_RVMIX, 0.1f, S1C_P_RVON, 1.0f); a = tail(); render(S1C_P_RVMIX, 0.8f, S1C_P_RVON, 1.0f); b = tail(); check("reverb mix", a, b, 1);
    render(S1C_P_RVFB, 0.5f, S1C_P_RVON, 1.0f); a = tail(); render(S1C_P_RVFB, 0.95f, S1C_P_RVON, 1.0f); b = tail(); check("reverb size", a, b, 1);
    render(S1C_P_RVHP, 80.0f, S1C_P_RVON, 1.0f); a = tail(); render(S1C_P_RVHP, 2000.0f, S1C_P_RVON, 1.0f); b = tail(); check("reverb lo-cut", a, b, 0);
    render(S1C_P_DLMIX, 0.1f, S1C_P_DLON, 1.0f); a = tail(); render(S1C_P_DLMIX, 0.8f, S1C_P_DLON, 1.0f); b = tail(); check("delay mix", a, b, 1);
    render(S1C_P_DLFB, 0.1f, S1C_P_DLON, 1.0f); a = tail(); render(S1C_P_DLFB, 0.8f, S1C_P_DLON, 1.0f); b = tail(); check("delay feedback", a, b, 1);
    render(S1C_P_DLT, 0.1f, S1C_P_DLON, 1.0f); a = first_echo(); render(S1C_P_DLT, 0.4f, S1C_P_DLON, 1.0f); b = first_echo(); check("delay time", a, b, 1);
    render(S1C_P_PHMIX, 0.0f, 0, 0); memcpy(ref, L, sizeof ref);
    render(S1C_P_PHMIX, 0.3f, 0, 0); a = diff_from(ref); render(S1C_P_PHMIX, 1.0f, 0, 0); b = diff_from(ref); check("phaser mix", a, b, 1);
    render(S1C_P_BCSR, 48000.0f, 0, 0); memcpy(ref, L, sizeof ref);
    render(S1C_P_BCSR, 16000.0f, 0, 0); a = diff_from(ref); render(S1C_P_BCSR, 3000.0f, 0, 0); b = diff_from(ref); check("crush (clock down)", a, b, 1);
    render(S1C_P_WIDEN, 0.1f, 0, 0); a = side(); render(S1C_P_WIDEN, 1.0f, 0, 0); b = side(); check("widen", a, b, 1);
    render(S1C_P_PANA, 0.1f, S1C_P_PANF, 4.0f); a = panvar(); render(S1C_P_PANA, 1.0f, S1C_P_PANF, 4.0f); b = panvar(); check("auto-pan amount", a, b, 1);
    render(S1C_P_MVOL, 0.3f, 0, 0); a = energy(0, NOTE); render(S1C_P_MVOL, 1.2f, 0, 0); b = energy(0, NOTE); check("master volume", a, b, 1);
    printf("s1c fx controls: %d failed\n", fails);
    return fails != 0;
}
