#pragma once
#include <cstddef>
#include <cstdint>
namespace original_conductor_v1 {
struct LiveLfo {bool enabled,snapback,delay_finished;unsigned mode,waveform;float rate;float amount;float bases[2];std::uint16_t destinations[2];};
struct ApplyState {bool armed,cycle_complete[3];float current_time[3];std::size_t held_notes;};
struct ResolvedControl {std::uintptr_t token;std::uint16_t parameter,global;};
enum class ResolveStatus {Found,Missing,Failure};
struct ApplyBackend {
 void* context;
 bool(*read_live)(void*,unsigned,LiveLfo&) noexcept;
 ResolveStatus(*resolve_first)(void*,std::uint16_t,ResolvedControl&) noexcept;
 bool(*set_parameter_flag_zero)(void*,ResolvedControl,float) noexcept;
 bool(*set_cutoff_target)(void*,ResolvedControl,float) noexcept;
 bool(*dispatch_global)(void*,std::uint16_t,float) noexcept;
};
enum class ApplyStatus {Unsupported,Noop,AppliedCommandsOnly,Invalid,MissingCapability,ReadFailure,PartialFailure};
struct ApplyResult {ApplyStatus status;unsigned effects;bool cycle_changed;};
// Cold serialized tick apply stage. read_live is side-effect-free but reads ACTUAL
// current controls each time. resolve_first pins immutable identity/lifetime until
// set + global dispatch finish; missing controls are not fabricated. Backend must
// implement real effects and report failure honestly; success is not audio identity.
// Earlier callbacks may change later LFO inputs or the second destination's base.
ApplyResult apply_tick_lfo(unsigned,const float (&wave_row)[6],ApplyState&,ApplyBackend) noexcept;
}
