// Authentic source control routing adapter, private unreviewed experiment.
#pragma once
#include "full_oracle.hpp"
#include <cmath>
#include <string>
#include <type_traits>
// Original Apple/Clang floating overload, not GCC integer abs.
using std::abs;
static_assert(std::is_same<decltype(abs(1.f)),float>::value,"floating abs overload required");
#include "S1Rate.hpp"
namespace s1frontend_oracle {
typedef struct DependentParameter {
    S1Parameter parameter;
    float normalizedValue;// [0,1] for ui
    float value;
    int payload;
} DependentParameter;
class OracleControls {
 s1scheduler::FullOracle*s_;float*parameters;
 S1Rate _rate;
 DependentParameter _lfo1Rate{},_lfo2Rate{},_autoPanRate{},_delayTime{},_pitchbend{},_arpSeqTempoMultiplier{};
 unsigned ready_=0;bool failed_=false;
 static unsigned slot(unsigned p){switch(p){case lfo1Rate:return 1;case lfo2Rate:return 2;case autoPanFrequency:return 4;case delayTime:return 8;case pitchbend:return 16;case arpSeqTempoMultiplier:return 32;default:return 0;}}
 float minimum(S1Parameter p){return ::minimums[p];}float maximum(S1Parameter p){return ::maximums[p];}
 float clampedValue(S1Parameter p,float x){return s_->clampedValue(p,x);}
 float getParameter(S1Parameter p){return parameters[p];}
 float getSynthParameter(S1Parameter p){return (s_->s1p[p].usePortamento?s_->s1p[p].portamentoTarget:s_->parameters[p]);}
 void _setSynthParameter(S1Parameter p,float x){s_->_setSynthParameter(p,clampedValue(p,x));}
 void updatePortamento(float x){for(auto &p:s_->s1p)if(p.usePortamento)p.portamento->htime=clampedValue(portamentoHalfTime,x);}
 void dependentParameterDidChange(DependentParameter p){ready_|=slot(p.parameter);if(event_count<8)events[event_count++]=p;else failed_=true;}
 void setSynthParameter(S1Parameter,float);void _rateHelper(S1Parameter,float,bool,int);void _setSynthParameterHelper(S1Parameter,float,bool,int);
 float getDependentParameter(S1Parameter);void setDependentParameter(S1Parameter,float,int);
 float taper01(float,float);float taper01Inverse(float,float);
 public:
 DependentParameter events[8]{};unsigned event_count=0;
 OracleControls(const OracleControls&)=delete;OracleControls&operator=(const OracleControls&)=delete;
 explicit OracleControls(s1scheduler::FullOracle&s):s_(&s),parameters(s.parameters){}
 // Single serialized caller; backend lives longer and remains initialized.
 // Notifications are captured by value, not transmitted to UI/thread queues.
 // A getter is unavailable until the corresponding actual source setter emitted
 // a dependent update; no invented cold uninitialized notification values.
 bool set_parameter(unsigned p,float x){if(!s_->initialized()||p>=150||!std::isfinite(x))return false;event_count=0;failed_=false;setSynthParameter(static_cast<S1Parameter>(p),x);return !failed_;}
 bool set_normalized(unsigned p,float x,int payload){if(!s_->initialized()||!slot(p)||!std::isfinite(x)||x<0||x>1)return false;event_count=0;failed_=false;setDependentParameter(static_cast<S1Parameter>(p),x,payload);return !failed_;}
 bool normalized(unsigned p,float&out){if(!slot(p)||!(ready_&slot(p)))return false;float v=getDependentParameter(static_cast<S1Parameter>(p));if(!std::isfinite(v))return false;out=v;return true;}
 float target(unsigned p)const{return p<150?(s_->s1p[p].usePortamento?s_->s1p[p].portamentoTarget:s_->parameters[p]):0.f;}
};
}
