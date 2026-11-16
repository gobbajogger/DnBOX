#include "lfo_laws.hpp"
#include <cmath>
#include <limits>
namespace original_conductor_v1 {
namespace {
float add(float a,float b)noexcept{volatile float x=a+b;return x;}
float sub(float a,float b)noexcept{volatile float x=a-b;return x;}
float mul(float a,float b)noexcept{volatile float x=a*b;return x;}
float div(float a,float b)noexcept{volatile float x=a/b;return x;}
bool representable(float x)noexcept{return std::isfinite(x)&&std::fabs(x)<=std::numeric_limits<float>::max();}
}
Status lfo_rate(float normalized,bool hz,float tempo,Providers p,float& out)noexcept{
 if(!std::isfinite(normalized)||normalized<0||normalized>1||!std::isfinite(tempo)||tempo<0)return Status::Invalid;
 float value;
 if(hz){if(!p.pow)return Status::MissingPow;float powered=p.pow(float(normalized),3.33f);if(!std::isfinite(powered)||powered<0||powered>1)return Status::MathFailure;value=16.f*powered;}
 else{
  static const float factors[18]={64.f,48.f,32.f,24.f,16.f,12.f,8.f,6.f,4.f,3.f,2.f,1.5f,1.f,.5f,.3333f,.25f,.1667f,.125f};
  float scaled=mul(sub(1.f,normalized),17.f);
  // Positive-domain equivalent of source FRINTA; no substitution of exact thirds.
  unsigned i=static_cast<unsigned>(float(scaled)+.5f);if(i>=18)return Status::Invalid;
  value=((tempo/60.f)*factors[i])*.25f;
 }
 if(!representable(value))return Status::MathFailure;
 out=static_cast<float>(value);return Status::State;
}
Status lfo_phase(float phase,float rate,float dt,float& out)noexcept{
 if(!std::isfinite(phase)||!std::isfinite(rate)||rate<0||!std::isfinite(dt)||dt<0)return Status::Invalid;
 float next=phase+float(rate)*dt;if(!std::isfinite(next))return Status::MathFailure;
 if(next>=1.f){next-=1.f;}
 out=next;return Status::State;
}
Status lfo_index(float phase,std::int64_t& out)noexcept{
 if(!std::isfinite(phase))return Status::Invalid;
 float x=phase*256.f;if(!std::isfinite(x)||x< -9223372036854775808.0f||x>=9223372036854775808.0f)return Status::Invalid;
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
Status lfo_modulate(float base,float wave,float amount,unsigned waveform,float& out)noexcept{
 if(!std::isfinite(base)||!std::isfinite(wave)||wave< -1||wave>1||!std::isfinite(amount)||amount<0||amount>1||waveform>5)return Status::Invalid;
 float delta=waveform==1?amount*((wave+1.f)*.5f):waveform==2?amount*((wave-1.f)*.5f):(wave*amount)*.5f;
 float value=delta+float(base);if(value<0)value=0;if(value>1)value=1;
 out=static_cast<float>(value);return Status::State;
}
}
