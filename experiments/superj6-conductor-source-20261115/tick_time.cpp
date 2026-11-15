#include "tick_time.hpp"
#include <cmath>
namespace original_conductor_v1 {
TimeStatus advance_tick_times(TickTimes& s,double dt,double delay1,double delay3,RestartVibrato provider)noexcept{
 if(!std::isfinite(dt)||dt<0||!std::isfinite(delay1)||delay1<0||!std::isfinite(delay3)||delay3<0)return TimeStatus::Invalid;
 for(double t:s.current)if(!std::isfinite(t)||t<0)return TimeStatus::Invalid;
 for(double t:s.delay_elapsed)if(!std::isfinite(t)||t<0)return TimeStatus::Invalid;
 double passed1=s.delay_elapsed[0]+dt,passed3=s.delay_elapsed[2]+dt;
 if(!std::isfinite(passed1)||!std::isfinite(passed3))return TimeStatus::Invalid;
 bool crossed1=!s.delay_finished[0]&&passed1>delay1;
 bool finished1=s.delay_finished[0]||crossed1,finished3=s.delay_finished[2]||passed3>delay3;
 if(!std::isfinite(s.current[1]+dt)||(finished1&&(!std::isfinite(s.current[0]+dt)||!std::isfinite(s.current[3]+dt)))||(finished3&&!std::isfinite(s.current[2]+dt)))return TimeStatus::Invalid;
 bool restart=crossed1&&delay1>=.2;
 if(restart&&!provider.restart_collection_and_noise)return TimeStatus::MissingCapability;
 s.delay_elapsed[0]=passed1;
 if(crossed1){s.delay_finished[0]=true;if(restart&&!provider.restart_collection_and_noise(provider.context))return TimeStatus::RestartFailedAfterStateChange;}
 if(s.delay_finished[0]){s.current[0]+=dt;s.current[3]+=dt;}
 s.current[1]+=dt;
 s.delay_elapsed[2]=passed3;
 if(!s.delay_finished[2]&&passed3>delay3)s.delay_finished[2]=true;
 if(s.delay_finished[2])s.current[2]+=dt;
 return TimeStatus::Updated;
}
}
