#include "source_fx.hpp"
#include <cstdio>
#include <cmath>
#include <cstdint>
#include <cstring>
int main(){
 for(int rate:{44100,48000}){
  SourceFx f(rate);double energy=0;unsigned nonzero=0;
  f.parameters[delayTime]=.05f;f.parameters[delayMix]=.4f;f.parameters[delayOn]=1.f;f.parameters[reverbMix]=.25f;f.parameters[reverbOn]=1.f;f.parameters[phaserMix]=.3f;f.parameters[widen]=1.f;
  for(unsigned i=0;i<(unsigned)rate;++i){float x=.15f*std::sin(2.*3.141592653589793*220.*i/rate);float l=999,r=999;f.frame(x,&l,&r,.3f,.8f,.6f,.9f);if(!std::isfinite(l)||!std::isfinite(r))return 2;energy+=double(l)*l+double(r)*r;nonzero+=l!=0||r!=0;}
  if(energy<=0||nonzero==0)return 3;std::printf("{\"rate\":%d,\"frames\":%d,\"finite\":true,\"nonzero_frames\":%u,\"energy\":%.17g}\n",rate,rate,nonzero,energy);
 }
}
