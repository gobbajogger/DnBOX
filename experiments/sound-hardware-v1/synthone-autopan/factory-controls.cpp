// SPDX-License-Identifier: GPL-3.0-only
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <string>
using std::abs;
#include "S1Rate.hpp"

/* Read the repository's original Swift-derived load commands. arpRate and
 * tempoSyncToArpRate load before autoPanFrequency in every archived bank.
 * Use the actual frontend's frequency quantizer for the sync branch. */
template <class T> T read_word(FILE *f)
{
    T value;
    assert(std::fread(&value,sizeof value,1,f)==1);
    return value;
}
int main(int argc, char **argv)
{
    assert(argc==2);
    FILE *commands=std::fopen(argv[1],"rb");
    assert(commands && read_word<unsigned>(commands)==0x53315043u);
    unsigned programs=read_word<unsigned>(commands);
    assert(programs==694);
    S1Rate original;
    for (unsigned k=0; k<programs; ++k) {
        unsigned n=read_word<unsigned>(commands), state=0;
        float tempo=120.f, rate=.25f, depth=0.f, sync=1.f;
        for (unsigned i=0; i<n; ++i) {
            unsigned id=read_word<unsigned>(commands);
            float value=read_word<float>(commands);
            switch(id) {
            case 64: assert(state==0); tempo=std::clamp(value,1.f,200.f); ++state; break;
            case 139: assert(state==1); sync=std::clamp(value,0.f,1.f); ++state; break;
            case 36:
                assert(state==2);
                rate=std::clamp(value,1.f/1920.f,10.f);
                if (sync>0.f) rate=original.nearestFrequency(rate,tempo,1.f/1920.f,10.f).value;
                ++state;
                break;
            case 35: assert(state==3); depth=std::clamp(value,0.f,1.f); ++state; break;
            }
        }
        assert(state==4);
        std::printf("%a %a\n",double(depth),double(rate));
    }
    assert(std::fgetc(commands)==EOF);
    std::fclose(commands);
}
