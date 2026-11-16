// Copyright AudioKit. All Rights Reserved.

#pragma once
#include <cstdint>
#include "f64_bits.hpp"
#include "SampleBuffer.h"

namespace DunneCore
{

    struct SampleOscillator
    {
        bool isLooping;
        // Integer sample plus the binary64 fraction in (-1, 1). The pair is the
        // original double index, so a loop longer than 2^23 frames keeps its fraction.
        std::int64_t indexWhole{0};
        std::uint64_t indexFrac{0};
        std::uint64_t incrementBits{0x3ff0000000000000ULL};
        std::uint64_t multiplierBits{0x3ff0000000000000ULL};

        std::uint64_t position() const {
            return original_conductor_v1::f64_add(original_conductor_v1::f64_from_i64(indexWhole), indexFrac);
        }
        void setPosition(std::uint64_t bits) {
            const std::uint64_t whole = original_conductor_v1::f64_trunc(bits);
            indexWhole = original_conductor_v1::f64_to_i64(whole);
            indexFrac = original_conductor_v1::f64_sub(bits, whole);
        }
        void setIndex(float x) { setPosition(original_conductor_v1::f64_from_f32(x)); }
        void setIncrement(float x) { incrementBits = original_conductor_v1::f64_from_f32(x); }
        void setPitchOffsetSemitones(float semitones) {
            const std::uint64_t semis = original_conductor_v1::f64_from_f32(semitones);
            const std::uint64_t steps = original_conductor_v1::f64_div(semis, original_conductor_v1::f64_from_f32(12.f));
            multiplierBits = original_conductor_v1::f64_pow(0x4000000000000000ULL, steps);
        }
        void advance(SampleBuffer *sampleBuffer) {
            setPosition(original_conductor_v1::f64_fma(multiplierBits, incrementBits, position()));
            if (sampleBuffer->isLooping && isLooping) {
                const std::uint64_t loopEnd = original_conductor_v1::f64_from_f32(sampleBuffer->loopEndPoint);
                if (original_conductor_v1::f64_gt(position(), loopEnd)) {
                    const std::uint64_t loopStart = original_conductor_v1::f64_from_f32(sampleBuffer->loopStartPoint);
                    setPosition(original_conductor_v1::f64_add(original_conductor_v1::f64_sub(position(), loopEnd), loopStart));
                }
            }
        }
        float readAt(SampleBuffer *sampleBuffer, int channelBase, float gain) const {
            const std::uint64_t one = 0x3ff0000000000000ULL;
            const int ri = indexWhole < 0 ? -1 : (indexWhole > 0x7fffffffLL ? 0x7fffffff : int(indexWhole));
            const int rj = ri + 1;
            auto at = [&](int i) {
                return i < 0 || i >= sampleBuffer->sampleCount ? 0.f : sampleBuffer->readValue(unsigned(channelBase + i));
            };
            const float si = at(ri), sj = at(rj);
            const std::uint64_t mixed = original_conductor_v1::f64_add(
                original_conductor_v1::f64_mul(original_conductor_v1::f64_sub(one, indexFrac), original_conductor_v1::f64_from_f32(si)),
                original_conductor_v1::f64_mul(indexFrac, original_conductor_v1::f64_from_f32(sj)));
            return original_conductor_v1::f64_to_f32(original_conductor_v1::f64_mul(original_conductor_v1::f64_from_f32(gain), mixed));
        }
        bool pastEnd(SampleBuffer *sampleBuffer) const {
            return sampleBuffer == nullptr || original_conductor_v1::f64_gt(position(), original_conductor_v1::f64_from_f32(sampleBuffer->endPoint));
        }

        inline bool getSample(SampleBuffer *sampleBuffer, int sampleCount, float *output, float gain)
        {
            (void)sampleCount;
            if (pastEnd(sampleBuffer)) return true;
            *output = readAt(sampleBuffer, 0, gain);
            advance(sampleBuffer);
            return false;
        }

        inline bool getSamplePair(SampleBuffer *sampleBuffer, int sampleCount, float *leftOutput, float *rightOutput, float gain)
        {
            (void)sampleCount;
            if (pastEnd(sampleBuffer)) return true;
            if (sampleBuffer->channelCount == 1) {
                const float s = readAt(sampleBuffer, 0, gain);
                *leftOutput = *rightOutput = s;
            } else {
                *leftOutput = readAt(sampleBuffer, 0, gain);
                *rightOutput = readAt(sampleBuffer, sampleBuffer->sampleCount, gain);
            }
            advance(sampleBuffer);
            return false;
        }
    };

}
