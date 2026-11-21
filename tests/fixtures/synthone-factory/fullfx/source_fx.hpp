// Original-source full FX reference, isolated host experiment, NOT target admission.
#pragma once
#include <array>
#include <memory>
#include <cmath>
#include <algorithm>
#include <cstring>
#include <cstdint>
#include "fx_compressor.hpp"
#define nil nullptr
struct FxSpOwner { sp_data *sp=nullptr; FxSpOwner(int rate){sp_create(&sp);sp->sr=rate;sp->nchan=2;}~FxSpOwner(){sp_destroy(&sp);} };
struct SourceFx:FxSpOwner {
 DSPParameters parameters{};float lows[150]{},highs[150]{};
 sp_ftbl*sine=nullptr;sp_osc*panOscillator=nullptr;sp_pan2*pan=nullptr;sp_phaser*phaser0=nullptr;
 sp_moogladder*loPassInputDelayL=nullptr,*loPassInputDelayR=nullptr;
 sp_vdelay*delayL=nullptr,*delayR=nullptr,*delayRR=nullptr,*delayFillIn=nullptr;
 sp_crossfade*delayCrossfadeL=nullptr,*delayCrossfadeR=nullptr,*revCrossfadeL=nullptr,*revCrossfadeR=nullptr;
 sp_revsc*reverbCostello=nullptr;sp_buthp*butterworthHipassL=nullptr,*butterworthHipassR=nullptr;sp_delay*widenDelay=nullptr;
 S1Compressor<compressorMasterRatio,compressorMasterThreshold,compressorMasterAttack,compressorMasterRelease,compressorMasterMakeupGain>mCompMaster;
 S1Compressor<compressorReverbInputRatio,compressorReverbInputThreshold,compressorReverbInputAttack,compressorReverbInputRelease,compressorReverbInputMakeupGain>mCompReverbIn;
 S1Compressor<compressorReverbWetRatio,compressorReverbWetThreshold,compressorReverbWetAttack,compressorReverbWetRelease,compressorReverbWetMakeupGain>mCompReverbWet;
 float bitcrushIncr=1.f,bitcrushIndex=0.f,bitcrushSampleIndex=0.f,bitcrushValue=0.f;
 SourceFx(const SourceFx&)=delete;SourceFx&operator=(const SourceFx&)=delete;
 explicit SourceFx(int rate):FxSpOwner(rate),mCompMaster(sp,&parameters),mCompReverbIn(sp,&parameters),mCompReverbWet(sp,&parameters){
static constexpr uint32_t minimum_words[150]={0x00000000u,0x00000000u,0x00000000u,0xc1400000u,0xc1400000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x3a088889u,0x42800000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x3a03126fu,0x3ba3d70au,0x00000000u,0x00000000u,0x3a03126fu,0x3ba3d70au,0x00000000u,0x3b83126fu,0xc0800000u,0x3f800000u,0x00000000u,0x3f800000u,0x45000000u,0x00000000u,0x3a088889u,0x00000000u,0x00000000u,0x42a00000u,0x00000000u,0x00000000u,0x00000000u,0x39be37c6u,0x00000000u,0x00000000u,0x00000000u,0x3a088889u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x3f800000u,0x00000000u,0x3f800000u,0xc1400000u,0xc1400000u,0xc1400000u,0xc1400000u,0xc1400000u,0xc1400000u,0xc1400000u,0xc1400000u,0xc1400000u,0xc1400000u,0xc1400000u,0xc1400000u,0xc1400000u,0xc1400000u,0xc1400000u,0xc1400000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x3f800000u,0x00000000u,0x42c80000u,0x00000000u,0x00000000u,0x3f800000u,0x3f800000u,0x3f800000u,0xc2700000u,0xc2700000u,0xc2700000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x3f000000u,0x3f000000u,0x3f000000u,0x3f000000u,0x00000000u,0x00000000u,0x00000000u,0xc1c00000u,0x00000000u,0x43cd0000u,0x358637bdu,0xbf800000u,0x00000000u,0x3c2aaaabu,0xc1c00000u,0x00000000u};
static constexpr uint32_t default_words[150]={0x3f800000u,0x3f800000u,0x3f000000u,0x00000000u,0x00000000u,0x3f4ccccdu,0x3f4ccccdu,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x3e800000u,0x469c4000u,0x3dcccccdu,0x3f800000u,0x00000000u,0x00000000u,0x00000000u,0x3d4ccccdu,0x3d4ccccdu,0x3f800000u,0x3f000000u,0x3d4ccccdu,0x3ba3d70au,0x3f4ccccdu,0x3d4ccccdu,0x00000000u,0x3f800000u,0x3f000000u,0x41c00000u,0x473b8000u,0x00000000u,0x3e800000u,0x3f800000u,0x3f000000u,0x442f0000u,0x00000000u,0x00000000u,0x3dcccccdu,0x3e800000u,0x3e000000u,0x00000000u,0x00000000u,0x3e800000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x3f800000u,0x41400000u,0x00000000u,0x3f800000u,0x42f00000u,0x00000000u,0x40800000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x00000000u,0x41400000u,0x00000000u,0x44480000u,0x00000000u,0x00000000u,0x41a00000u,0x41500000u,0x41500000u,0xc1100000u,0xc1080000u,0xc1000000u,0x3a83126fu,0x3a83126fu,0x3a83126fu,0x3e19999au,0x3e666666u,0x3e19999au,0x40000000u,0x3ff0a3d7u,0x3ff0a3d7u,0x3f400000u,0x00000000u,0x3f800000u,0x46000000u,0xc1400000u,0x41400000u,0x43dc0000u,0x3dcccccdu,0xbf800000u,0x00000000u,0x3e800000u,0x00000000u,0x00000000u};
static constexpr uint32_t maximum_words[150]={0x3f800000u,0x3f800000u,0x3f800000u,0x41c00000u,0x41c00000u,0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u,0x41700000u,0x3e800000u,0x40400000u,0x3f800000u,0x42a00000u,0x46ac4400u,0x3f7ae148u,0x3f800000u,0x3f99999au,0x3f800000u,0x3e4ccccdu,0x40000000u,0x40000000u,0x3f800000u,0x40000000u,0x40000000u,0x40000000u,0x3f800000u,0x40000000u,0x40800000u,0x40000000u,0x40000000u,0x41c00000u,0x473b8000u,0x3f800000u,0x41200000u,0x3f800000u,0x3f800000u,0x44610000u,0x3f800000u,0x3f800000u,0x3f666666u,0x40200000u,0x3f800000u,0x40400000u,0x3f800000u,0x42a00000u,0x40400000u,0x40400000u,0x40400000u,0x40400000u,0x40400000u,0x40400000u,0x40400000u,0x40400000u,0x40400000u,0x40400000u,0x40400000u,0x40400000u,0x40000000u,0x41400000u,0x3f800000u,0x40400000u,0x43480000u,0x3f800000u,0x41800000u,0x41400000u,0x41400000u,0x41400000u,0x41400000u,0x41400000u,0x41400000u,0x41400000u,0x41400000u,0x41400000u,0x41400000u,0x41400000u,0x41400000u,0x41400000u,0x41400000u,0x41400000u,0x41400000u,0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u,0x3f800000u,0x40000000u,0x3f800000u,0x43960000u,0x3f4ccccdu,0x447a0000u,0x3f800000u,0x3f800000u,0x41a00000u,0x41a00000u,0x41a00000u,0x00000000u,0x00000000u,0x00000000u,0x3c23d70au,0x3c23d70au,0x3c23d70au,0x3f000000u,0x3f000000u,0x3f000000u,0x40800000u,0x40800000u,0x40800000u,0x3f800000u,0x3f7ae148u,0x3f800000u,0x467ffc00u,0x00000000u,0x41c00000u,0x43eb0000u,0x3f7d70a4u,0x41400000u,0x3f800000u,0x41000000u,0x41c00000u,0x3f800000u};
 for(unsigned i=0;i<150;++i){std::memcpy(&lows[i],minimum_words+i,4);std::memcpy(&highs[i],maximum_words+i,4);std::memcpy(&parameters[i],default_words+i,4);}
 sp_ftbl_create(sp,&sine,4096);sp_gen_sine(sp,sine);sp_phaser_create(&phaser0);sp_phaser_init(sp,phaser0);sp_osc_create(&panOscillator);sp_osc_init(sp,panOscillator,sine,0.f);sp_pan2_create(&pan);sp_pan2_init(sp,pan);
    //STEREO
    sp_moogladder_create(&loPassInputDelayL);
    sp_moogladder_init(sp, loPassInputDelayL);
    sp_moogladder_create(&loPassInputDelayR);
    sp_moogladder_init(sp, loPassInputDelayR);
    sp_vdelay_create(&delayL);
    sp_vdelay_create(&delayR);
    sp_vdelay_create(&delayRR);
    sp_vdelay_create(&delayFillIn);
    sp_vdelay_init(sp, delayL, 10.f);
    sp_vdelay_init(sp, delayR, 10.f);
    sp_vdelay_init(sp, delayRR, 10.f);
    sp_vdelay_init(sp, delayFillIn, 10.f);
    sp_crossfade_create(&delayCrossfadeL);
    sp_crossfade_create(&delayCrossfadeR);
    sp_crossfade_init(sp, delayCrossfadeL);
    sp_crossfade_init(sp, delayCrossfadeR);
    sp_revsc_create(&reverbCostello);
    sp_revsc_init(sp, reverbCostello);
    sp_buthp_create(&butterworthHipassL);
    sp_buthp_init(sp, butterworthHipassL);
    sp_buthp_create(&butterworthHipassR);
    sp_buthp_init(sp, butterworthHipassR);
    sp_crossfade_create(&revCrossfadeL);
    sp_crossfade_create(&revCrossfadeR);
    sp_crossfade_init(sp, revCrossfadeL);
    sp_crossfade_init(sp, revCrossfadeR);
    sp_delay_create(&widenDelay);
    sp_delay_init(sp, widenDelay, 0.05f);
    widenDelay->feedback = 0.f;
    *phaser0->MinNotch1Freq = 100;
    *phaser0->MaxNotch1Freq = 800;
    *phaser0->Notch_width = 1000;
    *phaser0->NotchFreq = 1.5;
    *phaser0->VibratoMode = 1;
    *phaser0->depth = 1;
    *phaser0->feedback_gain = 0;
    *phaser0->invert = 0;
    *phaser0->lfobpm = 30;

    loPassInputDelayL->freq = getSynthParameter(cutoff);
    loPassInputDelayL->res = getSynthParameter(delayInputResonance);
    loPassInputDelayR->freq = getSynthParameter(cutoff);
    loPassInputDelayR->res = getSynthParameter(delayInputResonance);


 }
 ~SourceFx(){
 sp_osc_destroy(&panOscillator);sp_pan2_destroy(&pan);sp_phaser_destroy(&phaser0);sp_moogladder_destroy(&loPassInputDelayL);sp_moogladder_destroy(&loPassInputDelayR);sp_vdelay_destroy(&delayL);sp_vdelay_destroy(&delayR);sp_vdelay_destroy(&delayRR);sp_vdelay_destroy(&delayFillIn);sp_crossfade_destroy(&delayCrossfadeL);sp_crossfade_destroy(&delayCrossfadeR);sp_crossfade_destroy(&revCrossfadeL);sp_crossfade_destroy(&revCrossfadeR);sp_revsc_destroy(&reverbCostello);sp_buthp_destroy(&butterworthHipassL);sp_buthp_destroy(&butterworthHipassR);sp_delay_destroy(&widenDelay);sp_ftbl_destroy(&sine);
 }
 void source_reset_delays(){
    sp_vdelay_reset(sp, delayL);
    sp_vdelay_reset(sp, delayR);
    sp_vdelay_reset(sp, delayRR);
    sp_vdelay_reset(sp, delayFillIn);
 }
 void source_restore_effect_controls(){
    *phaser0->MinNotch1Freq = 100;
    *phaser0->MaxNotch1Freq = 800;
    *phaser0->Notch_width = 1000;
    *phaser0->NotchFreq = 1.5;
    *phaser0->VibratoMode = 1;
    *phaser0->depth = 1;
    *phaser0->feedback_gain = 0;
    *phaser0->invert = 0;
    *phaser0->lfobpm = 30;

    loPassInputDelayL->freq = getSynthParameter(cutoff);
    loPassInputDelayL->res = getSynthParameter(delayInputResonance);
    loPassInputDelayR->freq = getSynthParameter(cutoff);
    loPassInputDelayR->res = getSynthParameter(delayInputResonance);


 }
 int sampleRate()const{return sp->sr;}float maximum(S1Parameter i)const{return highs[i];}float getSynthParameter(S1Parameter i)const{return parameters[i];}float clampedValue(S1Parameter i,float v)const{return std::min(std::max(v,lows[i]),highs[i]);}
 void frame(float input,float*outL,float*outR,float lfo1_0_1,float lfo1_1_0,float lfo2_0_1,float lfo2_1_0){const unsigned frameIndex=0;float lfo3_0_1=.5f*(lfo1_0_1+lfo2_0_1),lfo3_1_0=.5f*(lfo1_1_0+lfo2_1_0);
        // MONO: NoteState render output "synthOut" is mono
        float synthOut = input;

        // BITCRUSH LFO
        float bitcrushSrate = parameters[bitCrushSampleRate];
        bitcrushSrate = log2(bitcrushSrate);
        const float magicNumber = 4.f;
        if (parameters[bitcrushLFO] == 1.f)
            bitcrushSrate += magicNumber * lfo1_0_1;
        else if (parameters[bitcrushLFO] == 2.f)
            bitcrushSrate += magicNumber * lfo2_0_1;
        else if (parameters[bitcrushLFO] == 3.f)
            bitcrushSrate += magicNumber * lfo3_0_1;
        bitcrushSrate = exp2(bitcrushSrate);
        bitcrushSrate = clampedValue(bitCrushSampleRate, bitcrushSrate); // clamp

        ///BITCRUSH
        float bitCrushOut = synthOut;
        bitcrushIncr = sampleRate() / bitcrushSrate;
        if (bitcrushIncr < 1.f) bitcrushIncr = 1.f; // for the case where the audio engine samplerate > 44100 (i.e., 48000)
        if (bitcrushIndex <= bitcrushSampleIndex) {
            bitCrushOut = bitcrushValue = synthOut;
            bitcrushIndex += bitcrushIncr; // bitcrushIncr >= 1
            bitcrushIndex -= bitcrushSampleIndex;
            bitcrushSampleIndex = 0;
        } else {
            bitCrushOut = bitcrushValue;
        }
        bitcrushSampleIndex += 1.f;

        ///TREMOLO
        if (parameters[tremoloLFO] == 1.f)
            bitCrushOut *= lfo1_1_0;
        else if (parameters[tremoloLFO] == 2.f)
            bitCrushOut *= lfo2_1_0;
        else if (parameters[tremoloLFO] == 3.f)
            bitCrushOut *= lfo3_1_0;

        ///MARK: STEREO CHAIN (EFX)

        // Signal goes from mono to stereo with autopan
        panOscillator->freq = parameters[autoPanFrequency];
        panOscillator->amp = parameters[autoPanAmount];
        float panValue = 0.f;
        sp_osc_compute(sp, panOscillator, nil, &panValue);
        pan->pan = panValue;
        float panL = 0.f, panR = 0.f;
        sp_pan2_compute(sp, pan, &bitCrushOut, &panL, &panR); // pan2 is equal power

        // PHASER+CROSSFADE
        float phaserOutL = panL;
        float phaserOutR = panR;
        float lPhaserMix = parameters[phaserMix];
        *phaser0->Notch_width = parameters[phaserNotchWidth];
        *phaser0->feedback_gain = parameters[phaserFeedback];
        *phaser0->lfobpm = parameters[phaserRate];
        if (lPhaserMix != 0.f) {
            lPhaserMix = 1.f - lPhaserMix;
            sp_phaser_compute(sp, phaser0, &panL, &panR, &phaserOutL, &phaserOutR);
            phaserOutL = lPhaserMix * panL + (1.f - lPhaserMix) * phaserOutL;
            phaserOutR = lPhaserMix * panR + (1.f - lPhaserMix) * phaserOutR;
        }

        // For lowpass osc filter: use a lowpass on delay input, with magically-attenuated cutoff
        float delayInputLowPassOutL = phaserOutL;
        float delayInputLowPassOutR = phaserOutR;
        if(parameters[filterType] == 0.f) {
            const float pmin2 = log2(1024.f);
            const float pmax2 = log2(maximum(cutoff));
            const float pval1 = parameters[cutoff];
            float pval2 = log2(pval1);
            if (pval2 < pmin2) pval2 = pmin2;
            if (pval2 > pmax2) pval2 = pmax2;
            const float pnorm2 = (pval2 - pmin2)/(pmax2 - pmin2);
            const float mmax = parameters[delayInputCutoffTrackingRatio];
            const float mmin = 1.f;
            const float oscFilterFreqCutoffPercentage = mmin + pnorm2 * (mmax - mmin);
            const float oscFilterResonance = 0.f; // constant
            float oscFilterFreqCutoff = pval1 * oscFilterFreqCutoffPercentage;
            oscFilterFreqCutoff = clampedValue(cutoff, oscFilterFreqCutoff);
            loPassInputDelayL->freq = oscFilterFreqCutoff;
            loPassInputDelayL->res = oscFilterResonance;
            loPassInputDelayR->freq = oscFilterFreqCutoff;
            loPassInputDelayR->res = oscFilterResonance;
            sp_moogladder_compute(sp, loPassInputDelayL, &phaserOutL, &delayInputLowPassOutL);
            sp_moogladder_compute(sp, loPassInputDelayR, &phaserOutR, &delayInputLowPassOutR);
        }

        // PING PONG DELAY
        float delayOutL = 0.f;
        float delayOutR = 0.f;
        float delayOutRR = 0.f;
        float delayFillInOut = 0.f;
        delayL->del = delayR->del = parameters[delayTime] * 2.f;
        delayRR->del = delayFillIn->del = parameters[delayTime];
        delayL->feedback = delayR->feedback = parameters[delayFeedback];
        delayRR->feedback = delayFillIn->feedback = parameters[delayFeedback];
        sp_vdelay_compute(sp, delayL,      &delayInputLowPassOutL, &delayOutL);
        sp_vdelay_compute(sp, delayR,      &delayInputLowPassOutR, &delayOutR);
        sp_vdelay_compute(sp, delayFillIn, &delayInputLowPassOutR, &delayFillInOut);
        sp_vdelay_compute(sp, delayRR,     &delayOutR,  &delayOutRR);
        delayOutRR += delayFillInOut;

        // DELAY MIXER
        float mixedDelayL = 0.f;
        float mixedDelayR = 0.f;
        delayCrossfadeL->pos = parameters[delayMix] * parameters[delayOn];
        delayCrossfadeR->pos = parameters[delayMix] * parameters[delayOn];
        sp_crossfade_compute(sp, delayCrossfadeL, &phaserOutL, &delayOutL, &mixedDelayL);
        sp_crossfade_compute(sp, delayCrossfadeR, &phaserOutR, &delayOutRR, &mixedDelayR);

        // REVERB INPUT HIPASS FILTER
        float butOutL = 0.f;
        float butOutR = 0.f;
        butterworthHipassL->freq = parameters[reverbHighPass];
        butterworthHipassR->freq = parameters[reverbHighPass];
        sp_buthp_compute(sp, butterworthHipassL, &mixedDelayL, &butOutL);
        sp_buthp_compute(sp, butterworthHipassR, &mixedDelayR, &butOutR);

        // Pre Gain + compression on reverb input
        butOutL *= 2.f;
        butOutR *= 2.f;
        float butCompressOutL = 0.f;
        float butCompressOutR = 0.f;
        mCompReverbIn.compute(butOutL, butOutR, butCompressOutL, butCompressOutR);

        // REVERB
        float reverbWetL = 0.f;
        float reverbWetR = 0.f;
        reverbCostello->feedback = parameters[reverbFeedback];
        reverbCostello->lpfreq = 0.5f * sampleRate();
        sp_revsc_compute(sp, reverbCostello, &butCompressOutL, &butCompressOutR, &reverbWetL, &reverbWetR);

        // compressor for wet reverb; like X2, FM
        float wetReverbLimiterL = reverbWetL;
        float wetReverbLimiterR = reverbWetR;
        mCompReverbWet.compute(reverbWetL, reverbWetR, wetReverbLimiterL, wetReverbLimiterR);

        // crossfade wet reverb with wet+dry delay
        float reverbCrossfadeOutL = 0.f;
        float reverbCrossfadeOutR = 0.f;
        float reverbMixFactor = parameters[reverbMix] * parameters[reverbOn];
        if (parameters[reverbMixLFO] == 1.f)
            reverbMixFactor *= lfo1_1_0;
        else if (parameters[reverbMixLFO] == 2.f)
            reverbMixFactor *= lfo2_1_0;
        else if (parameters[reverbMixLFO] == 3.f)
            reverbMixFactor *= lfo3_1_0;
        revCrossfadeL->pos = reverbMixFactor;
        revCrossfadeR->pos = reverbMixFactor;
        sp_crossfade_compute(sp, revCrossfadeL, &mixedDelayL, &wetReverbLimiterL, &reverbCrossfadeOutL);
        sp_crossfade_compute(sp, revCrossfadeR, &mixedDelayR, &wetReverbLimiterR, &reverbCrossfadeOutR);

        // MASTER COMPRESSOR/LIMITER
        // 3db pre gain on input to master compressor
        reverbCrossfadeOutL *= (2.f * parameters[masterVolume]);
        reverbCrossfadeOutR *= (2.f * parameters[masterVolume]);
        float compressorOutL = reverbCrossfadeOutL;
        float compressorOutR = reverbCrossfadeOutR;

        // MASTER COMPRESSOR TOGGLE: 0 = no compressor, 1 = compressor
        mCompMaster.compute(reverbCrossfadeOutL, reverbCrossfadeOutR, compressorOutL, compressorOutR);

        // WIDEN: constant delay with no filtering, so functionally equivalent to being inside master
        float widenOutR = 0.f;
        sp_delay_compute(sp, widenDelay, &compressorOutR, &widenOutR);
        widenOutR = parameters[widen] * widenOutR + (1.f - parameters[widen]) * compressorOutR;

        // MASTER
        outL[frameIndex] = compressorOutL;
        outR[frameIndex] = widenOutR;
 }
};
#undef nil
