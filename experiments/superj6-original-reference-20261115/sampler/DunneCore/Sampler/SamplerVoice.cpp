// Copyright AudioKit. All Rights Reserved.

#include "SamplerVoice.h"
#include <stdio.h>

#define MIDDLE_C_HZ 261.626f

namespace DunneCore
{
    void SamplerVoice::init(double sampleRate)
    {
        synthOsc.reset();whiteNoiseOsc.reset();whiteNoise=pendingWhiteNoise=false;
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

    void SamplerVoice::start(unsigned note, float sampleRate, float frequency, float volume, SampleBuffer *buffer, bool noise)
    {
        synthOsc.reset();whiteNoise=pendingWhiteNoise=noise;whiteNoiseOsc.reset();
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
    
    void SamplerVoice::restartNewNote(unsigned note, float sampleRate, float frequency, float volume, SampleBuffer *buffer, bool noise)
    {
        pendingWhiteNoise=noise;
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

    void SamplerVoice::restartSameNote(float volume, SampleBuffer *buffer, bool noise)
    {
        pendingWhiteNoise=noise;
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
        noteNumber = -1;whiteNoise=pendingWhiteNoise=false;whiteNoiseOsc.reset();synthOsc.reset();
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
                sampleBuffer = newSampleBuffer;whiteNoise=pendingWhiteNoise;whiteNoiseOsc.reset();synthOsc.restart();
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
        float normalized=(std::exp2(pitchOffsetModified/12.f)*noteFrequency)/samplingRate;
        if(normalized>.49000000953674316f)normalized=.49000000953674316f;
        if(normalized<0.f)normalized=0.f;
        synthOsc.increment=normalized;

        const float otaEnv=otaFilterEnvelope.getSample();
        const float env=(useOTAFilter||vintageFilterADSR||cutoffMultiple>=0.f)?filterEnvelope.getSample():0.f;
        // Original vintage/OTA preparation advances even while audio bypasses it.
        isFilterEnabled=cutoffMultiple>=0.f;
        if (isFilterEnabled||useOTAFilter||vintageFilterADSR)
        {
            float noteHz = noteFrequency * powf(2.0f, pitchOffsetModified / 12.0f);

            if(useOTAFilter){
                const float vel=std::fma(cutoffEnvelopeVelocityScaling,noteVolume,1.f-cutoffEnvelopeVelocityScaling);
                float amount=cutoffEnvelopeStrength*14.f;amount=amount*vel;amount=otaEnv*amount;
                const float key=keyTracking*std::log2(std::max(noteHz,1.f)/MIDDLE_C_HZ);
                const float cutoff=std::min(std::max(std::max(userCutoff,3.f)*std::exp2(amount+key),3.f),30000.f);
                const float q=(-20.f*std::log10(std::max(resLinear,.00009999999747378752f)))/18.989999771118164f;
                leftOTAFilter.setParameters(double(cutoff),double(q),sampleCount);rightOTAFilter.setParameters(double(cutoff),double(q),sampleCount);
            } else if(useSyrupFilter){
                // Legacy32=false path is note-relative, independent of isOSC.
                const float baseFrequency=std::fma(keyTracking,noteHz-MIDDLE_C_HZ,MIDDLE_C_HZ);
                const float envStrength=std::fma(cutoffEnvelopeVelocityScaling,noteVolume,1.f-cutoffEnvelopeVelocityScaling);
                double cutoff=baseFrequency*std::fma(cutoffEnvelopeStrength*envStrength,env,1.f+cutoffMultiple);
                if(vintageFilterADSR){
                    const double lo=std::max(double(userCutoff),1.0),hi=std::max(lo,22050.0);
                    const double e=std::min(std::max(double(env),0.0),1.0);
                    const double amount=std::min(std::max(double(cutoffEnvelopeStrength*envStrength),0.0),1.0);
                    const double t=1.0-e,shape=1.0-t*t;
                    cutoff=std::exp2(std::fma(amount*shape,std::log2(hi)-std::log2(lo),std::log2(lo)));
                    if(keyTracking!=0.f&&noteHz>1.f&&cutoff>1.0)cutoff*=std::pow(double(noteHz)/double(MIDDLE_C_HZ),double(keyTracking));
                    cutoff=std::min(std::max(cutoff,1.0),22050.0);
                }
                leftSyrupFilter.fourPole=rightSyrupFilter.fourPole=syrup24;
                leftSyrupFilter.setParameters(cutoff,resLinear);rightSyrupFilter.setParameters(cutoff,resLinear);
            } else {
                float baseFrequency = std::fma(keyTracking, noteHz - MIDDLE_C_HZ, MIDDLE_C_HZ);
                float envStrength = std::fma(cutoffEnvelopeVelocityScaling, noteVolume, 1.0f - cutoffEnvelopeVelocityScaling);
                double cutoffFrequency = baseFrequency * std::fma(cutoffEnvelopeStrength * envStrength, env, 1.0f + cutoffMultiple);
                if(vintageFilterADSR){
                    const float span=22050.f-userCutoff;
                    const float add=span*((cutoffEnvelopeStrength*envStrength)*env);
                    cutoffFrequency=double(userCutoff)+double(add);
                    if(noteHz>130.80999755859375f){
                        const float ratio=noteHz/130.80999755859375f;
                        const float exponent=std::max(1.f/(12.f-std::log2(ratio)),.0416666679084301f);
                        const float bend=(((std::pow(ratio,exponent)-1.f)*keyTracking)*span)/3.3299999237060547f;
                        cutoffFrequency+=double(bend);
                    }
                    cutoffFrequency=std::min(cutoffFrequency,22050.0);
                }
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
            const float inputGain=useOTAFilter&&isFilterEnabled?1.f:useSyrupFilter&&isFilterEnabled?tempGain:gain;
            const bool analytic=isOSC&&(oscSaw||oscTriangle||oscSine||oscPWM);
            if(whiteNoise){leftSample=rightSample=inputGain*whiteNoiseOsc.sample();}
            else if(analytic){
                const float inputGain=useOTAFilter&&isFilterEnabled?1.f:useSyrupFilter&&isFilterEnabled?tempGain:gain;
                leftSample=rightSample=oscSaw?synthOsc.processSaw(inputGain):oscTriangle?synthOsc.processTriangle(inputGain):oscSine?synthOsc.processSine(inputGain):oscPWM?synthOsc.processPWM(inputGain,oscPWMWidth):0.f;
            }
            else if(oscillator.getSamplePair(sampleBuffer,sampleCount,&leftSample,&rightSample,inputGain))return true;
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
