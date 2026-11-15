#include "vibrato4.hpp"
#include <cmath>
namespace original_conductor_v1 {
namespace {
bool valid(const LiveVibrato& c)noexcept{return std::isfinite(c.delay1)&&c.delay1>=0&&std::isfinite(c.tempo)&&c.tempo>=0&&std::isfinite(c.rate_target)&&c.rate_target>=0&&c.rate_target<=1&&std::isfinite(c.depth_target)&&c.depth_target>=0&&c.depth_target<=1;}
bool frequency(float normalized,const LiveVibrato& c,Providers p,double& out)noexcept{
 if(c.tempo_sync){static const double factors[18]={64.,48.,32.,24.,16.,12.,8.,6.,4.,3.,2.,1.5,1.,.5,.3333,.25,.1667,.125};
  volatile float inverse=1.f-normalized;volatile float scaled=inverse*17.f;unsigned index=static_cast<unsigned>(double(scaled)+.5);if(index>=18)return false;
  out=((c.tempo/60.)*factors[index])*.25;
 }else{double power=p.pow(double(normalized),3.33);if(!std::isfinite(power)||power<0||power>1)return false;out=16.*power;}
 return std::isfinite(out);
}
}
VibratoResult apply_vibrato4(float factor,const double (&row)[6],Vibrato4State& state,Providers providers,VibratoBackend backend)noexcept{
 if(!std::isfinite(factor)||factor<1||factor>3||!std::isfinite(state.current_time)||state.current_time<0)return {VibratoStatus::Invalid,0,false};
 for(double value:row)if(!std::isfinite(value)||value< -1||value>1)return {VibratoStatus::Invalid,0,false};
 if(!state.armed||!state.delay_finished)return {VibratoStatus::Noop,0,false};
 if(!backend.read_live)return {VibratoStatus::MissingCapability,0,false};
 LiveVibrato live;if(!backend.read_live(backend.context,live))return {VibratoStatus::ReadFailure,0,false};if(!valid(live))return {VibratoStatus::Invalid,0,false};
 if(live.delay1<.02){bool changed=!state.cycle_complete;state.cycle_complete=true;return {VibratoStatus::Noop,0,changed};}
 if(state.cycle_complete)return {VibratoStatus::Noop,0,false};
 if(!providers.pow||!backend.apply_frequency||!backend.apply_depth)return {VibratoStatus::MissingCapability,0,false};
 bool changed=false;unsigned effects=0;
 if(state.current_time>=1./double(factor)){
  state.cycle_complete=true;changed=true;double mapped;if(!frequency(live.rate_target,live,providers,mapped))return {VibratoStatus::PartialFailure,effects,changed};
  ++effects;if(!backend.apply_frequency(backend.context,live.rate_target,mapped))return {VibratoStatus::PartialFailure,effects,changed};
  if(!backend.read_live(backend.context,live)||!valid(live))return {VibratoStatus::PartialFailure,effects,changed};
 }
 const double multiplier=state.cycle_complete?1.:(row[1]+1.)*.5;
 const double rate_product=double(live.rate_target)*multiplier;double rate=rate_product+0.;if(rate<0)rate=0;if(rate_product>1)rate=1;
 const double depth_product=double(live.depth_target)*multiplier+0.;
 const float normalized_rate=static_cast<float>(rate),normalized_depth=static_cast<float>(depth_product);
 double mapped;if(!frequency(normalized_rate,live,providers,mapped))return {VibratoStatus::PartialFailure,effects,changed};
 ++effects;if(!backend.apply_frequency(backend.context,normalized_rate,mapped))return {VibratoStatus::PartialFailure,effects,changed};
 // Source depth mapping narrows once to Float, then widens for its receiver group.
 double power=providers.pow(double(normalized_depth),3.03);if(!std::isfinite(power)||power<0||power>1)return {VibratoStatus::PartialFailure,effects,changed};
 const double mapped_depth=double(static_cast<float>(12.*power));
 ++effects;if(!backend.apply_depth(backend.context,normalized_depth,mapped_depth))return {VibratoStatus::PartialFailure,effects,changed};
 return {VibratoStatus::AppliedCommandsOnly,effects,changed};
}
}
