#pragma once
#include "ordered_tick.hpp"
namespace original_conductor_v1 {
struct Lifecycle {OrderedState tick;bool new_note,retrigger[4];double last_beat,beat_accumulator;};
struct BeatUpdate {bool supplied;double finite_nonnegative;};
enum class LifecycleStatus {Unsupported,Noop,CompletedEffectsOnly,Invalid,MissingCapability,PartialFailure};
struct LifecycleResult {LifecycleStatus status;OrderedResult tick;};
// Source-closed finite/nonnegative optional beat subset only. General original
// IEEE/optional payload validation is NOT reproduced. Earlier stage adapter
// ownership/reentrancy limits apply. Full reset preserves delay state and held4;
// retrigger preserves all held values/accumulators and delay state. Both invoke
// actual ordered tick(0), including entropy and effects when loaded.
LifecycleResult full_reset(Lifecycle&,BeatUpdate,const OrderedInput&,OrderedBackend) noexcept;
LifecycleResult selective_retrigger(Lifecycle&,BeatUpdate,const OrderedInput&,OrderedBackend) noexcept;
}
