// Copyright AudioKit. All Rights Reserved.

#include "EnvelopeGeneratorBase.h"
#include <cmath>
#include <cassert>

namespace DunneCore
{

    void ExponentialSegmentGenerator::reset(float initialValue, float targetValue, float tco, int segmentLengthSamples)
    {
        output = segmentLengthSamples > 0 ? initialValue : targetValue;
        target = targetValue;
        isHorizontal = targetValue == initialValue;
        isLinear = tco <= 0.0f;
        isRising = targetValue > initialValue;

        if (isHorizontal)
        {
            tcount = 0;
            segLength = segmentLengthSamples;
        }
        else if (isLinear)
        {
            if (segmentLengthSamples <= 0)
                coefficient = target - output;
            else
                coefficient = (targetValue - initialValue) / segmentLengthSamples;
        }
        else
        {
            if (segmentLengthSamples == 0)
            {
                coefficient = 0.0f;
                offset = target;
            }
            else
            {
                // Correction to Pirkle (who uses delta = 1.0f always)
                // According to Redmon (who only discusses the delta = 1.0f case), delta should be defined thus
                float delta = std::abs(targetValue - initialValue);
                coefficient = std::exp(-std::log((delta + tco) / tco) / segmentLengthSamples);
                if (isRising)
                    offset = (target + tco) * (1.0f - coefficient);
                else
                    offset = (target - tco) * (1.0f - coefficient);
            }
        }
    }

    void MultiSegmentEnvelopeGenerator::setupCurSeg()
    {
        SegmentDescriptor seg = (*segments)[curSegIndex];
        ExponentialSegmentGenerator::reset(seg.initialValue, seg.finalValue, seg.tco, seg.lengthSamples);
    }

    void MultiSegmentEnvelopeGenerator::setupCurSeg(float initValue)
    {
        SegmentDescriptor seg = (*segments)[curSegIndex];
        float targetValue = seg.finalValue;
        bool isHorizontal = seg.initialValue == seg.finalValue;
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
