/* Comparison-only. Link to the original Soundpipe delay in the factory fixture. */
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "soundpipe.h"

enum { rate = 44100, long_tap = 5 * rate, owners = 2 };
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "failed at line %d: %s\n", __LINE__, #x); exit(1); } } while (0)

static uint32_t advance(uint32_t *state) {
    uint32_t x = *state;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    return *state = x;
}
static void run(int warm_sweep) {
    sp_data sp = {0};
    sp.sr = rate;
    sp_vdelay *delay[owners];
    int16_t *words[owners];
    uint32_t seed[owners] = {1370, 145744};
    unsigned differences = 0, matched = 0;
    for (unsigned p = 0; p < owners; ++p) {
        CHECK(sp_vdelay_create(&delay[p]) == SP_OK);
        CHECK(sp_vdelay_init(&sp, delay[p], 10) == SP_OK);
        CHECK(delay[p]->buf.size == (10 * rate + 1) * sizeof(float));
        delay[p]->feedback = 0;
        words[p] = malloc(long_tap * sizeof(*words[p]));
        CHECK(words[p]);
    }
    for (unsigned frame = 0; frame < 2 * long_tap; ++frame) {
        for (unsigned p = 0; p < owners; ++p) {
            float input = 0, output;
            if (frame < long_tap) {
                int n = (int)(advance(&seed[p]) >> 16) - 32768;
                words[p][frame] = (int16_t)n;
                input = (float)n / 32768.f;
            }
            /* The left ping-pong branch reads twice the public base time.
             * At mix zero its ring is still advanced by the original graph. */
            delay[p]->del = warm_sweep && frame < long_tap ? 2.f * .00036281178472563624f : 5.f;
            CHECK(sp_vdelay_compute(&sp, delay[p], &input, &output) == SP_OK);
            if (frame >= long_tap) {
                float expected = (float)words[p][frame-long_tap] / 32768.f;
                CHECK(output == expected);
                ++matched;
            } else if (!warm_sweep) {
                CHECK(output == 0);
            }
        }
    }
    for (unsigned i = 0; i < long_tap; ++i) if (words[0][i] != words[1][i]) ++differences;
    CHECK(differences > long_tap - 30);
    printf("%s,%u,%u,%u\n",warm_sweep ? "warm_short_to_long" : "fixed_long",matched,differences,long_tap);
    for (unsigned p = 0; p < owners; ++p) {
        free(words[p]);
        CHECK(sp_vdelay_destroy(&delay[p]) == SP_OK);
    }
}
int main(void) {
    puts("scenario,exact_delayed_q15_words,unequal_owner_inputs,delay_frames");
    run(0); run(1);
}
