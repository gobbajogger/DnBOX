#pragma once
#include "conductor.hpp"
namespace original_conductor_v1 {
// Literal mechanics only, not a tick executor. In particular these functions do
// not construct/omit the required ID12 entropy table or implement LFO4.
Status lfo_rate(float normalized,bool hz,float tempo,Providers,float&) noexcept;
Status lfo_phase(float phase,float rate,float dt,float&) noexcept;
Status lfo_index(float phase,std::int64_t&) noexcept;
Status shape_table_sample(unsigned original_table_kind,unsigned index,float&) noexcept;
Status lfo_modulate(float base,float wave,float amount,unsigned waveform,float&) noexcept;
}
