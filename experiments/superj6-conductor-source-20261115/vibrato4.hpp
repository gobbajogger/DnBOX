#pragma once
#include "conductor.hpp"
namespace original_conductor_v1 {
struct Vibrato4State {bool armed,delay_finished,cycle_complete;double current_time;};
struct LiveVibrato {double delay1,tempo;float rate_target,depth_target;bool tempo_sync;};
struct VibratoBackend {
 void* context;bool(*read_live)(void*,LiveVibrato&) noexcept;
 // Frequency group: Core receivers136,144,152,160. Depth additionally168(noise).
 bool(*apply_frequency)(void*,float normalized,double frequency) noexcept;
 bool(*apply_depth)(void*,float normalized,double depth) noexcept;
};
enum class VibratoStatus {Unsupported,Noop,AppliedCommandsOnly,Invalid,MissingCapability,ReadFailure,PartialFailure};
struct VibratoResult {VibratoStatus status;unsigned effects;bool cycle_changed;};
// captured_factor is Float(3-Float(2*rateTarget)) from BEFORE prior LFO callbacks.
// live targets are read at actual application points. Completion intentionally
// applies frequency twice; do not deduplicate those effects.
VibratoResult apply_vibrato4(float captured_factor,const double (&row4)[6],Vibrato4State&,Providers,VibratoBackend) noexcept;
}
