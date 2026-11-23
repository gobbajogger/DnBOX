/* SPDX-License-Identifier: GPL-3.0-only */
/* Public-API regression for the documented live envelope-update policy. */
#include "dnbx_core.h"
#include <assert.h>
#include <stdio.h>

static void advance(dnbx_part *part, unsigned frames)
{
    float left[1024], right[1024];
    while (frames) {
        unsigned take = frames > 1024 ? 1024 : frames;
        assert(dnbx_render(part, left, right, take, NULL) == DNBX_OK);
        frames -= take;
    }
}
static dnbx_voice_snapshot view(const dnbx_part *part)
{
    dnbx_voice_snapshot result;
    assert(dnbx_read_voice(part, 0, &result) == DNBX_OK);
    return result;
}
int main(void)
{
    dnbx_part unchanged, changed;
    dnbx_layer_params initial, update, observed;
    assert(dnbx_init(&unchanged, 48000) == DNBX_OK);
    assert(dnbx_init(&changed, 48000) == DNBX_OK);
    dnbx_default_layer_params(&initial);
    initial.amp = (dnbx_envelope_params){.01f, 0, .01f, .2f, 0, .05f};
    initial.filter = (dnbx_envelope_params){0, 0, 0, 1, 0, .01f};
    assert(dnbx_set_layer(&unchanged, 0, &initial) == DNBX_OK);
    assert(dnbx_set_layer(&changed, 0, &initial) == DNBX_OK);
    assert(dnbx_note_on(&unchanged, 60, 100) == 0);
    assert(dnbx_note_on(&changed, 60, 100) == 0);
    advance(&unchanged, 100);
    advance(&changed, 100);
    update = initial;
    update.amp.attack_s = .1f;
    update.amp.decay_s = .02f;
    update.amp.sustain = .6f;
    assert(dnbx_set_layer(&changed, 0, &update) == DNBX_OK);
    assert(dnbx_get_layer(&changed, 0, &observed) == DNBX_OK);
    assert(observed.amp.attack_s == .1f);
    /* Existing attack retains its prepared contour, including exact endpoint. */
    for (unsigned i = 100; i < 480; ++i) {
        advance(&unchanged, 1);
        advance(&changed, 1);
        assert(view(&changed).amp[0] == view(&unchanged).amp[0]);
    }
    assert(view(&changed).amp[0] == 1);
    advance(&unchanged, 16);
    advance(&changed, 16);
    advance(&unchanged, 480);
    advance(&changed, 480);
    assert(view(&unchanged).amp[0] == .2f);
    assert(view(&changed).amp[0] > .6f && view(&changed).amp[0] < 1);
    /* Next decay adopted the newly configured duration and sustain level. */
    advance(&changed, 481);
    assert(view(&changed).amp_stage[0] == DNBX_ENV_SUSTAIN);
    assert(view(&changed).amp[0] == .6f);
    update.amp.sustain = .3f;
    assert(dnbx_set_layer(&changed, 0, &update) == DNBX_OK);
    assert(view(&changed).amp[0] == .3f);
    /* A subsequently running release also keeps its prepared duration. */
    assert(dnbx_note_off(&changed, 60) == DNBX_OK);
    advance(&changed, 100);
    update.amp.release_s = .2f;
    assert(dnbx_set_layer(&changed, 0, &update) == DNBX_OK);
    advance(&changed, 2300);
    assert(view(&changed).amp[0] == 0);
    advance(&changed, 1);
    assert(view(&changed).amp_stage[0] == DNBX_ENV_IDLE);
    puts("PASS live controls: cached attack/release, next decay, immediate sustain");
    return 0;
}
