// Copyright AudioKit. All Rights Reserved.

#include "SamplerVoice.h"
#include <stdio.h>

#define MIDDLE_C_HZ 261.626f

namespace DunneCore
{
    void SamplerVoice::init(double sampleRate)
    {
        samplingRate = float(sampleRate);
        leftFilter.init(sampleRate);
        rightFilter.init(sampleRate);
        leftSmoothFilter.init(sampleRate);
        leftSyrupFilter.init(sampleRate);rightSyrupFilter.init(sampleRate);
        leftOTAFilter.init(float(sampleRate));rightOTAFilter.init(float(sampleRate));
        rightSmoothFilter.init(sampleRate);
        ampEnvelope.init();
        filterEnvelope.init();
        otaFilterEnvelope.pParameters=filterEnvelope.pParameters;
        otaFilterEnvelope.init(ADSREnvelope::kOriginalOTA);
        pitchEnvelope.init();
        vibratoLFO.waveTable.sinusoid();
        vibratoLFO.init(sampleRate/CORESAMPLER_CHUNKSIZE, 5.0f);
        restartVoiceLFO = false;
        hasStartedVoiceLFO = false; // Explicitly initialize upstream indeterminate bool.
        volumeRamper.init(0.0f);
        tempGain = 0.0f;
    }

    void SamplerVoice::start(unsigned note, float sampleRate, float frequency, float volume, SampleBuffer *buffer)
    {
        sampleBuffer = buffer;
        oscillator.indexPoint = buffer->startPoint;
        oscillator.increment = (buffer->sampleRate / sampleRate) * (frequency / buffer->noteFrequency);
        oscillator.multiplier = 1.0;
        oscillator.isLooping = buffer->isLooping;
        
        noteVolume = volume;
        ampEnvelope.start();
        volumeRamper.init(0.0f);
        
        samplingRate = sampleRate;
        leftFilter.updateSampleRate(double(samplingRate));
        rightFilter.updateSampleRate(double(samplingRate));
        leftSmoothFilter.updateSampleRate(double(samplingRate));
        rightSmoothFilter.updateSampleRate(double(samplingRate));
        leftSyrupFilter.updateSampleRate(double(samplingRate));rightSyrupFilter.updateSampleRate(double(samplingRate));
        leftOTAFilter.updateSampleRate(samplingRate);rightOTAFilter.updateSampleRate(samplingRate);
        filterEnvelope.start();otaFilterEnvelope.start();

        pitchEnvelope.start();

        pitchEnvelopeSemitones = 0.0f;

        voiceLFOSemitones = 0.0f;

        glideSemitones = 0.0f;
        if (*glideSecPerOctave != 0.0f && noteFrequency != 0.0 && noteFrequency != frequency)
        {
            // prepare to glide
            glideSemitones = -12.0f * log2f(frequency / noteFrequency);
            if (fabsf(glideSemitones) < 0.01f) glideSemitones = 0.0f;
        }
        noteFrequency = frequency;
        noteNumber = note;

        restartVoiceLFOIfNeeded();
    }
    
    void SamplerVoice::restartNewNote(unsigned note, float sampleRate, float frequency, float volume, SampleBuffer *buffer)
    {
        samplingRate = sampleRate;
        leftFilter.updateSampleRate(double(samplingRate));
        rightFilter.updateSampleRate(double(samplingRate));
        leftSmoothFilter.updateSampleRate(double(samplingRate));
        rightSmoothFilter.updateSampleRate(double(samplingRate));
        leftSyrupFilter.updateSampleRate(double(samplingRate));rightSyrupFilter.updateSampleRate(double(samplingRate));
        leftOTAFilter.updateSampleRate(samplingRate);rightOTAFilter.updateSampleRate(samplingRate);

        oscillator.increment = (sampleBuffer->sampleRate / sampleRate) * (frequency / sampleBuffer->noteFrequency);
        glideSemitones = 0.0f;
        if (*glideSecPerOctave != 0.0f && noteFrequency != 0.0 && noteFrequency != frequency)
        {
            // prepare to glide
            glideSemitones = -12.0f * log2f(frequency / noteFrequency);
            if (fabsf(glideSemitones) < 0.01f) glideSemitones = 0.0f;
        }

        pitchEnvelopeSemitones = 0.0f;

        voiceLFOSemitones = 0.0f;

        noteFrequency = frequency;
        noteNumber = note;
        tempNoteVolume = noteVolume;
        newSampleBuffer = buffer;
        ampEnvelope.restart();
        noteVolume = volume;
        filterEnvelope.restart();otaFilterEnvelope.restart();
        pitchEnvelope.restart();
        restartVoiceLFOIfNeeded();
    }

