// SPDX-License-Identifier: GPL-3.0-only
// Host-only event comparison against the archived public CoreSampler.
#include "CoreSampler.h"
#include <cassert>
#include <cmath>
#include <cstdio>

static void render_sounding(CoreSampler &sampler, unsigned frames)
{
    double energy = 0;
    for (unsigned i = 0; i < frames; i += 16) {
        float left[16] = {}, right[16] = {};
        float *channels[] = {left, right};
        sampler.render(2, 16, channels);
        for (float value : left) {
            assert(std::isfinite(value));
            energy += value * value;
        }
    }
    assert(energy > 0);
}

int main()
{
    for (unsigned pedal = 0; pedal < 2; ++pedal) {
        float data[256];
        for (unsigned i = 0; i < 256; ++i)
            data[i] = .5f * std::sin(i * (6.283185307179586 / 256));
        CoreSampler sampler;
        sampler.init(44100);
        SampleDataDescriptor descriptor = {};
        descriptor.sampleDescriptor = {60, 261.6255653f, 0, 127, 0, 127, true, 2, 254, 0, 255};
        descriptor.sampleRate = 44100;
        descriptor.channelCount = 1;
        descriptor.sampleCount = 256;
        descriptor.data = data;
        sampler.loadSampleData(descriptor);
        sampler.buildKeyMap();
        sampler.setLoopThruRelease(true);
        sampler.setADSRReleaseDurationSeconds(1.f);

        sampler.playNote(67, 92);
        sampler.playNote(60, 100);
        render_sounding(sampler, 2048);
        if (pedal) sampler.sustainPedal(true);
        sampler.stopNote(60, false);
        render_sounding(sampler, 64);
        assert(sampler.voicePlayingNote(60));
        sampler.playNote(60, 83);
        render_sounding(sampler, 512);

        // Each immediate stop removes the first matching voice.
        unsigned matches = 0;
        while (sampler.voicePlayingNote(60)) {
            sampler.stopNote(60, true);
            ++matches;
            assert(matches < 64);
        }
        assert(matches == 1 && sampler.voicePlayingNote(67));
        std::printf("pedal=%u sounding voices on re-keyed pitch=%u; companion retained\n", pedal, matches);
    }
}
