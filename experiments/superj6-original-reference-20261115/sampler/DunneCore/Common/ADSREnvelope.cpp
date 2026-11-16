// Copyright AudioKit. All Rights Reserved.

#include "ADSREnvelope.h"

namespace DunneCore
{
    static std::uint64_t W(float x) { return original_conductor_v1::f64_from_f32(x); }
    static std::uint64_t E(std::uint64_t x) { return original_conductor_v1::f64_exp(x); }
    static std::uint64_t expm1(std::uint64_t x) { return original_conductor_v1::f64_sub(E(x), 0x3ff0000000000000ULL); }

    ADSREnvelopeParameters::ADSREnvelopeParameters()
        : sampleRateHz(44100.0f) // a guess, will be overridden later by a call to init(,,,,)
    {
        init(0.0f, 0.0f, 1.0f, 0.0f);
    }

    void ADSREnvelopeParameters::init(float attackSeconds, float decaySeconds, float susFraction, float releaseSeconds)
    {
        attackSamples = attackSeconds * sampleRateHz;
        decaySamples = decaySeconds * sampleRateHz;
        sustainFraction = susFraction;
        releaseSamples = releaseSeconds * sampleRateHz;
    }

    void ADSREnvelopeParameters::init(float newSampleRateHz, float attackSeconds, float decaySeconds, float susFraction, float releaseSeconds)
    {
        sampleRateHz = newSampleRateHz;
        init(attackSeconds, decaySeconds, susFraction, releaseSeconds);
    }

    void ADSREnvelopeParameters::updateSampleRate(float newSampleRateHz)
    {
        float scaleFactor = newSampleRateHz / sampleRateHz;
        sampleRateHz = newSampleRateHz;
        attackSamples *= scaleFactor;
        decaySamples *= scaleFactor;
        releaseSamples *= scaleFactor;
    }


    void ADSREnvelope::init(CurvatureType curvatureType)
    {
        currentCurvature=curvatureType;
        int silenceSamples = int(0.01f * pParameters->sampleRateHz);     // always 10 mSec
        int attackSamples = int(pParameters->attackSamples);
        int decaySamples = int(pParameters->decaySamples);
        const std::uint64_t sustainFraction = W(pParameters->sustainFraction);
        int releaseSamples = int(pParameters->releaseSamples);
        const std::uint64_t z = 0, one = W(1.f);

        envDesc.clear();
        envDesc.push_back({ z, z, z, -1 });               // kIdle: 0 forever
        envDesc.push_back({ one, z, z, silenceSamples });   // kSilence in 10 mSec

        if(curvatureType==kOriginalOTA) {
            const std::uint64_t reciprocal = original_conductor_v1::f64_div(one, expm1(0x3ff8000000000000ULL));
            envDesc.push_back({z,one,original_conductor_v1::f64_div(one, expm1(one)),attackSamples});
            envDesc.push_back({one,sustainFraction,original_conductor_v1::f64_mul(original_conductor_v1::f64_sub(one, sustainFraction), reciprocal),decaySamples});
            envDesc.push_back({sustainFraction,sustainFraction,z,-1});
            envDesc.push_back({sustainFraction,z,original_conductor_v1::f64_mul(sustainFraction, reciprocal),releaseSamples});
        }
        else if (curvatureType == kAnalogLike)
        {
            envDesc.push_back({ z, one, E(0xbff8000000000000ULL), attackSamples });                  // kAttack
            envDesc.push_back({ one, sustainFraction, E(0xc013cccccccccccdULL), decaySamples });      // kDecay
            envDesc.push_back({ sustainFraction, sustainFraction, z, -1 });           // kSustain
            envDesc.push_back({ sustainFraction, z, E(0xc013cccccccccccdULL), releaseSamples });    // kRelease
        }
        else if (curvatureType == kLinearInDb)
        {
            envDesc.push_back({ z, one, 0x3fefffeb074a771dULL, attackSamples });                    // kAttack
            envDesc.push_back({ one, sustainFraction, E(0xc02619999999999aULL), decaySamples });     // kDecay
            envDesc.push_back({ sustainFraction, sustainFraction, z, -1 });           // kSustain
            envDesc.push_back({ sustainFraction, z, E(0xc02619999999999aULL), releaseSamples });   // kRelease
        }
        else
        {
            envDesc.push_back({ z, one, z, attackSamples });                        // kAttack
            envDesc.push_back({ one, sustainFraction, z, decaySamples });             // kDecay
            envDesc.push_back({ sustainFraction, sustainFraction, z, -1 });           // kSustain
            envDesc.push_back({ sustainFraction, z, z, releaseSamples });           // kRelease
        }

        env.reset(&envDesc);
    }

    void ADSREnvelope::updateParams()
    {
        if (envDesc.size() < 6) return;
        const std::uint64_t sustainFraction = W(pParameters->sustainFraction);
        const std::uint64_t one = W(1.f);

        envDesc[kAttack].lengthSamples = int(pParameters->attackSamples);
        envDesc[kDecay].finalValue = sustainFraction;
        envDesc[kDecay].lengthSamples = int(pParameters->decaySamples);
        envDesc[kSustain].initialValue = envDesc[kSustain].finalValue = sustainFraction;
        envDesc[kRelease].initialValue = sustainFraction;
        envDesc[kRelease].lengthSamples = int(pParameters->releaseSamples);
        if(currentCurvature==kOriginalOTA){
            const std::uint64_t reciprocal = original_conductor_v1::f64_div(one, expm1(0x3ff8000000000000ULL));
            envDesc[kDecay].tco=original_conductor_v1::f64_mul(original_conductor_v1::f64_sub(one, sustainFraction), reciprocal);
            envDesc[kRelease].tco=original_conductor_v1::f64_mul(sustainFraction, reciprocal);
        }
    }

    void ADSREnvelope::start()
    {
        env.startAtSegment(kAttack);
    }

    void ADSREnvelope::restart()
    {
        env.advanceToSegment(kSilence);
    }

    void ADSREnvelope::release()
    {
        envDesc[kRelease].initialValue = W(env.getValue()); //update release start value to current val of envelope
        env.advanceToSegment(kRelease);
    }

    void ADSREnvelope::reset()
    {
        env.reset(&envDesc);
    }

}
