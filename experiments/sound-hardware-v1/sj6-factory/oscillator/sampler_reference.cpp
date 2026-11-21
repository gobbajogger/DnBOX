// Linux comparison harness for the pinned source-derived sampler, outside the firmware.
#include "CoreSampler.h"
#include "SamplerVoice.h"
#include <algorithm>
#include <cstddef>
#include <cstring>

extern "C" int sampler_reference(unsigned shape, float width, unsigned note,
                                 float *increment, float *out, size_t count)
{
    if (shape > 3 || !(width > 0 && width <= .5f) || note > 127 || !count) return 0;

    // The sampler requires a registered key map before it admits a note.
    // Every PCM word is silent, so an accidental sample-playback path is observable.
    unsigned char silence[256] = {};
    SampleDescriptor key = {};
    key.noteNumber = 60;
    key.noteFrequency = 261.6255653f;
    key.maximumNoteNumber = 127;
    key.maximumVelocity = 127;
    key.isLooping = true;
    key.loopStartPoint = 16;
    key.loopEndPoint = 100;
    key.endPoint = 127;

    CoreSampler sampler;
    if (sampler.init(44100) ||
        !sampler.loadBorrowedPcm16(key, 44100, 1, 128, silence, sizeof silence)) return 0;
    sampler.buildKeyMap();
    sampler.isOSC = true;
    sampler.oscSine = shape == 0;
    sampler.oscTriangle = shape == 1;
    sampler.oscSaw = shape == 2;
    sampler.oscPWM = shape == 3;
    sampler.oscPWMWidth = width;
    sampler.setADSRAttackDurationSeconds(0);
    sampler.setADSRHoldDurationSeconds(0);
    sampler.setADSRDecayDurationSeconds(0);
    sampler.setADSRSustainFraction(1);
    sampler.playNote(note, 127);
    auto *voice = sampler.voicePlayingNote(note);
    if (!voice) return 0;

    for (size_t start = 0; start < count; start += CORESAMPLER_CHUNKSIZE) {
        unsigned frames = std::min<size_t>(count - start, CORESAMPLER_CHUNKSIZE);
        float left[CORESAMPLER_CHUNKSIZE] = {}, right[CORESAMPLER_CHUNKSIZE] = {};
        float *channels[] = {left, right};
        sampler.render(2, frames, channels);
        if (std::memcmp(left, right, frames * sizeof(float))) return 0;
        std::memcpy(out + start, left, frames * sizeof(float));
    }
    *increment = voice->synthOsc.increment;
    return *increment > 0;
}
