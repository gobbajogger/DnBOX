#include <algorithm>
#include <cassert>
#include <cstdio>
#include <vector>
#include "original_sequence.hpp"
#include "adapted.hpp"
#include "factor_probe.h"

namespace A=s1source_sequence;
struct Event {
    unsigned frame; char action; int note, velocity;
    bool operator==(const Event& b)const {
        return frame==b.frame&&action==b.action&&note==b.note&&velocity==b.velocity;
    }
};
struct Trace {
    unsigned frame=0;
    std::vector<Event> events;
    void on(int k,int v){events.push_back({frame,'+',k,v});}
    void off(int k){events.push_back({frame,'-',k,0});}
    void beat(){events.push_back({frame,'=',0,0});}
};
struct Owner {
    Trace gold, test;
    s1source_oracle::OriginalSequence original;
    fenced_event_adapter::SerializedSequence active;
    s1source_oracle::DSPParameters p{};
    fenced_s1_clock::Clock clock;
    A::HeldNote held[2];
    Owner(double sr,int root):original([this](int k,int v){gold.on(k,v);},
            [this](int k){gold.off(k);},[this](){gold.beat();}),
            clock(sr),held{{root,0,99},{root+7,0,75}} {
        original.setSampleRate(sr); original.setNotesPerOctave(12); original.init();
        active.init();
        p[A::arpIsOn]=p[A::arpIsSequencer]=1;
        p[A::arpTotalSteps]=4;
        p[A::sequencerNoteOn00]=p[A::sequencerNoteOn02]=p[A::sequencerNoteOn03]=1;
        p[A::sequencerPattern01]=5; p[A::sequencerPattern02]=-2; p[A::sequencerPattern03]=3;
        p[A::sequencerOctBoost02]=1;
    }
    void edit(float bpm,unsigned i) {
        assert(clock.set_bpm(bpm)&&clock.set_stop(i));
        p[A::arpRate]=bpm; p[A::arpSeqTempoMultiplier]=s1_factor_units[i]/192.f;
        assert(clock.rate()==bpm&&clock.factor_stop()==i);
    }
    void frame(unsigned i,unsigned n) {
        gold.frame=test.frame=i;
        A::HeldView h{held,n};
        original.process(p,h);
        assert(active.process(&p[0],p.size(),h,test,clock));
    }
    unsigned verify() {
        assert(test.events==gold.events);
        return test.events.size();
    }
};
int main() {
    unsigned sum=0,rests=0,two_keys=0;
    for(unsigned r:{44100u,48000u}) for(unsigned stop=0;stop<19;stop++) {
        Owner a(r,48), b(r,66);
        a.edit(123.4f,stop); b.edit(200,(stop+7)%19);
        const unsigned after=(unsigned)(r*double(a.clock.multiplier())*60/123.4)+32;
        for(unsigned i=0;i<2*after+3000;i++) {
            if(i==after) { a.edit(200,18); b.edit(98.75f,17); }
            if(i==after+400) b.p[A::arpIsOn]=0;
            if(i==after+950) b.p[A::arpIsOn]=1;
            unsigned an=i<49|| (i>=after+1100&&i<after+1150) ?0: (i>after+10?2:1);
            unsigned bn=i<102|| (i>=after+1400&&i<after+1500) ?0:1;
            a.frame(i,an); b.frame(i,bn);
        }
        sum+=a.verify()+b.verify();
        for(size_t j=1;j<a.test.events.size();++j) {
            const auto &q=a.test.events[j],&p=a.test.events[j-1];
            if(q.action=='='&&p.action=='='&&q.frame!=p.frame) ++rests;
            if(q.action=='+'&&p.action=='+'&&q.frame==p.frame&&q.velocity!=p.velocity)++two_keys;
        }
        double held=a.clock.time();
        assert(!a.clock.set_bpm(275)&&!a.clock.set_bpm(0)&&!a.clock.set_bpm(NAN));
        assert(!a.clock.set_stop(19)&&held==a.clock.time()&&a.clock.rate()==200&&a.clock.factor_stop()==18);
    }
    // The slowest original pair actually crosses above the former uint32 Q8 limit.
    Owner slow(44100,52);
    slow.edit(1,0); slow.p[A::sequencerNoteOn01]=1;
    const unsigned until=44100u*60u*8u+32;
    for(unsigned i=0;i<until;++i) slow.frame(i,1);
    sum+=slow.verify();
    std::vector<Event> ons;
    for(auto &e:slow.test.events) if(e.action=='+') ons.push_back(e);
    assert(ons.size()==2&&ons[0].frame==0&&ons[1].frame>21167990&&ons[1].note==57);
    assert(rests&&two_keys);
    printf("PASS %u exact source callbacks, 2 audio rates x all 19 stops with 2 interleaved owners; %u silent step witnesses, %u layered onsets; 1 BPM/factor8 second onset %u, no product activation\n",sum,rests,two_keys,ons[1].frame);
}
