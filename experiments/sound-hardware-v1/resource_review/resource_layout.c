#include <stdint.h>
#include <stddef.h>
#include "firmware/src/app/original_runtime.h"
#include "experiments/original_synth/chicago_fx2_compact.h"
#include "firmware/src/app/chicago_cho.h"
#include "firmware/src/app/chicago_ens.h"
#include "firmware/src/app/chicago_hall.h"
#include "firmware/src/app/chicago_sdly.h"
#include "firmware/src/app/chicago_bpmdly.h"
#include "firmware/src/app/chicago_gaterev.h"
#include "firmware/src/dsp/fxbus.h"
#include "firmware/src/dsp/master.h"
#include "firmware/src/dsp/busfx.h"
#include "firmware/src/dsp/drum909.h"
#include "firmware/src/dsp/drum808.h"
#include "firmware/src/dsp/breaks.h"
#include "firmware/src/app/x0x.h"

/* Compile-only target ABI probe. Values are read from the ELF object, never executed on a target. */
const uint32_t resource_layout[] = {
 sizeof(void*), sizeof(float), sizeof(double),
 sizeof(juno6_voice_t), sizeof(juno6_t), sizeof(juno6_chorus_t),
 sizeof(original_runtime_backend), sizeof(original_runtime_part),
 offsetof(original_runtime_part, keys), sizeof(original_runtime_workspace),
 sizeof(original_chicago_fx2_compact), sizeof(original_chicago_fx2_cold_owner),
 sizeof(fxbus_t), sizeof(master_t), sizeof(drum909_t), sizeof(drum808_t), sizeof(breaks_t),
 sizeof(chi_cho_t), sizeof(chi_ens_t), sizeof(chi_hall_t), sizeof(chi_sdly_t), sizeof(chi_bpmdly_t),
 sizeof(chi_gaterev_t), sizeof(busfx_t), sizeof(busfx_stut_hub_t),
 sizeof(project_t), sizeof(seq_t), sizeof(original_chicago_tuning),
 sizeof(chi_fx2_insert_arena)
};
