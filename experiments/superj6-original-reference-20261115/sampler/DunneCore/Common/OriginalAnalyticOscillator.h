// Source-derived analytic fallback waveforms; not the complete original instrument.
#pragma once
#include <cmath>
namespace DunneCore {
struct OriginalAnalyticOscillator {
 float phase=0,increment=0,down[8]={};
 void reset(){phase=increment=0;for(float &x:down)x=0;}
 void restart(){phase=0;for(float &x:down)x=0;}
 static float blep(float t,float dt){
  if(t<dt){const float x=t/dt;return std::fma(-x,x,x+x)+(-1.f);}
  if(t>1.f-dt){const float x=(t+(-1.f))/dt;const float y=std::fma(x,x,x);return(x+y)+1.f;}
  return 0.f;
 }
 static float blamp(float t,float dt){
  if(t<dt){const float x=t/dt+(-1.f);return ((x*x)*x)*(-.1666666716337204f);}
  if(t>1.f-dt){const float x=(t+(-1.f))/dt+1.f;return (x*(x*x))*.1666666716337204f;}
  return 0.f;
 }
 static float saw(float p,float dt){const float t=p-std::floor(p);return std::fma(t,2.f,-1.f)-blep(t,dt);}
 static float tri(float p,float dt){
  const float t=p-std::floor(p);float shift=t+.5f;if(shift>=1.f)shift+=-1.f;
  const float base=std::fma(std::fabs(std::fma(t,2.f,-1.f)),2.f,-1.f);
  return std::fma(dt*8.f,blamp(shift,dt)-blamp(t,dt),base);
 }
 float advance(float dt){
  const float second=phase+dt;float mid=second;if(mid>=1.f)mid+=-1.f;
  phase=mid+dt;if(phase>=1.f)phase+=-1.f;return mid;
 }
 float decimate(float first,float second){
  float a=std::fma(second-down[2],.079866424202919f,down[0]);
  float b=std::fma(a-down[3],.5453536510467529f,down[1]);
  float c=std::fma(first-down[6],.28382933139801025f,down[4]);
  float d=std::fma(c-down[7],.8344119191169739f,down[5]);
  down[0]=second;down[1]=a;down[2]=a;down[3]=b;down[4]=first;down[5]=c;down[6]=c;down[7]=d;
  return(b+d)*.5f;
 }
 float processSaw(float gain){
  const float dt=increment*.5f;const float first=saw(phase-std::floor(phase),dt);const float mid=advance(dt);const float second=saw(mid,dt);
  return((gain*.5f)*decimate(first,second))*.6800000071525574f;
 }
 float processTriangle(float gain){
  const float dt=increment*.5f;const float first=tri(phase,dt);const float mid=advance(dt);const float second=tri(mid,dt);
  return((gain*.5f)*decimate(first,second))*.9900000095367432f;
 }
 float processSine(float gain){
  const float dt=increment*.5f,firstPhase=phase;const float mid=advance(dt);
  const float second=std::sin(mid*6.2831854820251465f);
  const float first=std::sin(firstPhase*6.2831854820251465f);
  return(gain*.5f)*decimate(first,second);
 }
 float processPWM(float gain,float width){
  const float dt=increment*.5f,firstPhase=phase;const float mid=advance(dt);
  const float first=(saw(firstPhase,dt)-saw(firstPhase+width,dt))*.5f;
  const float second=(saw(mid,dt)-saw(mid+width,dt))*.5f;
  const float g=(gain*.8999999761581421f)*.6600000262260437f;
  return decimate(first*g,second*g);
 }
};
}
