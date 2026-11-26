// Original source sequencing only; original full-graph callback counts validate it in run-bank.py.
#include "original_sequence.hpp"
#include "stimulus.h"
#include <cassert>
#include <cstdio>
#include <cstdlib>
int main(int argc,char **argv)
{
    assert(argc==5);
    const unsigned index=(unsigned)std::strtoul(argv[2],nullptr,10);
    auto input=std::fopen(argv[1],"rt");assert(input);
    auto out=std::fopen(argv[3],"wt");assert(out);
    using namespace s1source_oracle;
    DSPParameters p{};
    unsigned i, n=0; float f;
    while(std::fscanf(input,"%u %a",&i,&f)==2){assert(i<p.size());p[i]=f;++n;}
    assert(n==56&&std::feof(input));std::fclose(input);
    unsigned frame=0;
    const auto emit=[&](int key,int v){if(key>=0&&key<128)std::fprintf(out,"%u %d %d\n",frame,key,v);};
    OriginalSequence seq(emit,[&](int key){emit(key,0);},[]{});
    seq.setSampleRate(S1AB_RATE);
    seq.init();
    seq.setNotesPerOctave(std::atoi(argv[4]));
    seq.reset(false);
    s1source_sequence::HeldNote held[1]{};
    size_t len=0;
    const bool active=p[62]==1.f;
    std::fprintf(out,"S1T1 %u %u %u %.9g\n",index,S1AB_LEAD+S1AB_FRAMES,unsigned(active),p[65]);
    for(frame=0;frame<S1AB_LEAD+S1AB_FRAMES;++frame){
        const unsigned cue=frame>=S1AB_LEAD?frame-S1AB_LEAD:UINT32_MAX;
        if(cue<=S1AB_NOTES*S1AB_NOTE_LEN && cue%S1AB_NOTE_LEN==0){
            i=cue/S1AB_NOTE_LEN;len=i<S1AB_NOTES;
            if(len)held[0]={(int)s1ab_note[i],0,(int)s1ab_vel(i)};
        }
        seq.process(p,{held,len});
    }
    return std::fclose(out);
}
