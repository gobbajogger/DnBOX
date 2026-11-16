// Original-compatible behavioral implementation; qualified scope in README.md.
// MIT sampler integration preserves AudioKit/Dunne source credits in LICENSE.
#pragma once
#include <cmath>
#include <algorithm>
#include "FunctionTable.h"
namespace DunneCore {
struct OriginalSmoothPassFilter {
    float c[5]{}, target[5]{}, smoothing{}, x1{}, x2{}, y1{}, y2{};
    bool first{true};
    float rate{44100}, cutoff{20000}, resonance{.1f};
    OriginalSmoothPassFilter() { init(44100); }
    static FunctionTable& sine() {
        static FunctionTable t;
        if (t.waveTable.empty()) { t.init(2048); t.sinusoid(); }
        return t;
    }
    void calculate(float frequency, float res, float* out) {
        frequency=std::max(12.f,frequency);
        float normalized=(frequency+frequency)/rate;
        normalized=std::min(.99f,normalized);
        float phase=float(.5f*normalized);
        float k=std::pow(std::min(20.f,std::max(0.f,res)),1.91f)*sine().interp_cyclic(phase);
        float u=.5f*(1.f-k)/(1.f+k);
        float v=.5f+u;
        float w=v*sine().interp_cyclic(phase+.25f);
        float z=(v-w)*.25f;
        out[0]=z+z;
        out[1]=z*4.f;
        out[2]=z+z;
        out[3]=-2.f*w;
        out[4]=u+u;
    }
    void init(float sampleRate) {
        rate=sampleRate;
        smoothing=1.f-std::exp(-94.24777960769379f/rate);
        x1=x2=y1=y2=0.f; first=true;
        cutoff=20000.f;resonance=.1f;
        calculate(cutoff,resonance,c);
        for(int i=0;i<5;++i)target[i]=c[i];
    }
    // Original-compatible rate update retains histories/current/first and updates targets.
    void updateSampleRate(float sampleRate) {
        rate=sampleRate;
        smoothing=1.f-std::exp(-94.24777960769379f/rate);
        setParameters(cutoff,resonance);
    }
    void setParameters(float frequency,float res) {
        cutoff=std::max(12.f,frequency);
        resonance=std::min(20.f,std::max(0.f,res));
        calculate(cutoff,resonance,target);
    }
    float process(float input) {
        if(first) { for(int i=0;i<5;++i)c[i]=target[i];first=false; }
        for(int i=0;i<5;++i)c[i]=std::fma(smoothing,target[i]-c[i],c[i]);
        float out=std::fma(-c[4],y2,std::fma(-c[3],y1,std::fma(c[2],x2,std::fma(c[0],float(input),c[1]*x1))));
        float result=float(out);
        x2=x1;x1=input;y2=y1;y1=result;
        return result;
    }
};
}
