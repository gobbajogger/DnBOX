/* SPDX-License-Identifier: GPL-3.0-only */
#define main x0x_host_main
#include "../../host/x0x_host.c"
#undef main
#include <assert.h>
#include <stdio.h>

/* The host executes each scanner/foreground/audio boundary serially. The
 * native build supplies the real all-IRQ exclusion at the Panic cutoff. */
uint32_t plat_early_drum_lock(void) { return 0; }
void plat_early_drum_unlock(uint32_t flags) { (void)flags; }
extern volatile uint32_t early_drum_admitted, early_drum_consumed;

int main(void)
{
    boot();
    run_ms(80); /* let boot's ordinary parameter commands retire */
    const uint32_t a=early_drum_admitted, c=early_drum_consumed;

    engine_project_hold(1);
    assert(!engine_early_drum_put(DR_LT,127));
    engine_project_hold(0);

    engine_set(T_MIX,0,MX_PAN,64);
    assert(!engine_early_drum_put(DR_LT,127)); /* do not overtake CQ */
    run_ms(20);
    host_midi_put(0x007bb00bu); /* existing CC queue must not be overtaken */
    assert(plat_midi_pending());
    assert(!engine_early_drum_put(DR_LT,127));
    run_ms(20);
    seq.part_off|=1u<<TRK_DRUM;
    assert(!engine_early_drum_put(DR_LT,127));
    seq.part_off&=~(1u<<TRK_DRUM);

    assert(engine_early_drum_put(DR_LT,127));
    assert(engine_project_notes()); /* actual engine admission barrier */
    assert(ui_project_begin()==UI_PROJECT_BUSY);
    assert(engine_project_notes()); /* rejected transfer leaves hit intact */
    engine_request_notes_off();
    assert(!engine_project_notes());
    assert(early_drum_consumed==c);

    /* Original audio owner applies Panic, then this fresh physical hit. */
    assert(engine_early_drum_put(DR_MT,119));
    run_ms(20);
    assert(early_drum_admitted==a+2 && early_drum_consumed==c+1);
    run_ms(30);
    assert(early_drum_consumed==c+1);
    assert(!engine_project_notes());
    printf("actual engine: project hold and busy, CQ/MIDI order, part off, Panic cutoff and fresh once-only hit pass\n");
}