    void SamplerVoice::restartNewNoteLegato(unsigned note, float sampleRate, float frequency)
    {
        samplingRate = sampleRate;
        leftFilter.updateSampleRate(double(samplingRate));
        rightFilter.updateSampleRate(double(samplingRate));
        leftSmoothFilter.updateSampleRate(double(samplingRate));
        rightSmoothFilter.updateSampleRate(double(samplingRate));
        leftSyrupFilter.updateSampleRate(double(samplingRate));rightSyrupFilter.updateSampleRate(double(samplingRate));
        leftOTAFilter.updateSampleRate(samplingRate);rightOTAFilter.updateSampleRate(samplingRate);

        oscillator.increment = (sampleBuffer->sampleRate / sampleRate) * (frequency / sampleBuffer->noteFrequency);
        float previousGlideSemitones = glideSemitones;
        glideSemitones = 0.0f;
        if (*glideSecPerOctave != 0.0f && noteFrequency != 0.0)
        {
            // prepare to glide
            glideSemitones = std::fma(-12.0f, log2f(frequency / noteFrequency), previousGlideSemitones);
            if (fabsf(glideSemitones) < 0.01f) glideSemitones = 0.0f;
        }
        noteFrequency = frequency;
        noteNumber = note;
    }

    void SamplerVoice::restartSameNote(float volume, SampleBuffer *buffer)
    {
        tempNoteVolume = noteVolume;
        newSampleBuffer = buffer;
        ampEnvelope.restart();
        noteVolume = volume;
        filterEnvelope.restart();otaFilterEnvelope.restart();
        pitchEnvelope.restart();
        restartVoiceLFOIfNeeded();
    }
    
    void SamplerVoice::release(bool loopThruRelease)
    {
        if (!loopThruRelease) oscillator.isLooping = false;
        ampEnvelope.release();
        filterEnvelope.release();otaFilterEnvelope.release();
        pitchEnvelope.release();
    }
    
    void SamplerVoice::stop()
    {
        noteNumber = -1;
        ampEnvelope.reset();
        volumeRamper.init(0.0f);
        filterEnvelope.reset();otaFilterEnvelope.reset();
        pitchEnvelope.reset();
    }

