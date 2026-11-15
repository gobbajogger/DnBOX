#pragma once
namespace original_conductor_v1 {
struct TickTimes {double current[4],delay_elapsed[3];bool delay_finished[3];};
struct RestartVibrato {void* context;bool(*restart_collection_and_noise)(void*) noexcept;};
enum class TimeStatus {Unsupported,Updated,Invalid,MissingCapability,RestartFailedAfterStateChange};
// Cold serialized mutable stage AFTER waveform/held rows and BEFORE apply1/2/3.
// Numeric/capability preflight precedes mutation. A failing restart callback is an
// explicit partial-effect failure, not rollback or permission to replay the tick.
TimeStatus advance_tick_times(TickTimes&,double dt,double delay1,double delay3,RestartVibrato) noexcept;
}
