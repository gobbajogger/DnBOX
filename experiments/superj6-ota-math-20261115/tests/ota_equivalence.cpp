#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <cfenv>
#define DunneCore BaselineOTA
#include "../baseline/OriginalOTAFilter.h"
#undef DunneCore
#include "../candidate/OriginalOTAFilter.h"
static unsigned word(float f){unsigned x;std::memcpy(&x,&f,4);return x;}
static double dbl(uint64_t x){double z;std::memcpy(&z,&x,8);return z;}
static uint64_t rng(uint64_t&s){s^=s<<13;s^=s>>7;s^=s<<17;return s;}
static uint64_t checks=0,frames=0,events=0;
static bool eq(float a,float b,const char*n){++checks;if(word(a)!=word(b)){std::printf("FAIL state%s %08x %08x frame%llu\n",n,word(a),word(b),(unsigned long long)frames);return false;}return true;}
static bool state(const BaselineOTA::OriginalOTAFilter&a,const DunneCore::OriginalOTAFilter&b){
for(int i=0;i<8;++i)if(!eq(a.up[i],b.up[i],"up")||!eq(a.down[i],b.down[i],"down"))return false;for(int i=0;i<4;++i)if(!eq(a.z[i],b.z[i],"z"))return false;
#define F(x) if(!eq(a.x,b.x,#x))return false;
F(rate)F(hp)F(hpTan)F(hpG)F(cutoffLog)F(g)F(gTarget)F(gStep)F(k)F(mode)F(modeTarget)F(modeStep)F(outGain)
#undef F
++checks;if(a.modeLeft!=b.modeLeft||a.gLeft!=b.gLeft||a.first!=b.first||a.seeded!=b.seeded||a.seedPermission!=b.seedPermission){std::puts("FAIL discrete state");return false;}return true;}
int main(){std::fesetround(FE_TONEAREST);uint64_t s=UINT64_C(0xa532f064);unsigned histories=0;const float rates[]={20000.f,22050.f,44100.f,48000.f,88200.f,96000.f,192000.f};
for(float rate:rates){for(unsigned history=0;history<8;++history){++histories;BaselineOTA::OriginalOTAFilter a;DunneCore::OriginalOTAFilter b;a.init(rate);b.init(rate);if(!state(a,b))return 1;
for(unsigned i=0;i<8192;++i){if(i%17==0){uint64_t r=rng(s);double cutoff=3.+double(r%4000000)/100.;double res=double((r>>32)%14000)/10000.-.2;
if(i%51==0)cutoff=dbl(UINT64_C(0x40af400010000001));if(i%85==0)res=dbl(UINT64_C(0x3fe0000010000001));if(history==1)res=1.;if(history==2)cutoff=rate*.49;if(history==3)cutoff=-1.;if(history==4)cutoff=1.e300;if(history==5)res=1.e300;if(history==6)res=-1.e300;
unsigned n=(r>>16)%257;a.setParameters(cutoff,res,n);b.setParameters(cutoff,res,n);events++;if(!state(a,b))return 1;}
if(i==2048){a.seedPermission=false;b.seedPermission=false;a.modeTarget=b.modeTarget=0.f;a.modeLeft=b.modeLeft=7;a.modeStep=b.modeStep=-a.mode/7.f;}
if(i==4096&&history%2==0){a.clear();b.clear();events++;if(!state(a,b))return 1;}
if(i==6144&&history%3==0){a.updateSampleRate(rate*1.01f);b.updateSampleRate(rate*1.01f);events++;if(!state(a,b))return 1;}
float x=float(int(rng(s)%65537)-32768)/32768.f*.2f;if(history==0&&i<512)x=0.f;if(history==7)x=(i%97==0)?.1f:0.f;float x1=a.process(x),x2=b.process(x);frames++;if(!eq(x1,x2,"output")||!state(a,b))return 1;}
}}
for(unsigned i=0;i<200000;++i){float q=float(int(rng(s)%131073)-65536)/65536.f;if(!eq(BaselineOTA::OriginalOTAFilter::curve(q),DunneCore::OriginalOTAFilter::curve(q),"curve"))return 1;}
std::printf("OTA histories=%u frames=%llu parameter_lifecycle_events=%llu exact_output_state_checks=%llu curves=200000 sizeof_baseline=%zu sizeof_candidate=%zu align_baseline=%zu align_candidate=%zu failures=0\n",histories,(unsigned long long)frames,(unsigned long long)events,(unsigned long long)checks,sizeof(BaselineOTA::OriginalOTAFilter),sizeof(DunneCore::OriginalOTAFilter),alignof(BaselineOTA::OriginalOTAFilter),alignof(DunneCore::OriginalOTAFilter));return 0;}
