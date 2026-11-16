#pragma once
#include "entropy_table.hpp"
#include "lfo_laws.hpp"
namespace original_conductor_v1 {
struct TickPrefixState {float phase[3];float held[4],held_elapsed[3];float time4;bool delay1_finished,delay3_finished;};
struct TickPrefixInput {float rates[3],vibrato_rate_target;unsigned waveforms[3];float dt;};
struct TickPrefixOutput {TickPrefixState state;float rows[4][6];std::uint64_t entropy_requests;float captured_factor4;};
enum class PrefixStatus {Unsupported,PreparedPrefixOnly,Invalid,MissingCapability,MathFailure,EntropyFailure};
struct Sine {float(*function)(float) noexcept;};
// This is the ordered pre-apply prefix only, not a full tick: old delay flags gate
// phases before delay/time updates. No caching/removal of the unused ID12 table.
// Caller owns valid cold-serialized objects and keeps provider-mutated storage
// disjoint from them. Provider consumption/math side effects cannot be rolled back.
PrefixStatus prepare_tick_prefix(const TickPrefixState&,const TickPrefixInput&,Sine,EntropyProvider,std::uint64_t budget,TickPrefixOutput&) noexcept;
}
