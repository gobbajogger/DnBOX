// Derived original method bodies: Copyright2018AudioKit/AureliusProchazka. MIT; see notices/SynthOne-MIT.txt.
#include "source_controls.hpp"
#include <cstdio>
#define S1_DEPENDENT_PARAM_TAPER (0.4f)
namespace dnbox_s1frontend {
void SourceControls::setSynthParameter(S1Parameter param, float inputValue) {
    _setSynthParameterHelper(param, inputValue, true, 0);
}

void SourceControls::_rateHelper(S1Parameter parameter, float inputValue, bool notifyMainThread, int payload) {

    // pitchbend
    if (parameter == pitchbend) {
        const float val = clampedValue(parameter, inputValue);
        const float val01 = (val - minimum(pitchbend)) / (maximum(pitchbend) - minimum(pitchbend));
        _pitchbend = {parameter, val01, val, payload};
        _setSynthParameter(parameter, val);
        if (notifyMainThread) {
            dependentParameterDidChange(_pitchbend);
        }
        return;
    }

    // arpSeqTempoMultiplier
    if (parameter == arpSeqTempoMultiplier) {
        const float value = clampedValue(parameter, inputValue);
        S1RateArgs syncdValue = _rate.nearestFactor(value);
        _setSynthParameter(parameter, syncdValue.value);
        _arpSeqTempoMultiplier = {parameter, 1.f - syncdValue.value01, syncdValue.value, payload};
        if (notifyMainThread) {
            dependentParameterDidChange(_arpSeqTempoMultiplier);
        }
        return;
    }

    // lfo1Rate, lfo2Rate, autoPanFrequency
    if (parameters[tempoSyncToArpRate] > 0.f) {
        // tempo sync
        if (parameter == lfo1Rate || parameter == lfo2Rate || parameter == autoPanFrequency) {
            const float value = clampedValue(parameter, inputValue);
            S1RateArgs syncdValue = _rate.nearestFrequency(value, parameters[arpRate], minimum(parameter), maximum(parameter));
            _setSynthParameter(parameter, syncdValue.value);
            DependentParameter outputDP = {S1Parameter::S1ParameterCount, 0.f, 0.f, 0};
            switch(parameter) {
                case lfo1Rate:
                    outputDP = _lfo1Rate = {parameter, syncdValue.value01, syncdValue.value, payload};
                    break;
                case lfo2Rate:
                    outputDP = _lfo2Rate = {parameter, syncdValue.value01, syncdValue.value, payload};
                    break;
                case autoPanFrequency:
                    outputDP = _autoPanRate = {parameter, syncdValue.value01, syncdValue.value, payload};
                    break;
                default:
                    break;
            }
            if (notifyMainThread) {
                dependentParameterDidChange(outputDP);
            }
        } else if (parameter == delayTime) {
            const float value = clampedValue(parameter, inputValue);
            S1RateArgs syncdValue = _rate.nearestTime(value, parameters[arpRate], minimum(parameter), maximum(parameter));
            _setSynthParameter(parameter, syncdValue.value);
            _delayTime = {parameter, 1.f - syncdValue.value01, syncdValue.value, payload};
            if (notifyMainThread) {
                dependentParameterDidChange(_delayTime);
            }
        }
    } else {
        // no tempo sync
        _setSynthParameter(parameter, inputValue);
        const float val = parameters[parameter];
        const float min = minimum(parameter);
        const float max = maximum(parameter);
        const float val01 = clamp((val - min) / (max - min), 0.f, 1.f);
        if (parameter == lfo1Rate || parameter == lfo2Rate || parameter == autoPanFrequency || parameter == delayTime) {
            DependentParameter outputDP = {S1Parameter::S1ParameterCount, 0.f, 0.f, 0};
            switch(parameter) {
                case lfo1Rate:
                    outputDP = _lfo1Rate = {parameter, val01, val, payload};
                    break;
                case lfo2Rate:
                    outputDP = _lfo2Rate = {parameter, val01, val, payload};
                    break;
                case autoPanFrequency:
                    outputDP = _autoPanRate = {parameter, val01, val, payload};
                    break;
                case delayTime:
                    outputDP = _delayTime = {parameter, val01, val, payload};
                    break;
                default:
                    break;
            }
            if (notifyMainThread) {
                outputDP = {parameter, taper01Inverse(outputDP.normalizedValue, S1_DEPENDENT_PARAM_TAPER), outputDP.value, payload};
                dependentParameterDidChange(outputDP);
            }
        }
    }
}

void SourceControls::_setSynthParameterHelper(S1Parameter parameter, float inputValue, bool notifyMainThread, int payload) {
    if (parameter == tempoSyncToArpRate || parameter == arpRate) {
        _setSynthParameter(parameter, inputValue);
        _rateHelper(lfo1Rate, getSynthParameter(lfo1Rate), notifyMainThread, payload);
        _rateHelper(lfo2Rate, getSynthParameter(lfo2Rate), notifyMainThread, payload);
        _rateHelper(autoPanFrequency, getSynthParameter(autoPanFrequency), notifyMainThread, payload);
        _rateHelper(delayTime, getSynthParameter(delayTime), notifyMainThread, payload);
    } else if (parameter == lfo1Rate ||
               parameter == lfo2Rate ||
               parameter == autoPanFrequency ||
               parameter == delayTime ||
               parameter == pitchbend ||
               parameter == arpSeqTempoMultiplier) {
        // dependent params
        _rateHelper(parameter, inputValue, notifyMainThread, payload);
    } else {
        // special case for updating the tuning table based on frequency at A4.
        // see https://en.wikipedia.org/wiki/A440_(pitch_standard)
        if (parameter == frequencyA4) {
            _setSynthParameter(parameter, truncf(inputValue));
        } else if (parameter == portamentoHalfTime) {
            _setSynthParameter(parameter, inputValue);
            const float actualValue = getParameter(portamentoHalfTime);
            updatePortamento(actualValue);
        } else {
            // all remaining independent params
            _setSynthParameter(parameter, inputValue);
        }
    }
}




float SourceControls::getDependentParameter(S1Parameter parameter) {

    if (parameter == pitchbend) {
        return _pitchbend.value;
    }

    if (parameter == arpSeqTempoMultiplier) {
        return _arpSeqTempoMultiplier.normalizedValue;
    }

    DependentParameter dp;
    switch(parameter) {
        case lfo1Rate:              dp = _lfo1Rate;              break;
        case lfo2Rate:              dp = _lfo2Rate;              break;
        case autoPanFrequency:      dp = _autoPanRate;           break;
        case delayTime:             dp = _delayTime;             break;
        default:return 0.f;
    }

    if (parameters[tempoSyncToArpRate] > 0.f) {
        return dp.normalizedValue;
    } else {
        return taper01Inverse(dp.normalizedValue, S1_DEPENDENT_PARAM_TAPER);
    }
}

// map normalized input to parameter range
void SourceControls::setDependentParameter(S1Parameter param, float inputValue01, int payload) {
    const bool notify = true;

    switch(param) {

        case lfo1Rate: case lfo2Rate: case autoPanFrequency:
            if (parameters[tempoSyncToArpRate] > 0.f) {
                // tempo sync
                AKSynthOneRate rate = _rate.rateFromFrequency01(inputValue01);
                const float val = _rate.frequency(getSynthParameter(arpRate), rate);
                _setSynthParameterHelper(param, val, notify, payload);
            } else {
                // no tempo sync
                const float min = minimum(param);
                const float max = maximum(param);
                const float taperValue01 = taper01(inputValue01, S1_DEPENDENT_PARAM_TAPER);
                const float val = min + taperValue01 * (max - min);
                _setSynthParameterHelper(param, val, notify, payload);
            }
            break;

        case delayTime:
            if (parameters[tempoSyncToArpRate] > 0.f) {
                // tempo sync
                const float valInvert = 1.f - inputValue01;
                AKSynthOneRate rate = _rate.rateFromTime01(valInvert);
                const float val = _rate.time(parameters[arpRate], rate);
                _setSynthParameterHelper(delayTime, val, notify, payload);
            } else {
                // no tempo sync
                const float min = minimum(delayTime);
                const float max = maximum(delayTime);
                const float taperValue01 = taper01(inputValue01, S1_DEPENDENT_PARAM_TAPER);
                const float val = min + taperValue01 * (max - min);
                _setSynthParameterHelper(delayTime, val, notify, payload);
            }
            break;

        case arpSeqTempoMultiplier:
        {
            const float valInvert = 1.f - inputValue01;
            AKSynthOneRate rate = _rate.rateFromFactor01(valInvert);
            const float val = _rate.factorForRate(rate);
            _setSynthParameterHelper(arpSeqTempoMultiplier, val, notify, payload);
        }
            break;
            
        case pitchbend:
        {
            const float min = minimum(param);
            const float max = maximum(param);
            const float val = min + inputValue01 * (max - min);
            _setSynthParameterHelper(pitchbend, val, notify, payload);
        }
            break;
        default:
            return;
    }
}

float SourceControls::taper01(float inputValue01, float taper) {
    return powf(inputValue01, 1.f / taper);
}
float SourceControls::taper01Inverse(float inputValue01, float taper) {
    return powf(inputValue01, taper);
}


}
