#include <stddef.h>
#include <stdint.h>
#include "dnbx_core.h"
#include "firmware/src/app/original_runtime.h"
#include "experiments/original_synth/chicago_fx2_compact.h"
struct candidate_with_chorus { dnbx_part core; juno6_chorus_t chorus; };
union candidate_backend { original_runtime_backend stock; struct candidate_with_chorus candidate; };
const uint32_t candidate_layout[] = {sizeof(void*),sizeof(float),sizeof(double),sizeof(dnbx_envelope_params),sizeof(dnbx_layer_params),sizeof(dnbx_envelope_state),sizeof(dnbx_layer_voice),sizeof(dnbx_voice),sizeof(dnbx_layer),sizeof(dnbx_part),sizeof(dnbx_work),sizeof(dnbx_voice_snapshot),offsetof(dnbx_part,layer),offsetof(dnbx_part,voice),offsetof(dnbx_part,raw),offsetof(dnbx_part,raw_present),sizeof(original_runtime_backend),sizeof(original_runtime_part),sizeof(original_chicago_fx2_compact),sizeof(juno6_chorus_t),sizeof(struct candidate_with_chorus),sizeof(union candidate_backend),2*sizeof(union candidate_backend)};
