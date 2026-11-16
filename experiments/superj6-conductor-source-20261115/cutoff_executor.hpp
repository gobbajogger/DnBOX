#pragma once
#include "conductor.hpp"
namespace original_conductor_v1 {
struct RuntimeCutoff {float current,target;bool active;};
struct CutoffBackend {void* context;bool(*apply_core)(void*,float) noexcept;bool(*post_modulation)(void*,unsigned,float) noexcept;bool(*read_core)(void*,float&) noexcept;};
enum class CutoffStatus {Unsupported,Noop,CompletedEffectsOnly,Invalid,MissingCapability,MathFailure,PartialFailure};
struct CutoffResult {CutoffStatus status;unsigned effects;};
// Actual ordered state/effect stage, unlike the separate command planner. Current
// is published before effects and re-read AFTER callbacks; captured target is kept
// for terminal comparison. Partial external effects are never rolled back.
CutoffResult execute_cutoff(RuntimeCutoff&,float dt,Providers,CutoffBackend) noexcept;
}
