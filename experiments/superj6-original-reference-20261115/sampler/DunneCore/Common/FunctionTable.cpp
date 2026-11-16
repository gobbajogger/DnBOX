// Copyright AudioKit. All Rights Reserved.

#include "FunctionTable.h"
#ifndef _USE_MATH_DEFINES
  #define _USE_MATH_DEFINES
#endif
#include <math.h>
#include <cmath>

namespace DunneCore
{

    void FunctionTable::init(int tableLength)
    {
        waveTable.resize(tableLength);
    }
    
    void FunctionTable::deinit()
    {
        waveTable.clear();
    }
    
    void FunctionTable::triangle(float amplitude)
    {
        // in case user forgot, init table to size 2
        if (waveTable.empty()) init(2);
        
        if (waveTable.size() == 2)   // default 2 elements suffice for a triangle wave
        {
            waveTable[0] = -amplitude;
            waveTable[1] = amplitude;
        }
        else    // you would normally only do this if you plan to low-pass filter the result
        {
            auto nTableSize = waveTable.size();
            for (int i=0; i < nTableSize; i++)
                waveTable[i] = 2.0f * amplitude * (0.25f - fabsf((float(i)/nTableSize) - 0.5f));
        }
    }
    
    void FunctionTable::sawtooth(float amplitude)
    {
        // in case user forgot, init table to default size
        if (waveTable.empty()) init();

        auto nTableSize = waveTable.size();
        for (int i=0; i < nTableSize; i++)
            waveTable[i] = (float)(2.0f * amplitude * float(i)/nTableSize - amplitude);
    }
    
    void FunctionTable::sinusoid(float amplitude)
    {
        // in case user forgot, init table to default size
        if (waveTable.empty()) init();

        auto nTableSize = waveTable.size();
        for (int i=0; i < nTableSize; i++) {
            const std::uint64_t turns = original_conductor_v1::f64_mul(
                original_conductor_v1::f64_mul(original_conductor_v1::f64_div(original_conductor_v1::f64_from_i64(i), original_conductor_v1::f64_from_i64(std::int64_t(nTableSize))), 0x4000000000000000ULL),
                0x400921fb54442d18ULL);
            waveTable[i] = original_conductor_v1::f64_to_f32(original_conductor_v1::f64_mul(original_conductor_v1::f64_from_f32(amplitude), original_conductor_v1::f64_sin(turns)));
        }
    }

    // A variation of sinusoid() which adds a tiny bit of 2nd harmonic, producing a tone closer to
    // that of a Hammond organ tonewheel generator.
    void DunneCore::FunctionTable::hammond(float amplitude)
    {
        // in case user forgot, init table to default size
        if (waveTable.empty()) init();

        auto nTableSize = waveTable.size();
        for (int i = 0; i < nTableSize; i++) {
            const std::uint64_t base = original_conductor_v1::f64_div(original_conductor_v1::f64_from_i64(i), original_conductor_v1::f64_from_i64(std::int64_t(nTableSize)));
            const std::uint64_t fundamental = original_conductor_v1::f64_sin(original_conductor_v1::f64_mul(original_conductor_v1::f64_mul(base, 0x4000000000000000ULL), 0x400921fb54442d18ULL));
            const std::uint64_t second = original_conductor_v1::f64_sin(original_conductor_v1::f64_mul(original_conductor_v1::f64_mul(base, 0x4010000000000000ULL), 0x400921fb54442d18ULL));
            const std::uint64_t shaped = original_conductor_v1::f64_add(fundamental, original_conductor_v1::f64_mul(original_conductor_v1::f64_from_f32(0.015f), second));
            waveTable[i] = original_conductor_v1::f64_to_f32(original_conductor_v1::f64_mul(original_conductor_v1::f64_from_f32(amplitude), shaped));
        }
    }

    void FunctionTable::square(float amplitude, float dutyCycle)
    {
        // in case user forgot, init table to default size
        if (waveTable.empty()) init();

        auto nTableSize = waveTable.size();
        float dcOffset = amplitude * (2.0f * dutyCycle - 1.0f);
        for (int i=0; i < nTableSize; i++)
        {
            float phase = (float)i / nTableSize;
            waveTable[i] = (phase < dutyCycle ? amplitude : -amplitude) - dcOffset;
        }
    }

    void FunctionTable::linearCurve(float gain)
    {
        // in case user forgot, init table to default size
        if (waveTable.empty()) init();

        auto nTableSize = waveTable.size();

        for (int i = 0; i < nTableSize; i++)
            waveTable[i] = gain * i / float(nTableSize);
    }
    
    // Initialize a FunctionTable to an exponential shape, scaled to fit in the unit square.
    // The function itself is y = -exp(-x), where x ranges from 'left' to 'right'.
    // The more negative 'left' is, the more vertical the start of the rise; -5.0f yields near-vertical.
    // The more positive 'right' is, the more horizontal then end of the rise; +5.0f yields near-horizontal.
    void FunctionTable::exponentialCurve(float left, float right)
    {
        // in case user forgot, init table to default size
        if (waveTable.empty()) init();
        
        float bottom = -expf(-left);
        float top = -expf(-right);
        float vscale = 1.0f / (top - bottom);

        auto nTableSize = waveTable.size();
        
        float x = left;
        float dx = (right - left) / (nTableSize - 1);
        for (int i=0; i < nTableSize; i++, x += dx)
            waveTable[i] = vscale * (-expf(-x) - bottom);
    }

    // Initialize a FunctionTable to a power-curve shape, defined in the unit square.
    // The given exponent may be positive for a concave-up shape or negative for concave-down.
    // Typical range of the exponent is plus or minus 4 or 5.f
    void FunctionTable::powerCurve(float exponent)
    {
        // in case user forgot, init table to default size
        if (waveTable.empty()) init();

        auto nTableSize = waveTable.size();

        float x = 0.0f;
        float dx = 1.0f / (nTableSize - 1);
        for (int i=0; i < nTableSize; i++, x += dx)
            waveTable[i] = powf(x, exponent);
    }

    void FunctionTableOscillator::init(float sampleRate, float frequency, int tableLength)
    {
        waveTable.init(tableLength);
        sampleRateHz = sampleRate;
        phase = 0.0f;
        phaseDelta = original_conductor_v1::f64_to_f32(original_conductor_v1::f64_div(original_conductor_v1::f64_from_f32(frequency), original_conductor_v1::f64_from_f32(sampleRate)));
    }
    
    void FunctionTableOscillator::deinit()
    {
        waveTable.deinit();
    }
    
    void FunctionTableOscillator::setFrequency(float frequency)
    {
        phaseDelta = original_conductor_v1::f64_to_f32(original_conductor_v1::f64_div(original_conductor_v1::f64_from_f32(frequency), original_conductor_v1::f64_from_f32(sampleRateHz)));
    }

    // Initialize WaveShaper's lookup table to an identity
    void WaveShaper::init(int tableLength)
    {
        waveTable.init(tableLength);
        for (int i = 0; i < tableLength; i++)
            waveTable.waveTable[i] = i / float(tableLength);
    }
}

