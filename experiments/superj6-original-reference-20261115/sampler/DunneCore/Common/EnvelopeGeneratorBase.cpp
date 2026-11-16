// Copyright AudioKit. All Rights Reserved.

#include "EnvelopeGeneratorBase.h"
#include <cassert>

namespace DunneCore
{
    namespace {
    using original_conductor_v1::f64_add;
    using original_conductor_v1::f64_div;
    using original_conductor_v1::f64_exp;
    using original_conductor_v1::f64_from_i64;
    using original_conductor_v1::f64_gt;
    using original_conductor_v1::f64_le;
    using original_conductor_v1::f64_log;
    using original_conductor_v1::f64_mul;
    using original_conductor_v1::f64_neg;
    using original_conductor_v1::f64_sub;
    const std::uint64_t kOne = 0x3ff0000000000000ULL;
    }

    void ExponentialSegmentGenerator::reset(std::uint64_t initialValue, std::uint64_t targetValue, std::uint64_t tco, int segmentLengthSamples)
    {
        output = segmentLengthSamples > 0 ? initialValue : targetValue;
        target = targetValue;
        isHorizontal = original_conductor_v1::f64_cmp(targetValue, initialValue) == 0;
        isLinear = f64_le(tco, 0);
        isRising = f64_gt(targetValue, initialValue);

        if (isHorizontal)
        {
            tcount = 0;
            segLength = segmentLengthSamples;
        }
        else if (isLinear)
        {
            if (segmentLengthSamples <= 0)
                coefficient = f64_sub(target, output);
            else
                coefficient = f64_div(f64_sub(targetValue, initialValue), f64_from_i64(segmentLengthSamples));
        }
        else
        {
            if (segmentLengthSamples == 0)
            {
                coefficient = 0;
                offset = target;
            }
            else
            {
                // Same formula as the double envelope: exp(-log((delta+tco)/tco) / length).
                std::uint64_t delta = f64_sub(targetValue, initialValue);
                if (delta >> 63) delta ^= 0x8000000000000000ULL;
                const std::uint64_t ratio = f64_div(f64_add(delta, tco), tco);
                coefficient = f64_exp(f64_neg(f64_div(f64_log(ratio), f64_from_i64(segmentLengthSamples))));
                const std::uint64_t span = f64_sub(kOne, coefficient);
                if (isRising)
                    offset = f64_mul(f64_add(target, tco), span);
                else
                    offset = f64_mul(f64_sub(target, tco), span);
            }
        }
    }

    void MultiSegmentEnvelopeGenerator::setupCurSeg()
    {
        SegmentDescriptor seg = (*segments)[curSegIndex];
        ExponentialSegmentGenerator::reset(seg.initialValue, seg.finalValue, seg.tco, seg.lengthSamples);
    }

    void MultiSegmentEnvelopeGenerator::setupCurSeg(std::uint64_t initValue)
    {
        SegmentDescriptor seg = (*segments)[curSegIndex];
        std::uint64_t targetValue = seg.finalValue;
        bool isHorizontal = original_conductor_v1::f64_cmp(seg.initialValue, seg.finalValue) == 0;
        if (isHorizontal) { // if flat (hold) then use same value, prevents fades from currentVal to hold val
            targetValue = initValue;
        }
        ExponentialSegmentGenerator::reset(initValue, targetValue, seg.tco, seg.lengthSamples);
    }

    void MultiSegmentEnvelopeGenerator::reset(Descriptor* pDesc, int initialSegmentIndex)
    {
        segments = pDesc;
        curSegIndex = initialSegmentIndex;
        setupCurSeg();
    }

    void MultiSegmentEnvelopeGenerator::startAtSegment(int segIndex) //puts the envelope in a 'fresh' state, allows sudden jumps to first segment
    {
        curSegIndex = segIndex;
        if (skipEmptySegments()) {
            SegmentDescriptor& seg = (*segments)[curSegIndex];
            setupCurSeg(seg.initialValue); // we are restarting, not advancing, so  always start from the first value we get to
        };
    }

    void MultiSegmentEnvelopeGenerator::advanceToSegment(int segIndex) //advances w/ awareness of state, so as to not make sudden jumps
    {
        curSegIndex = segIndex;
        if (skipEmptySegments()) {
            setupCurSeg(output); //we are advancing, not restarting, so always start from the value we are currently at
        };
    }

    bool MultiSegmentEnvelopeGenerator::skipEmptySegments() //skips over any segment w/ length 0, so as to not influence the state of the envelope
    {
        assert(segments);

        // skip any segments that are 0-length
        while(curSegIndex < segments->size()
              && (*segments)[curSegIndex].lengthSamples == 0) {
            curSegIndex++;
        }

        // if at end of the envelope, reset
        if(curSegIndex == segments->size()) {
            reset(segments);
            return false;
        }

        return true;

    }

}
