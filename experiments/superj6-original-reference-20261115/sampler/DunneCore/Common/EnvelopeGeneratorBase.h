// Copyright AudioKit. All Rights Reserved.

#pragma once
#include <cstdint>
#include <vector>
#include <cmath>
#include "f64_bits.hpp"

namespace DunneCore
{

    // Iterative-exponential envelope generator, as described by Will Pirkle's synthesizer book
    // (Designing Software Synthesizer Plug-Ins in C++, Focal Press, 2014, ISBN 978-1-138-78707-0)
    // and Nigel Redmon (http://www.earlevel.com/main/2013/06/02/envelope-generators-adsr-part-2/).

    // Additions/adaptations by Shane Dunne to support
    // * arbitrary number of segments, not just "ADSR"
    // * horizontal segments, both fixed-length ("hold time") and indefinite length ("sustain")
    // * linear segments (e.g. for fast shutoff)

    class ExponentialSegmentGenerator
    {
    public:
        void reset(std::uint64_t initialValue, std::uint64_t targetValue, std::uint64_t tco, int segmentLengthSamples);

        inline float getValue()
        {
            return original_conductor_v1::f64_to_f32(isHorizontal ? target : output);
        }

        inline bool getSample(float& out)
        {
            if (isHorizontal)
            {
                out = original_conductor_v1::f64_to_f32(target);
                if (segLength < 0) return false;        // non-timed "sustain"segment
                else return (++tcount >= segLength);    // timed "hold" segment
            }
            else
            {
                if (isLinear)
                    output = original_conductor_v1::f64_add(output, coefficient);
                else
                    output = original_conductor_v1::f64_fma(coefficient, output, offset);
                bool overshoot = isRising ? original_conductor_v1::f64_ge(output, target) : original_conductor_v1::f64_le(output, target);
                if (overshoot) output = target;
                out = original_conductor_v1::f64_to_f32(output);
                return overshoot;
            }
        }

    protected:
        // Binary64 bits. The recurrence matches the original double envelope.
        std::uint64_t output, target, offset, coefficient;
        bool isRising;
        bool isHorizontal;
        int tcount, segLength;
        bool isLinear;
    };

    class MultiSegmentEnvelopeGenerator : public ExponentialSegmentGenerator
    {
    public:
        struct SegmentDescriptor
        {
            std::uint64_t initialValue;
            std::uint64_t finalValue;
            std::uint64_t tco;
            int lengthSamples;
        };
        typedef std::vector<SegmentDescriptor> Descriptor;

        void reset(Descriptor* pDesc, int initialSegmentIndex = 0);
        void advanceToSegment(int segIndex);
        void startAtSegment(int segIndex);

        inline bool getSample(float& out)
        {            
            if (ExponentialSegmentGenerator::getSample(out))
            {
                if (++curSegIndex >= int(segments->size()))
                {
                    reset(segments);
                    return true;
                }
                else
                {
                    setupCurSeg();
                }
            }
            return false;
        }

        int getCurrentSegmentIndex() { return curSegIndex; }

    protected:
        Descriptor* segments = nullptr;
        int curSegIndex;

        void setupCurSeg();
        void setupCurSeg(std::uint64_t initValue);
        bool skipEmptySegments();
    };

}
