#include "lfo_laws.hpp"
#include <cmath>
#include <limits>
namespace original_conductor_v1 {
namespace {
float add(float a,float b)noexcept{volatile float x=a+b;return x;}
float sub(float a,float b)noexcept{volatile float x=a-b;return x;}
float mul(float a,float b)noexcept{volatile float x=a*b;return x;}
float div(float a,float b)noexcept{volatile float x=a/b;return x;}
bool representable(double x)noexcept{return std::isfinite(x)&&std::fabs(x)<=std::numeric_limits<float>::max();}
}
Status lfo_rate(float normalized,bool hz,double tempo,Providers p,float& out)noexcept{
 if(!std::isfinite(normalized)||normalized<0||normalized>1||!std::isfinite(tempo)||tempo<0)return Status::Invalid;
 double value;
 if(hz){if(!p.pow)return Status::MissingPow;double powered=p.pow(double(normalized),3.33);if(!std::isfinite(powered)||powered<0||powered>1)return Status::MathFailure;value=16.*powered;}
 else{
  static const double factors[18]={64.,48.,32.,24.,16.,12.,8.,6.,4.,3.,2.,1.5,1.,.5,.3333,.25,.1667,.125};
  float scaled=mul(sub(1.f,normalized),17.f);
  // Positive-domain equivalent of source FRINTA; no substitution of exact thirds.
  unsigned i=static_cast<unsigned>(double(scaled)+.5);if(i>=18)return Status::Invalid;
  value=((tempo/60.)*factors[i])*.25;
 }
 if(!representable(value))return Status::MathFailure;
 out=static_cast<float>(value);return Status::State;
}
Status lfo_phase(double phase,float rate,double dt,double& out)noexcept{
 if(!std::isfinite(phase)||!std::isfinite(rate)||rate<0||!std::isfinite(dt)||dt<0)return Status::Invalid;
 double next=phase+double(rate)*dt;if(!std::isfinite(next))return Status::MathFailure;
 if(next>=1.){next-=1.;}
 out=next;return Status::State;
}
Status lfo_index(double phase,std::int64_t& out)noexcept{
 if(!std::isfinite(phase))return Status::Invalid;
 double x=phase*256.;if(!std::isfinite(x)||x< -9223372036854775808.0||x>=9223372036854775808.0)return Status::Invalid;
 out=static_cast<std::int64_t>(x)%256;return Status::State;
}
Status shape_table_sample(unsigned kind,unsigned i,float& out)noexcept{
 if(i>=256)return Status::Invalid;
 float v;
 switch(kind){
 case 1:v=i<128?sub(mul(div(4.f,256.f),float(i)),1.f):add(mul(div(4.f,256.f),-float(i)),3.f);break;
 case 2:v=i<128?-1.f:1.f;break;
 case 3:v=sub(div(float(i+i),256.f),1.f);break;
 case 4:v=sub(1.f,div(float(i+i),256.f));break;
 default:return Status::Unsupported;
 }
 out=v;return Status::State;
}
Status lfo_modulate(float base,double wave,double amount,unsigned waveform,float& out)noexcept{
 if(!std::isfinite(base)||!std::isfinite(wave)||wave< -1||wave>1||!std::isfinite(amount)||amount<0||amount>1||waveform>5)return Status::Invalid;
 double delta=waveform==1?amount*((wave+1.)*.5):waveform==2?amount*((wave-1.)*.5):(wave*amount)*.5;
 double value=delta+double(base);if(value<0)value=0;if(value>1)value=1;
 out=static_cast<float>(value);return Status::State;
}
}
