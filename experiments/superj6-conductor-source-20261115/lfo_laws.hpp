#pragma once
#include "conductor.hpp"
namespace original_conductor_v1 {
// Literal mechanics only, not a tick executor. In particular these functions do
// not construct/omit the required ID12 entropy table or implement LFO4.
Status lfo_rate(float normalized,bool hz,double tempo,Providers,float&) noexcept;
Status lfo_phase(double phase,float rate,double dt,double&) noexcept;
Status lfo_index(double phase,std::int64_t&) noexcept;
Status shape_table_sample(unsigned original_table_kind,unsigned index,float&) noexcept;
Status lfo_modulate(float base,double wave,double amount,unsigned waveform,float&) noexcept;
}
