// SPDX-License-Identifier: GPL-3.0-only
// Offline mono IMA encoder. The firmware consumes the usual low-nibble-first stream.
#include <algorithm>
#include <array>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <vector>
#define X0X_IMA_TABLES_DEFINE
#include "ima_adpcm.h"

constexpr unsigned width = 32;
struct State {
    int predictor, index;
    std::int64_t error, estimate;
    unsigned parent, code;
};
struct Trace { std::uint8_t parent, code; };

static std::int64_t squared(int difference)
{
    return std::int64_t(difference) * difference;
}

static std::vector<std::uint8_t> encode(const std::vector<int> &pcm)
{
    int delta[89][16], next_index[89][16];
    for (int i=0; i<89; ++i) {
        int step=IMA_STEP_T[i];
        for (unsigned code=0; code<16; ++code) {
            int amount=(step>>3) + (code&4 ? step : 0) +
                       (code&2 ? step>>1 : 0) + (code&1 ? step>>2 : 0);
            delta[i][code]=code&8 ? -amount : amount;
            next_index[i][code]=std::clamp(i+IMA_IDX_T[code&7],0,88);
        }
    }
    std::vector<std::array<Trace,width>> history(pcm.size());
    std::array<State,width> beam{};
    std::array<State,width*16> choices;
    unsigned active=1;
    for (std::size_t frame=0; frame<pcm.size(); ++frame) {
        unsigned count=0;
        for (unsigned parent=0; parent<active; ++parent) {
            const State &base=beam[parent];
            for (unsigned code=0; code<16; ++code) {
                int pred=std::clamp(base.predictor+delta[base.index][code],-32768,32767);
                int idx=next_index[base.index][code];
                auto error=base.error+squared(pred-pcm[frame]);
                // Lower bound on the next error if the target lies beyond the largest code.
                int overshoot=0;
                if (frame+1<pcm.size())
                    overshoot=std::max(0,std::abs(pcm[frame+1]-pred)-delta[idx][7]);
                choices[count++]={pred,idx,error,error+squared(overshoot),parent,code};
            }
        }
        const auto better=[](const State &a,const State &b) {
            if (a.estimate!=b.estimate) return a.estimate<b.estimate;
            if (a.error!=b.error) return a.error<b.error;
            if (a.parent!=b.parent) return a.parent<b.parent;
            return a.code<b.code;
        };
        std::sort(choices.begin(),choices.begin()+count,better);
        active=0;
        for (unsigned i=0; i<count && active<width; ++i) {
            const auto &s=choices[i];
            bool repeated=false;
            for (unsigned j=0; j<active; ++j)
                if (beam[j].predictor==s.predictor && beam[j].index==s.index) {
                    repeated=true;
                    break;
                }
            if (repeated) continue;
            beam[active]=s;
            history[frame][active]={std::uint8_t(s.parent),std::uint8_t(s.code)};
            ++active;
        }
    }
    std::vector<std::uint8_t> output((pcm.size()+1)/2);
    unsigned index=0;
    for (std::size_t i=pcm.size(); i>0; --i) {
        const auto trace=history[i-1][index];
        output[(i-1)/2] |= trace.code << (4*((i-1)%2));
        index=trace.parent;
    }
    return output;
}

int main()
{
    std::vector<int> samples;
    for (;;) {
        int low=std::getc(stdin);
        if (low==EOF) break;
        int high=std::getc(stdin);
        if (high==EOF) return 2;
        const int word=low+(high<<8);
        samples.push_back(word>=32768 ? word-65536 : word);
    }
    if (std::ferror(stdin) || samples.empty()) return 2;
    auto encoded=encode(samples);
    return std::fwrite(encoded.data(),1,encoded.size(),stdout)==encoded.size() ? 0 : 1;
}
