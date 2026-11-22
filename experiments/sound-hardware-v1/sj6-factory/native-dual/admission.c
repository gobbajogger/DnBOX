/* SPDX-License-Identifier: GPL-3.0-only */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "juno6.h"

int main(void)
{
    juno6_t s, saved;
    juno6_envelope_snapshot view;
    juno6_init(&s, 44100);
    assert(!s.typed && !s.age && !juno6_active(&s));
    assert(juno6_note_on(&s, 60, 100));
    assert(juno6_read_envelope(&s, &view) && view.shared_amp_filter && view.selected_voice == 0);
    assert(view.active && view.gate && view.stage == 1 && view.level == 0);
    assert(s.age == 1 && s.voice[0].age == 1 && dnbx_active_voices(&s.core) == 0);
    saved = s;
    assert(!juno6_set(&s, JUNO6_APP_PARAM_BASE + 119, .5f) && !memcmp(&s, &saved, sizeof s));
    juno6_note_off(&s, 60);
    assert(juno6_active(&s) && !juno6_can_edit_amp2(&s));
    assert(juno6_set(&s, JUNO6_APP_PARAM_BASE, 0) && !s.typed && s.master_gain == 0);
    juno6_all_off(&s);
    assert(!juno6_active(&s) && s.age == 1);
    assert(juno6_set(&s, JUNO6_APP_PARAM_BASE + 119, .5f));
    assert(s.typed && s.core.layer[0].params.oscillator_mode == DNBX_OSC_STOCK_BLEP);
    assert(juno6_note_on(&s, 60, 100));
    assert(dnbx_active_voices(&s.core) == 1 && s.age == 1);
    assert(juno6_set(&s, JUNO6_APP_PARAM_BASE + 122, .75f));
    assert(juno6_read_envelope(&s, &view) && !view.shared_amp_filter && view.active);
    juno6_all_off(&s);
    juno6_oscillator_edit wave = {1, .75f, 1};
    juno6_apply_pre_note_wave(&s, 0, &wave);
    assert(s.core.layer[0].params.oscillator_mode == DNBX_OSC_SUPERJ6);
    assert(s.core.layer[1].params.oscillator_mode == DNBX_OSC_STOCK_BLEP);
    juno6_init(&s, 44100);
    assert(!s.typed && !s.age && s.master_gain == 1);
    assert(juno6_read_envelope(&s, &view) && view.shared_amp_filter && view.selected_voice == -1);
    puts("Legacy and typed admission, allocation, snapshot, panic and reset PASS");
}
