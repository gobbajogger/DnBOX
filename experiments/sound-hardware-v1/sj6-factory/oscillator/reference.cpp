// Host-only probe for the pinned, separately compiled comparison reference.
#include "OriginalAnalyticOscillator.h"
#include <cstddef>

extern "C" int reference_oscillator(unsigned shape, float width, float increment,
                                    float *samples, size_t count)
{
    if (shape > 3 || !(width > 0 && width <= .5f) || !(increment > 0 && increment < .49f)) return 0;
    DunneCore::OriginalAnalyticOscillator oscillator{};
    oscillator.reset();
    oscillator.increment = increment;
    for (size_t i = 0; i < count; ++i) {
        switch (shape) {
        case 0: samples[i] = oscillator.processSine(1.f); break;
        case 1: samples[i] = oscillator.processTriangle(1.f); break;
        case 2: samples[i] = oscillator.processSaw(1.f); break;
        case 3: samples[i] = oscillator.processPWM(1.f, width); break;
        }
    }
    return 1;
}
