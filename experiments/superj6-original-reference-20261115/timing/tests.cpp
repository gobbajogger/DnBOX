#include "saga_core.hpp"
#include <cassert>
#include <cstdio>
#include <set>
using namespace j6_saga;
static unsigned checks=0;
#define CHECK(x) do { if(!(x)){std::fprintf(stderr,"FAIL %s:%d %s\n",__FILE__,__LINE__,#x);return 1;}++checks; }while(0)
int main(){
 for(unsigned i=0;i<divisions.size();++i){auto d=duration(1.f-float(i)/17.f);CHECK(d&&*d==divisions[i]);}
 CHECK(*duration(.5f)==1.5f);CHECK(*duration(*rate_knob(0))==4.f);CHECK(*duration(*rate_knob(.5f))==.5f);CHECK(*duration(*rate_knob(1))==.0625f);
 CHECK(*snap(1.f,.5f,0)==1.f);CHECK(*snap(1.f+4e-10f,.5f,0)==1.f);CHECK(*snap(1.f+6e-10f,.5f,0)==1.f);CHECK(*snap(1.f+1e-7f,.5f,0)==1.5f);CHECK(*snap(-.1f,.5f,0)==0.f);CHECK(*snap(1.f,.5f,.25f)==1.25f);
 const std::vector<std::uint8_t> held{60,64,67};
 const std::vector<std::vector<int>> expected{{60,64,67},{67,64,60},{60,64,67,64},{60,60,64,64,67,67,64,64}};
 for(int m=0;m<4;++m){auto p=arp_pool(held,0,12,m);CHECK(p.status==Status::ok&&p.notes==expected[m]);}
 for(int m=0;m<3;++m)CHECK(arp_pool({60},0,12,m).notes==std::vector<int>{60});
 CHECK((arp_pool({60},0,12,3).notes==std::vector<int>{60,60}));
 CHECK((arp_pool({60,64},0,12,2).notes==std::vector<int>{60,64}));
 CHECK((arp_pool({64,60},1,7,0).notes==std::vector<int>{60,64,67,71}));
 CHECK((arp_pool(held,0,12,2).notes!=std::vector<int>{60,64,67,64,60}));
 CHECK((*sequence_notes({60,64},-3,1)==std::vector<std::uint8_t>{57,61}));
 CHECK((*sequence_notes({0,127},24,1)==std::vector<std::uint8_t>{24,127}));
 CHECK(sequence_notes({60},7,.999f)->empty());CHECK((*sequence_notes({60},-3.9f,1)==std::vector<std::uint8_t>{57}));
 const unsigned px[]={144,26,7,88,96,100,15,72,104,61},py[]={66,27,8,89,97,101,13,73,103,60};
 for(int i=0;i<10;++i){auto t=pad_targets(i);CHECK(t&&(*t)[0]==px[i]&&(*t)[1]==py[i]);}
 CHECK(*next_pad(9,1,1)==0);CHECK(*next_pad(0,8,-1)==9);CHECK(*next_pad(0,1,1)==2);CHECK(*next_pad(0,9,-1)==8);
 CHECK(*destination_mask(0)==15);CHECK(*destination_mask(1)==13);CHECK(*destination_mask(2)==10);
 Scale tie{true,std::uint16_t((1<<11)|(1<<1))};CHECK(*transform_note(60,tie)==59);CHECK(*transform_note(-3)==0);CHECK(*transform_note(300)==127);
 std::set<float> reached;
 for(int i=0;i<=10000;++i){float k=float(i)/10000.f;auto q=rate_knob(k);CHECK(q.has_value());auto d=duration(*q);CHECK(d.has_value());reached.insert(*d);}
 CHECK(reached.size()==13);CHECK(!reached.count(8.f));
 State s;s.settings.division=1.f-6.f/17.f;s.settings.running=1;s.settings.gate=.75f;s.step=15;s.next_step=2;s.next_gate_end=1;s.previous_beat=1;s.transport_change=false;s.held={60};s.generated={57};
 auto out=process(s,{480,48000,10,true});CHECK(out.status==Status::ok&&out.admitted_step);CHECK(out.events.size()==2);CHECK(out.events[0].note==57&&out.events[0].velocity==0);CHECK(out.events[1].note==60&&out.events[1].velocity==87);CHECK(out.next.next_step==2.5f&&out.next.next_gate_end==2.375f&&out.next.previous_step==2&&out.next.step==0);CHECK(out.next.previous_beat==10);CHECK(s.generated==std::vector<std::uint8_t>{57}&&s.next_step==2);
 // No catch-up loop despite beat10. No ad-hoc immediate gate-zero off.
 State zero;zero.settings.running=1;zero.settings.division=1.f-6.f/17.f;zero.settings.gate=0;zero.settings.extra_octaves=0;zero.held={60};auto z=process(zero,{0,48000,0,false});CHECK(z.status==Status::ok&&z.events.size()==1&&z.events[0].velocity==87);auto zz=process(z.next,{0,48000,0,false});CHECK(zz.status==Status::ok&&!zz.admitted_step&&zz.events.size()==1&&zz.events[0].velocity==0);
 State seq;seq.settings.arp_on=false;seq.settings.seq_on=true;seq.settings.running=1;seq.settings.offset[0]=-3.9f;seq.held={60,64};auto q=process(seq,{16,48000,0,false});CHECK(q.events.size()==2&&q.events[0].note==57&&q.events[1].note==61&&q.next.generated==std::vector<std::uint8_t>({57,61}));
 seq.settings.enabled[0]=.999f;q=process(seq,{16,48000,0,false});CHECK(q.events.empty()&&q.admitted_step&&q.next.step==1);
 // Rewind flushes actual generated pitches, never a fresh transform of held.
 s.generated={52,71};s.settings.running=1;s.settings.gate=1;s.next_gate_end=20;s.previous_beat=5;s.next_step=6;auto rw=process(s,{16,48000,.2f,true});CHECK(rw.status==Status::ok&&rw.events.size()==2&&rw.events[0].note==52&&rw.events[1].note==71&&rw.next.next_step==.5f&&!rw.admitted_step);
 s.settings.running=0;s.previous_beat=0;s.next_gate_end=0;auto stop=process(s,{0,48000,1,false});CHECK(stop.status==Status::ok&&stop.events.size()==2&&stop.next.previous_beat==0);
 for(int m=4;m<=6;++m){State r;r.settings.running=1;r.settings.arp_type=m;r.held={60};auto x=process(r,{0,48000,0,false});CHECK(x.status==Status::unresolved_random&&x.events.empty()&&x.next.transport_change==r.transport_change&&x.next.generated==r.generated&&x.next.step==r.step);}
 for(float bad:{std::numeric_limits<float>::quiet_NaN(),std::numeric_limits<float>::infinity(),-std::numeric_limits<float>::infinity(),std::numeric_limits<float>::max()}){
  CHECK(!duration(bad));CHECK(!snap(bad,.5f,0));auto x=process(s,{16,bad,0,false});
  if(std::isfinite(bad))CHECK(x.status==Status::ok);
  else CHECK(x.status==Status::invalid_input&&x.events.empty()&&x.next.generated==s.generated);
 }
 CHECK(!rate_knob(-.01f));CHECK(!rate_knob(1.01f));CHECK(!pad_targets(-1));CHECK(!pad_targets(10));CHECK(!destination_mask(3));CHECK(!next_pad(0,10,1));CHECK(!sequence_notes({60},25,1));
 for(int i=0;i<1000;++i){State t;t.settings.running=1;t.transport_change=false;t.previous_beat=0;t.next_step=100;t.next_gate_end=100;t.generated={60};const auto x=process(t,{std::uint32_t(i),48000,0,false});CHECK(x.status==Status::ok&&!x.admitted_step&&x.events.empty()&&x.next.generated==t.generated);}
 std::printf("PASS %u checks: static-derived deterministic Saga reference only; no original execution, full host, random, audio, native or device acceptance\n",checks);
}
