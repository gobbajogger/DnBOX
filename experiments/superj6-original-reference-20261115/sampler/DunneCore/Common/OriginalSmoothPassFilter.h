// Original-compatible behavioral implementation; qualified scope in README.md.
// MIT sampler integration preserves AudioKit/Dunne source credits in LICENSE.
#pragma once
#include <cmath>
#include <algorithm>
#include "FunctionTable.h"
namespace DunneCore {
struct OriginalSmoothPassFilter {
    double c[5]{}, target[5]{}, smoothing{}, x1{}, x2{}, y1{}, y2{};
    bool first{true};
    double rate{44100}, cutoff{20000}, resonance{.1};
    OriginalSmoothPassFilter() { init(44100); }
    static FunctionTable& sine() {
        static FunctionTable t;
        if (t.waveTable.empty()) { t.init(2048); t.sinusoid(); }
        return t;
    }
    void calculate(double frequency, double res, double* out) {
        frequency=std::max(12.,frequency);
        double normalized=(frequency+frequency)/rate;
        normalized=std::min(.99,normalized);
        float phase=float(.5*normalized);
        double k=std::pow(std::min(20.,std::max(0.,res)),1.91)*sine().interp_cyclic(phase);
        double u=.5*(1.-k)/(1.+k);
        double v=.5+u;
        double w=v*sine().interp_cyclic(phase+.25f);
        double z=(v-w)*.25;
        out[0]=z+z;
        out[1]=z*4.;
        out[2]=z+z;
        out[3]=-2.*w;
        out[4]=u+u;
    }
    void init(double sampleRate) {
        rate=sampleRate;
        smoothing=1.-std::exp(-94.24777960769379/rate);
        x1=x2=y1=y2=0.; first=true;
        cutoff=20000.;resonance=.1;
        calculate(cutoff,resonance,c);
        for(int i=0;i<5;++i)target[i]=c[i];
    }
    // Original-compatible rate update retains histories/current/first and updates targets.
    void updateSampleRate(double sampleRate) {
        rate=sampleRate;
        smoothing=1.-std::exp(-94.24777960769379/rate);
        setParameters(cutoff,resonance);
    }
    void setParameters(double frequency,double res) {
        cutoff=std::max(12.,frequency);
        resonance=std::min(20.,std::max(0.,res));
        calculate(cutoff,resonance,target);
    }
    float process(float input) {
        if(first) { for(int i=0;i<5;++i)c[i]=target[i];first=false; }
        for(int i=0;i<5;++i)c[i]=std::fma(smoothing,target[i]-c[i],c[i]);
        double out=std::fma(-c[4],y2,std::fma(-c[3],y1,std::fma(c[2],x2,std::fma(c[0],double(input),c[1]*x1))));
        float result=float(out);
        x2=x1;x1=input;y2=y1;y1=result;
        return result;
    }
};
}