    bool SamplerVoice::prepToGetSamples(int sampleCount, float masterVolume, float pitchOffset,
                                        float cutoffMultiple, float keyTracking,
                                        float cutoffEnvelopeStrength, float cutoffEnvelopeVelocityScaling,
                                        float resLinear, float pitchADSRSemitones,
                                        float voiceLFODepthSemitones, float voiceLFOFrequencyHz)
    {
        if (ampEnvelope.isIdle()) return true;

        if (ampEnvelope.isPreStarting())
        {
            tempGain = masterVolume * tempNoteVolume;
            volumeRamper.reinit(ampEnvelope.getSample(), sampleCount);
            // This can execute as part of the voice-stealing mechanism, and will be executed rarely.
            // To test, set MAX_POLYPHONY in CoreSampler.cpp to something small like 2 or 3.
            if (!ampEnvelope.isPreStarting())
            {
                tempGain = masterVolume * noteVolume;
                volumeRamper.reinit(ampEnvelope.getSample(), sampleCount);
                sampleBuffer = newSampleBuffer;
                oscillator.increment = (sampleBuffer->sampleRate / samplingRate) * (noteFrequency / sampleBuffer->noteFrequency);
                oscillator.indexPoint = sampleBuffer->startPoint;
                oscillator.isLooping = sampleBuffer->isLooping;
            }
        }
        else
        {
            tempGain = masterVolume * noteVolume;
            volumeRamper.reinit(ampEnvelope.getSample(), sampleCount);
        }

        if (*glideSecPerOctave != 0.0f && glideSemitones != 0.0f)
        {
            float seconds = sampleCount / samplingRate;
            float semitones = 12.0f * seconds / *glideSecPerOctave;
            if (glideSemitones < 0.0f)
            {
                glideSemitones += semitones;
                if (glideSemitones > 0.0f) glideSemitones = 0.0f;
            }
            else
            {
                glideSemitones -= semitones;
                if (glideSemitones < 0.0f) glideSemitones = 0.0f;
            }
        }

        float pitchCurveAmount = 1.0f; // >1 = faster curve, 0 < curve < 1 = slower curve - make this a parameter
        if (pitchCurveAmount < 0) { pitchCurveAmount = 0; }
        pitchEnvelopeSemitones = pow(pitchEnvelope.getSample(), pitchCurveAmount) * pitchADSRSemitones;

        vibratoLFO.setFrequency(voiceLFOFrequencyHz);
        voiceLFOSemitones = vibratoLFO.getSample() * voiceLFODepthSemitones;

        float pitchOffsetModified = pitchOffset + glideSemitones + pitchEnvelopeSemitones + voiceLFOSemitones;
        oscillator.setPitchOffsetSemitones(pitchOffsetModified);

        const float otaEnv=otaFilterEnvelope.getSample();
        // negative value of cutoffMultiple means filters are disabled
        if (cutoffMultiple < 0.0f)
        {
            isFilterEnabled = false;
        }
        else
        {
            isFilterEnabled = true;
            float noteHz = noteFrequency * powf(2.0f, pitchOffsetModified / 12.0f);
            float env=filterEnvelope.getSample();
            if(useOTAFilter){
                const float vel=std::fma(cutoffEnvelopeVelocityScaling,noteVolume,1.f-cutoffEnvelopeVelocityScaling);
                float amount=cutoffEnvelopeStrength*14.f;amount=amount*vel;amount=otaEnv*amount;
                const float key=keyTracking*std::log2(std::max(noteHz,1.f)/MIDDLE_C_HZ);
                const float cutoff=std::min(std::max(std::max(userCutoff,3.f)*std::exp2(amount+key),3.f),30000.f);
                const float q=(-20.f*std::log10(std::max(resLinear,.00009999999747378752f)))/18.989999771118164f;
                leftOTAFilter.setParameters(double(cutoff),double(q),sampleCount);rightOTAFilter.setParameters(double(cutoff),double(q),sampleCount);
            } else if(useSyrupFilter){
                // The recorded component32=false path is note-relative.
                // The separate vintageFilterADSR=true laws are not admitted here.
                const float baseFrequency=std::fma(keyTracking,noteHz-MIDDLE_C_HZ,MIDDLE_C_HZ);
                const float envStrength=std::fma(cutoffEnvelopeVelocityScaling,noteVolume,1.f-cutoffEnvelopeVelocityScaling);
                const double cutoff=baseFrequency*std::fma(cutoffEnvelopeStrength*envStrength,env,1.f+cutoffMultiple);
                leftSyrupFilter.fourPole=rightSyrupFilter.fourPole=syrup24;
                leftSyrupFilter.setParameters(cutoff,resLinear);rightSyrupFilter.setParameters(cutoff,resLinear);
            } else {
                float baseFrequency = std::fma(keyTracking, noteHz - MIDDLE_C_HZ, MIDDLE_C_HZ);
                float envStrength = std::fma(cutoffEnvelopeVelocityScaling, noteVolume, 1.0f - cutoffEnvelopeVelocityScaling);
                double cutoffFrequency = baseFrequency * std::fma(cutoffEnvelopeStrength * envStrength, env, 1.0f + cutoffMultiple);
                if(useJunoFilter) {
                    leftSmoothFilter.setParameters(cutoffFrequency,resLinear);rightSmoothFilter.setParameters(cutoffFrequency,resLinear);
                } else {
                    leftFilter.setParameters(cutoffFrequency,resLinear);rightFilter.setParameters(cutoffFrequency,resLinear);
                }
            }
        }
        
        return false;
    }
    
    bool SamplerVoice::getSamples(int sampleCount, float *leftOutput, float *rightOutput)
    {
        for (int i=0; i < sampleCount; i++)
        {
            float envelopeGain=volumeRamper.getNextValue();
            float gain = tempGain * envelopeGain;
            float leftSample, rightSample;
            if (oscillator.getSamplePair(sampleBuffer, sampleCount, &leftSample, &rightSample, useOTAFilter&&isFilterEnabled?1.f:useSyrupFilter&&isFilterEnabled?tempGain:gain))
                return true;
            if (isFilterEnabled)
            {
                if(useOTAFilter){*leftOutput++ += gain*leftOTAFilter.process(leftSample);*rightOutput++ += gain*rightOTAFilter.process(rightSample);}
                else if(useSyrupFilter){*leftOutput++ += envelopeGain*leftSyrupFilter.process(leftSample);*rightOutput++ += envelopeGain*rightSyrupFilter.process(rightSample);}
                else if(useJunoFilter) {
                    *leftOutput++ += leftSmoothFilter.process(leftSample);
                    *rightOutput++ += rightSmoothFilter.process(rightSample);
                } else {
                    *leftOutput++ += leftFilter.process(leftSample);
                    *rightOutput++ += rightFilter.process(rightSample);
                }
            }
            else
            {
                *leftOutput++ += leftSample;
                *rightOutput++ += rightSample;
            }
        }
        return false;
    }

    void SamplerVoice::restartVoiceLFOIfNeeded() {
        if (restartVoiceLFO || !hasStartedVoiceLFO) {
            vibratoLFO.phase = 0;
            hasStartedVoiceLFO = true;
        }
    }

}
