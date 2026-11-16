#pragma once
namespace original_conductor_v1 {
struct TickTimes {float current[4],delay_elapsed[3];bool delay_finished[3];};
struct RestartVibrato {void* context;bool(*restart_collection_and_noise)(void*) noexcept;};
enum class TimeStatus {Unsupported,Updated,Invalid,MissingCapability,RestartFailedAfterStateChange};
// Cold serialized mutable stage AFTER waveform/held rows and BEFORE apply1/2/3.f
// Numeric/capability preflight precedes mutation. A failing restart callback is an
// explicit partial-effect failure, not rollback or permission to replay the tick.
TimeStatus advance_tick_times(TickTimes&,float dt,float delay1,float delay3,RestartVibrato) noexcept;
}
