#include <stddef.h>
#include <stdint.h>
#include "app/original_runtime.h"
#include "seq/sequencer.h"
__attribute__((used, section(".rodata.layout"))) const uint32_t layout[] = {
    sizeof(original_runtime_part), offsetof(original_runtime_part, kind),
    offsetof(original_runtime_part, status),
    offsetof(original_runtime_part, dsp.synthone.patch.filter_type),
    offsetof(original_runtime_part, dsp.synthone.patch.sub_level),
    offsetof(original_runtime_part, dsp.synthone.voice[0].prepared.filter_type),
    sizeof(s1_dry_voice), S1_CORE_VOICES,
    offsetof(original_runtime_part, dsp.juno.synth.typed),
    offsetof(original_runtime_part, dsp.juno.synth.stock_preset),
    offsetof(original_runtime_part, dsp.juno.chorus.factory.mode),
    offsetof(original_runtime_part, dsp.juno.chorus.factory.follow_panel),
    offsetof(original_runtime_part, dsp.juno.chorus.factory.mix),
    offsetof(original_runtime_part, dsp.juno.chorus.factory.phase),
    offsetof(seq_t, playing), sizeof(seq_t),
};
