// Copyright AudioKit. All Rights Reserved.

// ResonantLowPassFilter implements a simple digital low-pass filter with dynamically
// adjustable cutoff frequency and resonance.
//
// Filter resonance is usually expressed in dB, but to avoid having to call expensive
// math functions like pow(), we use a linear value between 10.0f (-20 dB) and 0.1f (+20 dB)

#pragma once
#include <cmath>

namespace DunneCore
{

    struct ResonantLowPassFilter
    {
        // coefficients
        float a0, a1, a2, b1, b2;
        
        // state
        float x1, x2, y1, y2;
        
        // misc
        float sampleRateHz, mLastCutoffHz, mLastResLinear;
        
        ResonantLowPassFilter();
        
        void init(float sampleRateHz);
        void updateSampleRate(float sampleRate) { sampleRateHz = sampleRate; }
        
        void setParameters(float newCutoffHz, float newResLinear);
        void setCutoff(float newCutoffHz) { setParameters(newCutoffHz, mLastResLinear); }
        void setResonance(float newResLinear) { setParameters(mLastCutoffHz, newResLinear); }
        
        void process(const float *inSourceP, float *inDestP, int inFramesToProcess);

        inline float process(float inputSample)
        {
            float outputSample = (float)(std::fma(-b2, y2, std::fma(-b1, y1, std::fma(a2, x2, std::fma(a0, float(inputSample), a1*x1)))));

            x2 = x1;
            x1 = inputSample;
            y2 = y1;
            y1 = outputSample;

            return outputSample;
        }

    };

}
