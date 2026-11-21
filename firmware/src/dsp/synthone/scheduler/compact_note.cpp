// Generated from pinned original source. See DERIVATION.json and notices.
#include "compact_note.hpp"
#include "float_math.h"
extern "C" {
#include "compact-reader/compact_morph.h"
}
namespace {
static inline float nnToHz(float noteNumber) {
#if defined(S1_NO_DOUBLE)
    return s1_exp2f(noteNumber/12.f);
#else
    return exp2(noteNumber/12.f);
#endif
}
}
namespace s1scheduler_compact {
bool run_note(S1NoteState &note,int frameIndex,float *outL,float *outR,const s1c_binding &binding){
    auto &kernel=note.kernel;
    auto &oscmorph1=note.oscmorph1;
    auto &oscmorph2=note.oscmorph2;
    auto &subOsc=note.subOsc;
    auto &fmOsc=note.fmOsc;
    auto &noise=note.noise;
    auto &morphCrossFade=note.morphCrossFade;
    auto &filterCrossFade=note.filterCrossFade;
    auto &loPass=note.loPass;
    auto &bandPass=note.bandPass;
    auto &hiPass=note.hiPass;
    auto &adsr=note.adsr;
    auto &fadsr=note.fadsr;
    auto &internalGate=note.internalGate;
    auto &amp=note.amp;
    auto &filter=note.filter;
    auto getParam=[&](S1Parameter param)->float {
    return kernel->parameters[param];
};
    auto sampleRate=[&]()->int {
    return kernel->sampleRate();
};
    bool reader_ok=true;
    auto runtime_morph_compute=[&](sp_data *sp,sp_oscmorph2d *osc,SPFLOAT *in,SPFLOAT *out)->int {
        const int rc=s1c_runtime_morph_compute(sp,osc,in,out,&binding);
        if(rc!=SP_OK)reader_ok=false;
        return rc;
    };

    
    // isMono
    const bool isMonoMode = getParam(isMono) > 0.f;
    
    // convenience
    const float lfo1_0_1 = kernel->lfo1_0_1;
    const float lfo1_1_0 = kernel->lfo1_1_0;
    const float lfo2_0_1 = kernel->lfo2_0_1;
    const float lfo2_1_0 = kernel->lfo2_1_0;
    const float lfo3_0_1 = kernel->lfo3_0_1;
    const float lfo3_1_0 = kernel->lfo3_1_0;
    
    //pitchLFO common frequency coefficient
    float pitchLFOCoefficient = 1.f;
    const float semitone = 0.0594630944f; // 1 = 2^(1/12)
    if (getParam(pitchLFO) == 1.f)
        pitchLFOCoefficient = 1.f + lfo1_0_1 * semitone;
    else if (getParam(pitchLFO) == 2.f)
        pitchLFOCoefficient = 1.f + lfo2_0_1 * semitone;
    else if (getParam(pitchLFO) == 3.f)
        pitchLFOCoefficient = 1.f + lfo3_0_1 * semitone;
    
    // pitchbend coefficient
    const float pbmin = getParam(pitchbendMinSemitones);
    const float pbmax = getParam(pitchbendMaxSemitones);
    const float pbVal = getParam(pitchbend);
    float pitchbendCoefficient = 1.f;
    if (pbmin < 0.f && pbVal < 8192.f) {
        const float pbminst = pbmin * ((8191.f - pbVal) / 8191.f);
        pitchbendCoefficient = nnToHz(pbminst);
    } else if (pbmax > 0.f && pbVal >= 8192.f) {
        const float pbmaxst = pbmax * (-(8192.f - pbVal) / 8192.f);
        pitchbendCoefficient = nnToHz(pbmaxst);
    }
    
    //OSC1 frequency
    const float cachedFrequencyOsc1 = oscmorph1->freq;
    float newFrequencyOsc1 = isMonoMode ?kernel->monoFrequencySmooth :cachedFrequencyOsc1;
    newFrequencyOsc1 *= nnToHz((int)getParam(morph1SemitoneOffset));
    newFrequencyOsc1 *= getParam(detuningMultiplier) * pitchLFOCoefficient;
    newFrequencyOsc1 *= pitchbendCoefficient;
    newFrequencyOsc1 = clamp(newFrequencyOsc1, 0.f, 0.5f * sampleRate());
    oscmorph1->freq = newFrequencyOsc1;
    
    //OSC1: wavetable
    oscmorph1->wtpos = getParam(index1);
    oscmorph1->enableBandlimit = getParam(oscBandlimitEnable);

    //OSC2 frequency
    const float cachedFrequencyOsc2 = oscmorph2->freq;
    float newFrequencyOsc2 = isMonoMode ?kernel->monoFrequencySmooth :cachedFrequencyOsc2;
    newFrequencyOsc2 *= nnToHz((int)getParam(morph2SemitoneOffset));
    newFrequencyOsc2 *= getParam(detuningMultiplier) * pitchLFOCoefficient;
    newFrequencyOsc2 *= pitchbendCoefficient;

    //LFO DETUNE OSC2
    const float magicDetune = cachedFrequencyOsc2/261.6255653006f;
    if (getParam(detuneLFO) == 1.f)
        newFrequencyOsc2 += lfo1_0_1 * getParam(morph2Detuning) * magicDetune;
    else if (getParam(detuneLFO) == 2.f)
        newFrequencyOsc2 += lfo2_0_1 * getParam(morph2Detuning) * magicDetune;
    else if (getParam(detuneLFO) == 3.f)
        newFrequencyOsc2 += lfo3_0_1 * getParam(morph2Detuning) * magicDetune;
    else
        newFrequencyOsc2 += getParam(morph2Detuning) * magicDetune;
    newFrequencyOsc2 = clamp(newFrequencyOsc2, 0.f, 0.5f * sampleRate());
    oscmorph2->freq = newFrequencyOsc2;

    //OSC2: wavetable
    oscmorph2->wtpos = getParam(index2);
    oscmorph2->enableBandlimit = getParam(oscBandlimitEnable);

    //SUB OSC FREQ
    const float cachedFrequencySub = subOsc->freq;
    float newFrequencySub = isMonoMode ?kernel->monoFrequencySmooth :cachedFrequencySub;
    newFrequencySub *= getParam(detuningMultiplier) / (2.f * (1.f + getParam(subOctaveDown))) * pitchLFOCoefficient;
    newFrequencySub *= pitchbendCoefficient;
    newFrequencySub = clamp(newFrequencySub, 0.f, 0.5f * sampleRate());
    subOsc->freq = newFrequencySub;
    
    //FM OSC FREQ
    const float cachedFrequencyFM = fmOsc->freq;
    float newFrequencyFM = isMonoMode ?kernel->monoFrequencySmooth :cachedFrequencyFM;
    newFrequencyFM *= getParam(detuningMultiplier) * pitchLFOCoefficient;
    newFrequencyFM *= pitchbendCoefficient;
    newFrequencyFM = clamp(newFrequencyFM, 0.f, 0.5f * sampleRate());
    fmOsc->freq = newFrequencyFM;
    
    //FM LFO
    float fmOscIndx = getParam(fmAmount);
    if (getParam(fmLFO) == 1.f)
        fmOscIndx *= lfo1_1_0;
    else if (getParam(fmLFO) == 2.f)
        fmOscIndx *= lfo2_1_0;
    else if (getParam(fmLFO) == 3.f)
        fmOscIndx *= lfo3_1_0;
    fmOscIndx = kernel->clampedValue(fmAmount, fmOscIndx);
    fmOsc->indx = fmOscIndx;
    
    //ADSR
    adsr->atk = getParam(attackDuration);
    adsr->rel = getParam(releaseDuration);
    
    //ADSR decay LFO
    float dec = getParam(decayDuration);
    if (getParam(decayLFO) == 1.f)
        dec *= lfo1_1_0;
    else if (getParam(decayLFO) == 2.f)
        dec *= lfo2_1_0;
    else if (getParam(decayLFO) == 3.f)
        dec *= lfo3_1_0;
    dec = kernel->clampedValue(decayDuration, dec);
    adsr->dec = dec;
    
    //ADSR sustain LFO
    float sus = getParam(sustainLevel);
    adsr->sus = sus;
    
    //FILTER FREQ CUTOFF ADSR
    fadsr->atk = getParam(filterAttackDuration);
    fadsr->dec = getParam(filterDecayDuration);
    fadsr->sus = getParam(filterSustainLevel);
    fadsr->rel = getParam(filterReleaseDuration);
    
    //OSCMORPH CROSSFADE
    float crossFadePos = getParam(morphBalance);
    if (getParam(oscMixLFO) == 1.f)
        crossFadePos += lfo1_0_1;
    else if (getParam(oscMixLFO) == 2.f)
        crossFadePos += lfo2_0_1;
    else if (getParam(oscMixLFO) == 3.f)
        crossFadePos += lfo3_0_1;
    crossFadePos = clamp(crossFadePos, 0.f, 1.f);
    morphCrossFade->pos = crossFadePos;
    
    //TODO:param filterMix is hard-coded to 1.  I vote we get rid of it
    filterCrossFade->pos = getParam(filterMix);
    
    //FILTER RESONANCE LFO
    float filterResonance = getParam(resonance);
    if (getParam(resonanceLFO) == 1)
        filterResonance *= lfo1_1_0;
    else if (getParam(resonanceLFO) == 2)
        filterResonance *= lfo2_1_0;
    else if (getParam(resonanceLFO) == 3)
        filterResonance *= lfo3_1_0;
    filterResonance = kernel->clampedValue(resonance, filterResonance);
    if (getParam(filterType) == 0) {
        loPass->res = filterResonance;
    } else if (getParam(filterType) == 1) {
        // bandpass bandwidth is a different unit than lopass resonance.
        // take advantage of the range of resonance [0,1].
        const float bandwidth = 0.0625f * sampleRate() * (-1.f +
#if defined(S1_NO_DOUBLE)
            s1_exp2f
#else
            exp2
#endif
            ( clamp(1.f - filterResonance, 0.f, 1.f) ) );
        bandPass->bw = bandwidth;
    }
    
    //FINAL OUTs
    float oscmorph1_out = 0.f;
    float oscmorph2_out = 0.f;
    float osc_morph_out = 0.f;
    float subOsc_out = 0.f;
    float fmOsc_out = 0.f;
    float noise_out = 0.f;
    float filterOut = 0.f;
    float finalOut = 0.f;
    
    // osc amp adsr
    // amp was used to init the generators and is now be used for the adsr factor
    sp_adsr_compute(kernel->spp(), adsr, &internalGate, &amp);

    // filter cutoff adsr
    sp_adsr_compute(kernel->spp(), fadsr, &internalGate, &filter);
    
    // filter frequency cutoff calculation
    float filterCutoffFreq = getParam(cutoff);
    if (getParam(cutoffLFO) == 1.f)
        filterCutoffFreq *= lfo1_1_0;
    else if (getParam(cutoffLFO) == 2.f)
        filterCutoffFreq *= lfo2_1_0;
    else if (getParam(cutoffLFO) == 3.f)
        filterCutoffFreq *= lfo3_1_0;
    
    // filter frequency env lfo crossfade
    float filterEnvLFOMix = getParam(filterADSRMix);
    if (getParam(filterEnvLFO) == 1.f)
        filterEnvLFOMix *= lfo1_1_0;
    else if (getParam(filterEnvLFO) == 2.f)
        filterEnvLFOMix *= lfo2_1_0;
    else if (getParam(filterEnvLFO) == 3.f)
        filterEnvLFOMix *= lfo3_1_0;
    
    // filter frequency mixer
    filterCutoffFreq -= filterCutoffFreq * filterEnvLFOMix * (1.f - filter);
    filterCutoffFreq = kernel->clampedValue(cutoff, filterCutoffFreq);
    loPass->freq = filterCutoffFreq;
    bandPass->freq = filterCutoffFreq;
    hiPass->freq = filterCutoffFreq;
    
    //oscmorph1_out
    runtime_morph_compute(kernel->spp(), oscmorph1, nil, &oscmorph1_out);
    oscmorph1_out *= getParam(morph1Volume);
    
    //oscmorph2_out
    runtime_morph_compute(kernel->spp(), oscmorph2, nil, &oscmorph2_out);
    oscmorph2_out *= getParam(morph2Volume);

    //osc_morph_out
    sp_crossfade_compute(kernel->spp(), morphCrossFade, &oscmorph1_out, &oscmorph2_out, &osc_morph_out);
    
    //subOsc_out
    sp_osc_compute(kernel->spp(), subOsc, nil, &subOsc_out);
    if (getParam(subIsSquare)) {
        if (subOsc_out > 0.f) {
            subOsc_out = getParam(subVolume);
        } else {
            subOsc_out = -getParam(subVolume);
        }
    } else {
        // make sine louder
        subOsc_out *= getParam(subVolume) * 3.f;
    }
    
    //fmOsc_out
    sp_fosc_compute(kernel->spp(), fmOsc, nil, &fmOsc_out);
    fmOsc_out *= getParam(fmVolume);
    
    //noise_out
    sp_noise_compute(kernel->spp(), noise, nil, &noise_out);
    noise_out *= getParam(noiseVolume);
    if (getParam(noiseLFO) == 1.f)
        noise_out *= lfo1_1_0;
    else if (getParam(noiseLFO) == 2.f)
        noise_out *= lfo2_1_0;
    else if (getParam(noiseLFO) == 3.f)
        noise_out *= lfo3_1_0;

    // adsr pitch tracking
#if defined(S1_NO_DOUBLE)
    const float pitch = s1_log2f(newFrequencyOsc1 > 0 ? newFrequencyOsc1 : 261.f);
#else
    const float pitch = log2(newFrequencyOsc1 > 0 ? newFrequencyOsc1 : 261.f);
#endif
    const float ymin = 6.f;
    const float ymax = 11.f;
    const float kt0 = (pitch - ymin)/(ymax-ymin);
    float kt1 = 1.f - clamp(kt0, 0.f, 1.f);
    kt1 *= kt1;
    const float ktfloor = 1.f - getParam(adsrPitchTracking); // ??
    const float kt2 = ((1.f-ktfloor) * kt1) + ktfloor;

    //synthOut
    float synthOut = amp * kt2 * (osc_morph_out + subOsc_out + fmOsc_out + noise_out);

    //filterOut:  Always calcuate all filters so when user switches the buffers are up-to-date.
    float moogOut;
    sp_moogladder_compute(kernel->spp(), loPass, &synthOut, &moogOut);
    float bandOut;
    sp_butbp_compute(kernel->spp(), bandPass, &synthOut, &bandOut);
    float hipassOut;
    sp_buthp_compute(kernel->spp(), hiPass, &synthOut, &hipassOut);
    if (getParam(filterType) == 0.f)
        filterOut = moogOut;
    else if (getParam(filterType) == 1.f)
        filterOut = bandOut;
    else if (getParam(filterType) == 2.f)
        filterOut = hipassOut;

    // filter crossfade
    sp_crossfade_compute(kernel->spp(), filterCrossFade, &synthOut, &filterOut, &finalOut);
    
    // final output
    outL[frameIndex] += finalOut;
    outR[frameIndex] += finalOut;
    
    // restore cached values
    oscmorph1->freq = cachedFrequencyOsc1;
    oscmorph2->freq = cachedFrequencyOsc2;
    subOsc->freq = cachedFrequencySub;
    fmOsc->freq = cachedFrequencyFM;

    return reader_ok;
}
}
