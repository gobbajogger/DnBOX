#include "SampleBuffer.h"
#include "CoreSampler.h"
#include <vector>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <limits>
using namespace DunneCore;
static unsigned long long checks=0;
#define CK(x) do{if(!(x)){std::fprintf(stderr,"FAIL %d %s\n",__LINE__,#x);return 1;}++checks;}while(0)
static unsigned bits(float f){unsigned u;std::memcpy(&u,&f,4);return u;}
int main(){
 const unsigned count=65536;std::vector<unsigned char> raw(count*4);std::vector<float> normalized(count*2);
 for(unsigned c=0;c<2;++c)for(unsigned i=0;i<count;++i){
  const unsigned u=c?65535-i:i;raw[(c*count+i)*2]=u&255;raw[(c*count+i)*2+1]=u>>8;normalized[c*count+i]=float(u>=32768?int(u)-65536:int(u))*.000030517578125f;
 }
 const auto immutable=raw;
 for(unsigned ch:{1u,2u}){
  SampleBuffer owned,view;owned.init(44100,ch,count);for(unsigned i=0;i<count*ch;++i)owned.setData(i,normalized[i]);
  CK(view.initBorrowedPcm16(44100,ch,count,raw.data(),count*ch*2));CK(view.samples==nullptr&&view.ownedSampleWords==0);
  for(unsigned i=0;i<count*ch;++i)CK(bits(view.readValue(i))==bits(normalized[i]));
  unsigned r=0x32f0ac1u;
  for(unsigned i=0;i<300000;++i){
   r^=r<<13;r^=r>>17;r^=r<<5;double pos=double(r%count)+double((r>>16)&65535)/65536.;float gain=float((r>>8)%1025)/1024.f;
   float a,b,c,d;owned.interp(pos,&a,&b,gain);view.interp(pos,&c,&d,gain);CK(bits(a)==bits(c));CK(bits(b)==bits(d));CK(bits(owned.interp(pos,gain))==bits(view.interp(pos,gain)));
  }
  CK(raw==immutable);
  view.setData(0xffffffffu,.125f);CK(view.samples==nullptr&&view.pcm16Data==raw.data());CK(raw==immutable);CK(view.readValue(0xffffffffu)==0);
  view.setData(count*ch-1,.25f);CK(view.pcm16Data==nullptr&&view.samples!=nullptr&&view.ownedSampleWords==count*ch);CK(bits(view.readValue(count*ch-1))==bits(.25f));CK(raw==immutable);
 }
 SampleBuffer b;b.init(44100,1,8);for(unsigned i=0;i<8;++i)b.setData(i,float(i));auto p=b.samples;
 auto reject=[&](float sr,int ch,int n,const unsigned char*d,std::size_t bytes){
  bool accepted=b.initBorrowedPcm16(sr,ch,n,d,bytes);
  return !accepted&&b.samples==p&&b.pcm16Data==nullptr&&b.sampleCount==8&&b.channelCount==1&&b.sampleRate==44100&&b.ownedSampleWords==8&&b.samples[7]==7;
 };
 CK(reject(0,1,8,raw.data(),16));CK(reject(std::numeric_limits<float>::quiet_NaN(),1,8,raw.data(),16));CK(reject(44100,0,8,raw.data(),0));CK(reject(44100,3,8,raw.data(),48));CK(reject(44100,1,0,raw.data(),0));CK(reject(44100,1,16777217,raw.data(),33554434));CK(reject(44100,1,8,nullptr,16));CK(reject(44100,1,8,raw.data(),15));CK(reject(44100,1,8,reinterpret_cast<unsigned char*>(&b),16));CK(reject(44100,1,8,reinterpret_cast<unsigned char*>(p),16));CK(reject(44100,1,8,reinterpret_cast<unsigned char*>(UINTPTR_MAX-7),16));
 {
  CoreSampler s;s.init(48000);SampleDescriptor d{};d.noteNumber=60;d.noteFrequency=261.62558f;d.minimumNoteNumber=0;d.maximumNoteNumber=127;d.minimumVelocity=0;d.maximumVelocity=127;d.isLooping=true;d.loopStartPoint=16;d.loopEndPoint=100;d.endPoint=127;
  CK(!s.loadBorrowedPcm16(d,44100,1,128,reinterpret_cast<const unsigned char*>(&s),256));
  CK(s.loadBorrowedPcm16(d,44100,1,128,raw.data(),256));s.buildKeyMap();s.playNote(60,100);
  for(unsigned n=0;n<100;++n){float l[16]={},r[16]={};float*out[]={l,r};s.render(2,16,out);for(float f:l)CK(std::isfinite(f));for(float f:r)CK(std::isfinite(f));}
  s.stopNote(60,true);s.unloadAllSamples();
 }
 CK(raw==immutable);
 std::printf("PASS %llu exact PCM16 word/interpolation/copy-on-write/preflight/lifetime source checks; no original/target execution or storage mapping\n",checks);
}
