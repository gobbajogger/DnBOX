#pragma once
#include <cmath>
#include <cstdint>

namespace fenced_s1_clock {

// Caller-owned AudioKit clock only, off-product. Calls serialized with render.
// Sample rate is fixed by the rendering owner; no relationship to project/MIDI BPM.
class Clock {
    static constexpr unsigned units[19]={
        1536,1152,768,576,384,192,128,96,64,48,32,24,16,12,8,6,4,3,2
    };
    double sample_rate, next_delta, phase=0;
    float bpm=120, factor=1;
    unsigned previous_held=0;
    unsigned stop=5;
    void delta() { next_delta=(bpm/60.f)/sample_rate; }

public:
    struct Tick { bool reset, trigger; int index; };
    explicit Clock(double sr):sample_rate(sr) { delta(); }

    bool set_bpm(float v) {
        if (!(v>=1 && v<=200)) return false;
        bpm=v; delta(); return true;
    }
    bool set_stop(unsigned i) {
        if(i>=sizeof units/sizeof *units) return false;
        stop=i; factor=units[i]/192.f; return true;
    }
    float rate() const { return bpm; }
    unsigned factor_stop() const { return stop; }
    float multiplier() const { return factor; }
    double time() const { return phase; }

    Tick sample(bool on, unsigned held, bool pending) {
        const bool first=previous_held==0 && held>0;
        const bool reset=on && (first || (previous_held && !held));
        if(reset) phase=0;
        Tick r{reset,false,0};
        // In the source, reset clears pending notes before this conditional.
        if(on || (pending && !reset)) {
            const double r0=std::fmod(phase,factor);
            phase+=next_delta;
            const double r1=std::fmod(phase,factor);
            r.trigger=first || r1<r0;
            if(r.trigger && on && held) {
                const double i=phase/factor;
                // Off-product receiver guards C++ conversion beyond source domain.
                if(i>=2147483648.) r.trigger=false;
                else r.index=static_cast<int>(i);
            }
        }
        previous_held=held;
        return r;
    }
};
}
