// TEST ONLY: exact R7 prefix body, sole symbol rename; source SHA256 600ece040439a998de516cc9cd1650dbc3c393dca13ee04a16bdc125ffe5f1c6
#include "tick_prefix.hpp"
#include <cmath>
#include <cstring>
namespace original_conductor_v1 {
PrefixStatus prepare_tick_prefix_reference(const TickPrefixState& state,const TickPrefixInput& input,Sine sine,EntropyProvider entropy,std::uint64_t budget,TickPrefixOutput& output)noexcept{
 if(!std::isfinite(input.dt)||input.dt<0||!std::isfinite(input.vibrato_rate_target)||input.vibrato_rate_target<0||input.vibrato_rate_target>1||!std::isfinite(state.time4)||state.time4<0)return PrefixStatus::Invalid;
 for(unsigned i=0;i<3;i++)if(!std::isfinite(state.phase[i])||state.phase[i]<0||!std::isfinite(state.held_elapsed[i])||state.held_elapsed[i]<0||!std::isfinite(input.rates[i])||input.rates[i]<0||input.waveforms[i]>5)return PrefixStatus::Invalid;
 for(float held:state.held)if(!std::isfinite(held)||held< -1||held>1)return PrefixStatus::Invalid;
 if(!sine.function||!entropy.next_word)return PrefixStatus::MissingCapability;
 TickPrefixOutput next{};next.state=state;
 const unsigned phase_order[3]={0,2,1};
 for(unsigned i:phase_order){if((i==0&&!state.delay1_finished)||(i==2&&!state.delay3_finished))continue;
  if(lfo_phase(state.phase[i],input.rates[i],input.dt,next.state.phase[i])!=Status::State)return PrefixStatus::MathFailure;
 }
 std::int64_t indices[4];
 for(unsigned i=0;i<3;i++)if(lfo_index(next.state.phase[i],indices[i])!=Status::State||indices[i]<0)return PrefixStatus::MathFailure;
 volatile float doubled=input.vibrato_rate_target+input.vibrato_rate_target;volatile float factor=3.f-doubled;
 next.captured_factor4=factor;
 if(lfo_index(state.time4*float(factor),indices[3])!=Status::State||indices[3]<0)return PrefixStatus::MathFailure;
 float sine_table[256],shapes[4][256],unused_entropy_table[256];std::uint32_t u=0x40c90fdb;float twopi;std::memcpy(&twopi,&u,4);
 for(unsigned i=0;i<256;i++){volatile float product=float(i)*twopi;volatile float angle=product/256.f;float value=sine.function(angle);if(!std::isfinite(value)||value< -1||value>1)return PrefixStatus::MathFailure;sine_table[i]=value;}
 const unsigned kinds[4]={2,3,1,4};
 for(unsigned k=0;k<4;k++)for(unsigned i=0;i<256;i++)if(shape_table_sample(kinds[k],i,shapes[k][i])!=Status::State)return PrefixStatus::MathFailure;
 EntropyResult draw=build_entropy_table12(entropy,budget,unused_entropy_table);next.entropy_requests=draw.requests;
 if(draw.status!=EntropyStatus::Complete)return PrefixStatus::EntropyFailure;
 for(unsigned i=0;i<3;i++)if(input.waveforms[i]==4){
  next.state.held_elapsed[i]=state.held_elapsed[i]+input.dt;
  if(!std::isfinite(next.state.held_elapsed[i]))return PrefixStatus::MathFailure;
  if(next.state.held_elapsed[i]>=1.f/float(input.rates[i])){
   draw=draw_held_sample(entropy,budget-next.entropy_requests,next.state.held[i]);next.entropy_requests+=draw.requests;
   if(draw.status!=EntropyStatus::Complete)return PrefixStatus::EntropyFailure;
   next.state.held_elapsed[i]=0.f;
  }
 }
 for(unsigned i=0;i<4;i++){
  unsigned ix=static_cast<unsigned>(indices[i]);next.rows[i][0]=shapes[2][(ix+64)%256];next.rows[i][1]=shapes[1][ix];next.rows[i][2]=shapes[3][ix];next.rows[i][3]=shapes[0][(ix+128)%256];next.rows[i][4]=next.state.held[i];next.rows[i][5]=sine_table[ix];
 }
 output=next;return PrefixStatus::PreparedPrefixOnly;
}
}
