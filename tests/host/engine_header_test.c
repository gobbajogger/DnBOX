/* Public value-only engine interface must compile without generated DSP tables. */
#include <stddef.h>
#include "../../firmware/src/app/engine.h"
#define ABI_CHECK(name, expression) typedef char name[(expression) ? 1 : -1]
ABI_CHECK(juno_size, sizeof(juno6_envelope_snapshot)==40);
ABI_CHECK(juno_level, offsetof(juno6_envelope_snapshot,level)==16);
ABI_CHECK(juno_voice, offsetof(juno6_envelope_snapshot,selected_voice)==20);
ABI_CHECK(juno_stage, offsetof(juno6_envelope_snapshot,stage)==24);
ABI_CHECK(drum_size, sizeof(drum909_amp_snapshot)==52);
ABI_CHECK(drum_hold, offsetof(drum909_amp_snapshot,hold_left)==28);
ABI_CHECK(drum_active, offsetof(drum909_amp_snapshot,active)==40);
int (*const juno_snapshot_signature)(int,juno6_envelope_snapshot*)=engine_juno_envelope_view;
int (*const drum_snapshot_signature)(int,drum909_amp_snapshot*)=engine_drum909_amp_view;
