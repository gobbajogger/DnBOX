#pragma once
#include "tick_prefix.hpp"
#include "tick_time.hpp"
#include "lfo_apply.hpp"
#include "vibrato4.hpp"
#include "cutoff_executor.hpp"
namespace original_conductor_v1 {
struct OrderedState {float phases[3],held[4],held_elapsed[3];TickTimes times;ApplyState apply;Vibrato4State vibrato;RuntimeCutoff cutoff;};
struct OrderedInput {TickPrefixInput prefix;float delay1,delay3;bool fully_loaded,preset_loaded;};
struct OrderedBackend {Sine sine;EntropyProvider entropy;std::uint64_t entropy_budget;RestartVibrato restart;Providers math;ApplyBackend lfo;CutoffBackend cutoff;VibratoBackend vibrato;};
enum class OrderedStatus {NotLoaded,CompletedEffectsOnly,PrefixFailure,PartialFailure};
enum class OrderedStage {None,Prefix,Time,Lfo1,Lfo2,Lfo3,Cutoff,Vibrato4};
struct OrderedResult {OrderedStatus status;OrderedStage stage;unsigned effects;};
// Narrow cold serialized adapter contract, NOT a general reentrant app backend.
// State owns all timing/cycle fields; apply.armed and apply.held_notes are canonical.
// Stage cycle flags are published directly before their effect callbacks. Callbacks may change LIVE controls and
// cutoff fields through their supplied context, but may not reenter tick/note/
// preset or mutate timing/cycle/held ownership. read_live must reflect actual
// current delay flags and controls, not a prefrozen snapshot. All owner spans
// remain live/disjoint except the explicitly shared cutoff state. Restart may
// only restart its documented receiver group. Real effects must report failure.
// Prefix errors can consume external entropy; later failures preserve partial
// state/effects. Never replay a failed tick as though it rolled back.
OrderedResult execute_ordered_tick(OrderedState&,const OrderedInput&,OrderedBackend) noexcept;
}
