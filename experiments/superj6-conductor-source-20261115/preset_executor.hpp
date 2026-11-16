#pragma once
#include "conductor.hpp"
namespace original_conductor_v1 {
struct MainBinding {std::uint16_t id;Identifier name;bool derived_default;};
// Borrowed immutable static name view; out-of-range preserves the output.
Status main_binding(std::size_t index,MainBinding&) noexcept;
struct PresetBackend {void* context;unsigned capabilities;bool(*execute)(void*,const Command&) noexcept;};
enum class PresetStatus {Unsupported,CompletedMainPassEffectsOnly,Invalid,MissingCapability,MissingMath,PartialMathFailure,PartialBackendFailure,CompletedPwmPostpassEffectsOnly};
struct PresetResult {PresetStatus status;std::size_t attempted_effects,completed_bindings;std::uint16_t failed_id;};
// Separate STREAMING executor, not the failure-atomic plan_main replacement.
// Caller pins cold serialized immutable records/string spans, pure nonthrowing
// math and actual control/scheduler owner for the entire call. No callback may
// mutate these records, reenter preset/tick/note, or retain the command reference.
// Backend must actually apply/schedule each command (copy it if deferred), with
// original control identity/callback-vs-setter semantics; this is not fake DSP.
// Complete potential capabilities and missing derived-math provider are checked
// before effects. Later math/backend failures retain all attempted effects.
// Main pass skips212/213/214; a separate explicit PWM postpass API is below.
// Tuning/migration/full loader remain absent.
PresetResult execute_main(const Record*,std::size_t,std::uint64_t,bool,Providers,PresetBackend) noexcept;
}

namespace original_conductor_v1 {
// Snapshot AFTER the main pass, in original LFO2dest1/LFO2dest2/LFO3dest1/LFO3dest2
// order. Original captures all four route bytes and base-control identities before
// applying any slot. Caller pins the actual base-control objects184/186/188/190.f
// This API copies route bytes; callbacks cannot alter this captured selection.
struct PwmPostpassSnapshot {std::uint8_t route[4];};
// Only routes78/79 reapply their BASE-control record/default, not the routed
// destination record. Backend retains original callback-versus-setter behavior.
// Same immutable records/nonthrowing/no-reentry/lifetime contract as execute_main.
// Commands are phase-local order0..3. Partial failures are never rolled back.
// No tuning pass, full transaction or presetLoaded success is implied.
PresetResult execute_pwm_postpass(const Record*,std::size_t,std::uint64_t,PwmPostpassSnapshot,Providers,PresetBackend) noexcept;
}
