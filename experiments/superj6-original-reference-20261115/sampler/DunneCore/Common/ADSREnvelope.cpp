// Copyright AudioKit. All Rights Reserved.

#include "ADSREnvelope.h"

#include <cmath>

namespace DunneCore
{

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
        float sustainFraction = float(pParameters->sustainFraction);
        int releaseSamples = int(pParameters->releaseSamples);

        envDesc.clear();
        envDesc.push_back({ 0.0f, 0.0f, 0.0f, -1 });               // kIdle: 0 forever
        envDesc.push_back({ 1.0f, 0.0f, 0.0f, silenceSamples });   // kSilence in 10 mSec

        if(curvatureType==kOriginalOTA) {
            const float reciprocal=1.f/std::expm1(1.5f);
            envDesc.push_back({0.f,1.f,1.f/std::expm1(1.f),attackSamples});
            envDesc.push_back({1.f,sustainFraction,(1.f-sustainFraction)*reciprocal,decaySamples});
            envDesc.push_back({sustainFraction,sustainFraction,0.f,-1});
            envDesc.push_back({sustainFraction,0.f,sustainFraction*reciprocal,releaseSamples});
        }
        else if (curvatureType == kAnalogLike)
        {
            envDesc.push_back({ 0.0f, 1.0f, std::exp(-1.5f), attackSamples });                  // kAttack
            envDesc.push_back({ 1.0f, sustainFraction, std::exp(-4.95f), decaySamples });      // kDecay
            envDesc.push_back({ sustainFraction, sustainFraction, 0.0f, -1 });           // kSustain
            envDesc.push_back({ sustainFraction, 0.0f, std::exp(-4.95f), releaseSamples });    // kRelease
        }
        else if (curvatureType == kLinearInDb)
        {
            envDesc.push_back({ 0.0f, 1.0f, 0.99999f, attackSamples });                    // kAttack
            envDesc.push_back({ 1.0f, sustainFraction, std::exp(-11.05f), decaySamples });     // kDecay
            envDesc.push_back({ sustainFraction, sustainFraction, 0.0f, -1 });           // kSustain
            envDesc.push_back({ sustainFraction, 0.0f, std::exp(-11.05f), releaseSamples });   // kRelease
        }
        else
        {
            envDesc.push_back({ 0.0f, 1.0f, 0.0f, attackSamples });                        // kAttack
            envDesc.push_back({ 1.0f, sustainFraction, 0.0f, decaySamples });             // kDecay
            envDesc.push_back({ sustainFraction, sustainFraction, 0.0f, -1 });           // kSustain
            envDesc.push_back({ sustainFraction, 0.0f, 0.0f, releaseSamples });           // kRelease
        }

        env.reset(&envDesc);
    }

    void ADSREnvelope::updateParams()
    {
        if (envDesc.size() < 6) return;
        float sustainFraction = float(pParameters->sustainFraction);

        envDesc[kAttack].lengthSamples = int(pParameters->attackSamples);
        envDesc[kDecay].finalValue = sustainFraction;
        envDesc[kDecay].lengthSamples = int(pParameters->decaySamples);
        envDesc[kSustain].initialValue = envDesc[kSustain].finalValue = sustainFraction;
        envDesc[kRelease].initialValue = sustainFraction;
        envDesc[kRelease].lengthSamples = int(pParameters->releaseSamples);
        if(currentCurvature==kOriginalOTA){
            const float reciprocal=1.f/std::expm1(1.5f);
            envDesc[kDecay].tco=(1.f-sustainFraction)*reciprocal;
            envDesc[kRelease].tco=sustainFraction*reciprocal;
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
        envDesc[kRelease].initialValue = env.getValue(); //update release start value to current val of envelope
        env.advanceToSegment(kRelease);
    }

    void ADSREnvelope::reset()
    {
        env.reset(&envDesc);
    }

}
