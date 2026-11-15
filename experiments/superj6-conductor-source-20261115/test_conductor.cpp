#include "conductor.hpp"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>
using namespace original_conductor_v1;
static double pw(double a,double b) noexcept{return std::pow(a,b);}
static double ex(double x) noexcept{return std::exp(x);}
static int failures=0,checks=0;
#define CHECK(x) do{++checks;if(!(x)){++failures;std::fprintf(stderr,"FAIL %d: %s\n",__LINE__,#x);}}while(0)
static double badpw(double,double) noexcept{return 2.;}
static double badex(double) noexcept{return -1.;}
int main(){
 Providers p{31,pw,ex};Command out[256]{};std::size_t n=999;
 Record r[]={{"neoncutoffLayer1",.4},{"neonvolumeControl",1.32},{"neonvolumeControl",0.}};
 auto st=plan_main(r,3,7,true,p,out,256,n);CHECK(st==Status::MainPassOnly);
 if(st==Status::MainPassOnly){CHECK(n==167);CHECK(out[0].id==0&&out[0].value==double(float(1.32)));CHECK(out[1].id==7&&out[1].value==double(float(.4)));}
 n=999;CHECK(plan_main(r,3,7,true,p,out,0,n)==Status::Capacity);CHECK(n==999);
 Cutoff s{4,float(.4999),.5f,true},next{};n=999;
 st=cutoff_step(s,.001,p,out,256,n,next);CHECK(st==Status::PendingAck);
 if(st==Status::PendingAck){CHECK(n==2&&out[0].value!=.5&&next.current==.5f);CHECK(next.awaiting&&next.active);CHECK(out[0].token==next.token);Cutoff ack{};CHECK(cutoff_ack(next,4,next.token,.5f,ack)==Status::State);CHECK(!ack.active&&!ack.awaiting);}
 double dt=-1;CHECK(free_block_time(64,48000.,dt)==Status::Clock);CHECK(dt==64./48000.);
 Beat b{2,0.,0.},bn{};n=999;st=beat_catchup(b,.05,120.,true,out,256,n,bn);CHECK(st==Status::Clock);if(st==Status::Clock)CHECK(n==2&&out[0].value==.0125);
 Beat same{2,2.,.006};n=999;CHECK(beat_catchup(same,2.,120.,true,out,256,n,bn)==Status::Clock);CHECK(bn.accumulator==.006);
 Note note{{0,2,3},{false,false,false},false,false,false,{true,true,true},{.3,.4,.5}},nn{};
 CHECK(enter_note(note,0,nn)==Status::State);CHECK(nn.retriggers[1]&&nn.new_note&&nn.retrigger4&&nn.delay_passed[0]==0.);
 CHECK(enter_note(note,1,nn)==Status::State);CHECK(nn.delay_passed[0]==.3&&!nn.new_note);
 n=999;Providers badpow{31,badpw,ex};CHECK(plan_main(nullptr,0,7,true,badpow,out,256,n)==Status::MathFailure);CHECK(n==999);
 n=999;Providers badexp{31,pw,badex};CHECK(cutoff_step(s,.001,badexp,out,256,n,next)==Status::MathFailure);CHECK(n==999);
 float dv=-9.f;CHECK(query_default(218,p,dv)==Status::Invalid);CHECK(dv==-9.f);CHECK(query_default(0,p,dv)==Status::State);CHECK(dv>=0.f&&dv<=1.f);

 // Failure atomicity on actual valid objects; no arbitrary invalid pointer probes.
 Command saved[256];std::memcpy(saved,out,sizeof(out));n=777;
 Providers nocaps{0,pw,ex};CHECK(plan_main(r,3,1,true,nocaps,out,256,n)==Status::MissingCapability);CHECK(n==777&&std::memcmp(saved,out,sizeof(out))==0);
 Record huge[]={{"unknown",std::numeric_limits<double>::max()}};CHECK(plan_main(huge,1,1,true,p,out,256,n)==Status::Invalid);CHECK(n==777);
 CHECK(plan_main(nullptr,1,1,true,p,out,256,n)==Status::Invalid);CHECK(n==777);
 CHECK(plan_main(r,4097,1,true,p,out,256,n)==Status::Invalid);CHECK(n==777);
 CHECK(plan_main(r,3,1,true,p,out,256,*reinterpret_cast<std::size_t*>(out))==Status::Invalid);CHECK(std::memcmp(saved,out,sizeof(out))==0);
 Cutoff pending{4,.5f,.5f,true,3,true},unchanged{99,.1f,.2f,false,1,false};next=unchanged;
 CHECK(cutoff_step(pending,.01,p,out,256,n,next)==Status::AwaitingAck);CHECK(next.generation==99&&n==777);
 CHECK(cutoff_ack(pending,5,3,.5f,next)==Status::StaleAck);CHECK(next.generation==99);
 CHECK(cutoff_ack(pending,4,2,.5f,next)==Status::StaleAck);CHECK(next.generation==99);
 CHECK(cutoff_ack(pending,4,3,.4f,next)==Status::State);CHECK(next.active&&!next.awaiting);
 Cutoff inplace{1,.2f,.8f,true};CHECK(cutoff_step(inplace,.01,p,out,256,n,inplace)==Status::PendingAck);CHECK(inplace.awaiting);
 double unchanged_dt=9.;CHECK(free_block_time(1,std::numeric_limits<double>::denorm_min(),unchanged_dt)==Status::Invalid);CHECK(unchanged_dt==9.);
 Beat badbeat{1,0.,.0125};bn={99,9.,.001};n=777;CHECK(beat_catchup(badbeat,0.,120.,true,out,256,n,bn)==Status::Invalid);CHECK(bn.generation==99&&n==777);
 for(unsigned id=0;id<218;id++){float value=-1.f;CHECK(query_default(id,p,value)==Status::State);CHECK(std::isfinite(value));}
 const char unterminated[3]={'f','o','o'};Identifier bounded_name(unterminated);CHECK(bounded_name.size()==3);CHECK(bounded_name=="foo");
 std::printf("checks=%d failures=%d\n",checks,failures);return failures?1:0;
}
