// SPDX-License-Identifier: GPL-3.0-only
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
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
    bool tremolo = argc==3 && !std::strcmp(argv[2],"--tremolo");
    bool phaser=argc==3 && !std::strcmp(argv[2],"--phaser");
    assert(argc==2 || tremolo || phaser);
    FILE *commands=std::fopen(argv[1],"rb");
    assert(commands && read_word<unsigned>(commands)==0x53315043u);
    unsigned programs=read_word<unsigned>(commands);
    assert(programs==694);
    S1Rate original;
    for (unsigned k=0; k<programs; ++k) {
        unsigned n=read_word<unsigned>(commands), state=0;
        float tempo=120.f, rate=.25f, depth=0.f, sync=1.f;
        float lfo_rate[2]={}, level[2]={}, shape[2]={}, route=0.f;
        unsigned lfo_mask=0, phaser_mask=0;
        float phase_controls[4]={0.f,12.f,0.f,800.f};
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
            case 15: case 47: {
                assert(state==2);
                unsigned source=id==47;
                float hz=std::clamp(value,1.f/1920.f,80.f);
                lfo_rate[source]=sync>0.f ? original.nearestFrequency(hz,tempo,1.f/1920.f,80.f).value : hz;
                lfo_mask|=1u<<source;
                break;
            }
            case 13: case 45: shape[id==45]=value; lfo_mask|=4u<<(id==45); break;
            case 14: case 46: level[id==46]=value; lfo_mask|=16u<<(id==46); break;
            case 59: route=value; lfo_mask|=64u; break;
            case 116: case 117: case 118: case 119: {
                static const float min[4]={0,1,0,100}, max[4]={1,300,.8f,1000};
                unsigned slot=id-116;
                phase_controls[slot]=std::clamp(value,min[slot],max[slot]);
                phaser_mask|=1u<<slot; break;
            }
            }
        }
        assert(state==4);
        if (phaser) {
            assert(phaser_mask==15);
            std::printf("%a %a %a %a\n",double(phase_controls[0]),double(phase_controls[1]),
                        double(phase_controls[2]),double(phase_controls[3]));
        } else if (tremolo) {
            assert(lfo_mask==127 && route>=0 && route<=3 && std::trunc(route)==route);
            std::printf("%u %u %a %a %u %a %a\n",unsigned(route),unsigned(shape[0]),double(level[0]),
                        double(lfo_rate[0]),unsigned(shape[1]),double(level[1]),double(lfo_rate[1]));
        } else std::printf("%a %a\n",double(depth),double(rate));
    }
    assert(std::fgetc(commands)==EOF);
    std::fclose(commands);
}
