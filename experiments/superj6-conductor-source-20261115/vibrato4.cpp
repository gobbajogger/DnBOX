#include "vibrato4.hpp"
#include <cmath>
namespace original_conductor_v1 {
namespace {
bool valid(const LiveVibrato& c)noexcept{return std::isfinite(c.delay1)&&c.delay1>=0&&std::isfinite(c.tempo)&&c.tempo>=0&&std::isfinite(c.rate_target)&&c.rate_target>=0&&c.rate_target<=1&&std::isfinite(c.depth_target)&&c.depth_target>=0&&c.depth_target<=1;}
bool frequency(float normalized,const LiveVibrato& c,Providers p,float& out)noexcept{
 if(c.tempo_sync){static const float factors[18]={64.f,48.f,32.f,24.f,16.f,12.f,8.f,6.f,4.f,3.f,2.f,1.5f,1.f,.5f,.3333f,.25f,.1667f,.125f};
  volatile float inverse=1.f-normalized;volatile float scaled=inverse*17.f;unsigned index=static_cast<unsigned>(float(scaled)+.5f);if(index>=18)return false;
  out=((c.tempo/60.f)*factors[index])*.25f;
 }else{float power=p.pow(float(normalized),3.33f);if(!std::isfinite(power)||power<0||power>1)return false;out=16.f*power;}
 return std::isfinite(out);
}
}
VibratoResult apply_vibrato4(float factor,const float (&row)[6],Vibrato4State& state,Providers providers,VibratoBackend backend)noexcept{
 if(!std::isfinite(factor)||factor<1||factor>3||!std::isfinite(state.current_time)||state.current_time<0)return {VibratoStatus::Invalid,0,false};
 for(float value:row)if(!std::isfinite(value)||value< -1||value>1)return {VibratoStatus::Invalid,0,false};
 if(!state.armed||!state.delay_finished)return {VibratoStatus::Noop,0,false};
 if(!backend.read_live)return {VibratoStatus::MissingCapability,0,false};
 LiveVibrato live;if(!backend.read_live(backend.context,live))return {VibratoStatus::ReadFailure,0,false};if(!valid(live))return {VibratoStatus::Invalid,0,false};
 if(live.delay1<.02f){bool changed=!state.cycle_complete;state.cycle_complete=true;return {VibratoStatus::Noop,0,changed};}
 if(state.cycle_complete)return {VibratoStatus::Noop,0,false};
 if(!providers.pow||!backend.apply_frequency||!backend.apply_depth)return {VibratoStatus::MissingCapability,0,false};
 bool changed=false;unsigned effects=0;
 if(state.current_time>=1.f/float(factor)){
  state.cycle_complete=true;changed=true;float mapped;if(!frequency(live.rate_target,live,providers,mapped))return {VibratoStatus::PartialFailure,effects,changed};
  ++effects;if(!backend.apply_frequency(backend.context,live.rate_target,mapped))return {VibratoStatus::PartialFailure,effects,changed};
  if(!backend.read_live(backend.context,live)||!valid(live))return {VibratoStatus::PartialFailure,effects,changed};
 }
 const float multiplier=state.cycle_complete?1.f:(row[1]+1.f)*.5f;
 const float rate_product=float(live.rate_target)*multiplier;float rate=rate_product+0.f;if(rate<0)rate=0;if(rate_product>1)rate=1;
 const float depth_product=float(live.depth_target)*multiplier+0.f;
 const float normalized_rate=static_cast<float>(rate),normalized_depth=static_cast<float>(depth_product);
 float mapped;if(!frequency(normalized_rate,live,providers,mapped))return {VibratoStatus::PartialFailure,effects,changed};
 ++effects;if(!backend.apply_frequency(backend.context,normalized_rate,mapped))return {VibratoStatus::PartialFailure,effects,changed};
 // Source depth mapping narrows once to Float, then widens for its receiver group.
 float power=providers.pow(float(normalized_depth),3.03f);if(!std::isfinite(power)||power<0||power>1)return {VibratoStatus::PartialFailure,effects,changed};
 const float mapped_depth=float(static_cast<float>(12.f*power));
 ++effects;if(!backend.apply_depth(backend.context,normalized_depth,mapped_depth))return {VibratoStatus::PartialFailure,effects,changed};
 return {VibratoStatus::AppliedCommandsOnly,effects,changed};
}
}
