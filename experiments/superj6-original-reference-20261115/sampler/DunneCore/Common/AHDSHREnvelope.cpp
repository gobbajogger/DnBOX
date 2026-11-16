// Copyright AudioKit. All Rights Reserved.

#include "AHDSHREnvelope.h"

namespace DunneCore
{
    static std::uint64_t W(float x) { return original_conductor_v1::f64_from_f32(x); }
    static std::uint64_t E(std::uint64_t x) { return original_conductor_v1::f64_exp(x); }

    AHDSHREnvelopeParameters::AHDSHREnvelopeParameters()
        : sampleRateHz(44100.0f) // a guess, will be overridden later by a call to init(,,,,)
    {
        init(0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);
    }

    void AHDSHREnvelopeParameters::init(float attackSeconds, float holdSeconds, float decaySeconds,
                                        float susFraction, float releaseHoldSeconds, float releaseSeconds)
    {
        attackSamples = attackSeconds * sampleRateHz;
        holdSamples = holdSeconds * sampleRateHz;
        decaySamples = decaySeconds * sampleRateHz;
        sustainFraction = susFraction;
        releaseHoldSamples = releaseHoldSeconds * sampleRateHz;
        releaseSamples = releaseSeconds * sampleRateHz;
    }

    void AHDSHREnvelopeParameters::init(float newSampleRateHz, float attackSeconds, float holdSeconds, float decaySeconds,
                                        float susFraction, float releaseHoldSeconds, float releaseSeconds)
    {
        sampleRateHz = newSampleRateHz;
        init(attackSeconds, holdSeconds, decaySeconds, susFraction, releaseHoldSeconds, releaseSeconds);
    }

    void AHDSHREnvelopeParameters::updateSampleRate(float newSampleRateHz)
    {
        float scaleFactor = newSampleRateHz / sampleRateHz;
        sampleRateHz = newSampleRateHz;
        attackSamples *= scaleFactor;
        holdSamples *= scaleFactor;
        decaySamples *= scaleFactor;
        releaseHoldSamples *= scaleFactor;
        releaseSamples *= scaleFactor;
    }


    void AHDSHREnvelope::init(CurvatureType curvatureType)
    {
        int silenceSamples = int(0.01f * pParameters->sampleRateHz);     // always 10 mSec
        int attackSamples = int(pParameters->attackSamples);
        int holdSamples = int(pParameters->holdSamples);
        int decaySamples = int(pParameters->decaySamples);
        const std::uint64_t sustainFraction = W(pParameters->sustainFraction);
        int releaseHoldSamples = int(pParameters->releaseHoldSamples);
        int releaseSamples = int(pParameters->releaseSamples);
        const std::uint64_t z = 0, one = W(1.f);

        envDesc.clear();
        envDesc.push_back({ z, z, z, -1 });               // kIdle: 0 forever
        envDesc.push_back({ one, z, z, silenceSamples });   // kSilence in 10 mSec

        if (curvatureType == kAnalogLike)
        {
            envDesc.push_back({ z, one, E(0xbff8000000000000ULL), attackSamples });                          // kAttack
            envDesc.push_back({ one, one, z, holdSamples });                                  // kHold
            envDesc.push_back({ one, sustainFraction, E(0xc013cccccccccccdULL), decaySamples });              // kDecay
            envDesc.push_back({ sustainFraction, sustainFraction, z, -1 });                   // kSustain
            envDesc.push_back({ sustainFraction, sustainFraction, z, releaseHoldSamples });   // kReleaseHold
            envDesc.push_back({ sustainFraction, z, E(0xc013cccccccccccdULL), releaseSamples });            // kRelease
        }
        else if (curvatureType == kLinearInDb)
        {
            envDesc.push_back({ z, one, 0x3fefffeb074a771dULL, attackSamples });                            // kAttack
            envDesc.push_back({ one, one, z, holdSamples });                                  // kHold
            envDesc.push_back({ one, sustainFraction, E(0xc02619999999999aULL), decaySamples });             // kDecay
            envDesc.push_back({ sustainFraction, sustainFraction, z, -1 });                   // kSustain
            envDesc.push_back({ sustainFraction, sustainFraction, z, releaseHoldSamples });   // kReleaseHold
            envDesc.push_back({ sustainFraction, z, E(0xc02619999999999aULL), releaseSamples });           // kRelease
        }
        else
        {
            envDesc.push_back({ z, one, z, attackSamples });                                // kAttack
            envDesc.push_back({ one, one, z, holdSamples });                                  // kHold
            envDesc.push_back({ one, sustainFraction, z, decaySamples });                     // kDecay
            envDesc.push_back({ sustainFraction, sustainFraction, z, -1 });                   // kSustain
            envDesc.push_back({ sustainFraction, sustainFraction, z, releaseHoldSamples });   // kReleaseHold
            envDesc.push_back({ sustainFraction, z, z, releaseSamples });                   // kRelease
        }

        env.reset(&envDesc);
    }

    void AHDSHREnvelope::updateParams()
    {
        if (envDesc.size() < 8) return;
        const std::uint64_t sustainFraction = W(pParameters->sustainFraction);
        const std::uint64_t one = W(1.f);

        envDesc[kAttack].lengthSamples = int(pParameters->attackSamples);
        envDesc[kHold].initialValue = one;
        envDesc[kHold].finalValue = one;
        envDesc[kHold].lengthSamples = int(pParameters->holdSamples);
        envDesc[kDecay].finalValue = sustainFraction;
        envDesc[kDecay].lengthSamples = int(pParameters->decaySamples);
        envDesc[kSustain].initialValue = envDesc[kSustain].finalValue = sustainFraction;
        envDesc[kReleaseHold].initialValue = sustainFraction;
        envDesc[kReleaseHold].finalValue = sustainFraction;
        envDesc[kReleaseHold].lengthSamples = int(pParameters->releaseHoldSamples);
        envDesc[kRelease].initialValue = sustainFraction;
        envDesc[kRelease].lengthSamples = int(pParameters->releaseSamples);
    }

    void AHDSHREnvelope::start()
    {
        env.startAtSegment(kAttack);
    }

    void AHDSHREnvelope::restart()
    {
        env.advanceToSegment(kSilence);
    }

    void AHDSHREnvelope::release()
    {
        //update the 'release section' envelope values to be where the envelope currently is
        const std::uint64_t here = W(env.getValue());
        envDesc[kReleaseHold].initialValue = here;
        envDesc[kReleaseHold].finalValue = here;
        envDesc[kRelease].initialValue = here;
        env.advanceToSegment(kReleaseHold);
    }

    void AHDSHREnvelope::reset()
    {
        env.reset(&envDesc);
    }

}
