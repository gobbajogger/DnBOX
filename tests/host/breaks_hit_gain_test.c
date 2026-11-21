/* SPDX-License-Identifier: GPL-3.0-only */
/* Verify strike ownership in audio at the public BREAK entry points. */
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#define X0X_IMA_TABLES_DEFINE
#include "../../firmware/src/dsp/breaks.c"

enum { FRAMES = 256 };
static uint8_t data[4096], alternate[4096];
static brk_loop_t loop = {data, 8192, 44100, 1, 8};
static brk_loop_t preview = {alternate, 8192, 44100, 1, 1};
static breaks_t reference, subject, old_only, new_only;
static unsigned checked_frames;

static void reset(breaks_t *b)
{
    breaks_init(b);
    breaks_set_loop(b, 0, &loop);
    breaks_set(b, BRK_LEVEL, 127);
    breaks_set(b, BRK_COMPLEX, 0);
}

static void render_scaled(int frames, unsigned velocity)
{
    float expected[FRAMES], actual[FRAMES];
    unsigned audible = 0;
    while (frames) {
        int n = frames < FRAMES ? frames : FRAMES;
        breaks_render(&reference, expected, n);
        breaks_render(&subject, actual, n);
        for (int i = 0; i < n; i++) {
            float target = expected[i] * ((float)velocity / 127.0f);
            assert(fabsf(actual[i] - target) < 5e-7f);
            if (velocity == 127) assert(memcmp(actual + i, expected + i, sizeof(float)) == 0);
            audible += fabsf(expected[i]) > 0.001f;
        }
        checked_frames += (unsigned)n;
        frames -= n;
    }
    assert(audible);
}

static void start_pair(unsigned velocity, int pinned)
{
    reset(&reference);
    reset(&subject);
    if (pinned) {
        breaks_pin(&reference, 2);
        breaks_pin(&subject, 2);
    }
    breaks_step(&reference, 0, 0, 256, 1);
    breaks_step_velocity(&subject, 0, 0, 256, 1, (uint8_t)velocity);
}

static void crossfade(void)
{
    float a[FRAMES], b[FRAMES], actual[FRAMES];
    int overlap = 0;
    breaks_t *parts[] = {&old_only, &new_only, &subject};
    const uint8_t first[] = {127, 0, 31}, second[] = {0, 127, 96};
    for (int k = 0; k < 3; k++) {
        reset(parts[k]);
        breaks_step_velocity(parts[k], 0, 0, 256, 1, first[k]);
        breaks_render(parts[k], actual, FRAMES);
        breaks_pin(parts[k], 3);
        breaks_step_velocity(parts[k], 1, 0, 256, 1, second[k]);
    }
    breaks_render(&old_only, a, FRAMES);
    breaks_render(&new_only, b, FRAMES);
    breaks_render(&subject, actual, FRAMES);
    for (int i = 0; i < FRAMES; i++) {
        float target = a[i] * (31.0f / 127.0f) + b[i] * (96.0f / 127.0f);
        assert(fabsf(actual[i] - target) < 5e-7f);
        overlap += fabsf(a[i]) > 0.001f && fabsf(b[i]) > 0.001f;
    }
    assert(overlap > BRK_FADE / 2);
    checked_frames += FRAMES;
}

static void internal_restarts(void)
{
    unsigned v = 43;
    start_pair(v, 0);
    render_scaled(128, v);
    breaks_live(&reference, 8, 1);
    breaks_live(&subject, 8, 1);
    render_scaled(128, v);
    assert(subject.v[subject.cur].rev);
    /* No scheduled step intervenes during the real 90 ms grain interval. */
    breaks_live(&reference, 9, 1);
    breaks_live(&subject, 9, 1);
    unsigned before = subject.cur;
    render_scaled(BRK_GRAIN_HOP + 200, v);
    assert(subject.cur != before);

    start_pair(v, 0);
    breaks_set(&reference, BRK_ALEN, 3);
    breaks_set(&subject, BRK_ALEN, 3);
    breaks_live(&reference, 10, 1);
    breaks_live(&subject, 10, 1);
    breaks_step(&reference, 4, 0, 256, 1);
    breaks_step_velocity(&subject, 4, 0, 256, 1, (uint8_t)v);
    render_scaled(128, v);
    assert(subject.sub_div == 4);
    /* The next 16th has a different velocity but no fresh generator hit. */
    breaks_step(&reference, 5, 0, 256, 1);
    breaks_step_velocity(&subject, 5, 0, 256, 1, 0);
    render_scaled(800, v);
    assert(subject.sub_count == 3);

    start_pair(v, 0);
    breaks_set(&reference, BRK_ALEN, 0);
    breaks_set(&subject, BRK_ALEN, 0);
    breaks_step(&reference, 1, 0, 256, 1);
    breaks_step_velocity(&subject, 1, 0, 256, 1, (uint8_t)v);
    unsigned triggers = subject.st_trigs;
    render_scaled(200, v);
    assert(subject.st_trigs == triggers + 1);  /* scheduled 32nd hit */
}

static void live_strikes(int use_preview)
{
    float drop[BRK_FADE + 1];
    start_pair(0, 0);
    render_scaled(128, 0);
    if (use_preview) {
        assert(breaks_preview_begin(&reference));
        assert(breaks_preview_begin(&subject));
        breaks_set_loop(&reference, 0, &preview);
        breaks_set_loop(&subject, 0, &preview);
        assert(breaks_bank(&subject, 0)->adpcm == alternate);
        breaks_slice_on(&reference, 0);
        breaks_slice_on(&subject, 0);
    } else {
        breaks_live(&reference, 2, 1);
        breaks_live(&subject, 2, 1);
    }
    breaks_render(&reference, drop, BRK_FADE + 1);
    breaks_render(&subject, drop, BRK_FADE + 1);
    render_scaled(128, 127);
    /* Held slice wins over the sequencer, including an explicit zero. */
    breaks_step(&reference, 2, 0, 256, 1);
    breaks_step_velocity(&subject, 2, 0, 256, 1, 0);
    render_scaled(128, 127);
    breaks_slice_on(&reference, use_preview ? 0 : 2);
    breaks_slice_on(&subject, use_preview ? 0 : 2);
    render_scaled(128, 127);
}

int main(void)
{
    for (unsigned i = 0; i < sizeof(data); i++) {
        data[i] = (uint8_t)(i * 17u + (i >> 2));
        alternate[i] = (uint8_t)(i * 29u + (i >> 3));
    }
    for (unsigned v = 0; v <= 127; v++) {
        for (int pin = 0; pin <= 1; pin++) {
            start_pair(v, pin);
            render_scaled(256, v);
        }
    }
    crossfade();
    internal_restarts();
    live_strikes(0);
    live_strikes(1);
    printf("BREAK per-hit gain PASS: %u PCM frames, 128 velocities, generator and pin, "
           "two overlapping tails, reverse/grain/stutter/32nd, live and preview\n", checked_frames);
    return 0;
}
