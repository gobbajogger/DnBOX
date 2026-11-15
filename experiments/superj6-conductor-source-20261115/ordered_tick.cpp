#include "ordered_tick.hpp"
namespace original_conductor_v1 {
OrderedResult execute_ordered_tick(OrderedState& s,const OrderedInput& in,OrderedBackend b)noexcept{
 if(!in.fully_loaded||!in.preset_loaded)return {OrderedStatus::NotLoaded,OrderedStage::None,0};
 TickPrefixState prefix{};for(unsigned i=0;i<3;i++){prefix.phase[i]=s.phases[i];prefix.held_elapsed[i]=s.held_elapsed[i];}for(unsigned i=0;i<4;i++)prefix.held[i]=s.held[i];prefix.time4=s.times.current[3];prefix.delay1_finished=s.times.delay_finished[0];prefix.delay3_finished=s.times.delay_finished[2];
 TickPrefixOutput prepared{};if(prepare_tick_prefix(prefix,in.prefix,b.sine,b.entropy,b.entropy_budget,prepared)!=PrefixStatus::PreparedPrefixOnly)return {OrderedStatus::PrefixFailure,OrderedStage::Prefix,0};
 for(unsigned i=0;i<3;i++){s.phases[i]=prepared.state.phase[i];s.held_elapsed[i]=prepared.state.held_elapsed[i];}for(unsigned i=0;i<4;i++)s.held[i]=prepared.state.held[i];
 if(advance_tick_times(s.times,in.prefix.dt,in.delay1,in.delay3,b.restart)!=TimeStatus::Updated)return {OrderedStatus::PartialFailure,OrderedStage::Time,0};
 unsigned effects=0;
 for(unsigned i=0;i<3;i++){
 for(unsigned j=0;j<3;j++)s.apply.current_time[j]=s.times.current[j];
 ApplyResult r=apply_tick_lfo(i,prepared.rows[i],s.apply,b.lfo);effects+=r.effects;
 if(r.status!=ApplyStatus::Noop&&r.status!=ApplyStatus::AppliedCommandsOnly)return {OrderedStatus::PartialFailure,static_cast<OrderedStage>(unsigned(OrderedStage::Lfo1)+i),effects};
 }
 CutoffResult c=execute_cutoff(s.cutoff,in.prefix.dt,b.math,b.cutoff);effects+=c.effects;if(c.status!=CutoffStatus::Noop&&c.status!=CutoffStatus::CompletedEffectsOnly)return {OrderedStatus::PartialFailure,OrderedStage::Cutoff,effects};
 s.vibrato.armed=s.apply.armed;s.vibrato.delay_finished=s.times.delay_finished[0];s.vibrato.current_time=s.times.current[3];VibratoResult r=apply_vibrato4(prepared.captured_factor4,prepared.rows[3],s.vibrato,b.math,b.vibrato);effects+=r.effects;
 if(r.status!=VibratoStatus::Noop&&r.status!=VibratoStatus::AppliedCommandsOnly)return {OrderedStatus::PartialFailure,OrderedStage::Vibrato4,effects};
 return {OrderedStatus::CompletedEffectsOnly,OrderedStage::Vibrato4,effects};
}
}
