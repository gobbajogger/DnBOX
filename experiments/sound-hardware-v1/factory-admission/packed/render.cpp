/* SPDX-License-Identifier: GPL-3.0-only */
#include "full_fixture.hpp"
#include "source_controls.hpp"
#define S1FX_EXISTING_PARAMETERS 1
#include "source_fx.hpp"
#include "bank.h"

struct Output {
    SourceFx effects{48000};
    float left[128]{}, right[128]{};
    float mono = 0;
};

static void post_fx(const RuntimeState &voice, unsigned phase, unsigned frame, void *context)
{
    if (phase != 1)
        return;
    Output &out = *static_cast<Output *>(context);
    if (out.mono != voice.parameters[isMono]) {
        out.mono = voice.parameters[isMono];
        out.effects.source_reset_delays();
    }
    std::memcpy(out.effects.parameters.data(), voice.parameters, 150 * sizeof(float));
    out.effects.frame(out.left[frame], out.left + frame, out.right + frame,
                      voice.lfo1_0_1, voice.lfo1_1_0, voice.lfo2_0_1, voice.lfo2_1_0);
}

int main(int argc, char **argv)
{
    CHECK(argc == 7);
    Fixture fixture(argv[1]);
    FILE *pack_file = std::fopen(argv[2], "rb");
    CHECK(pack_file);
    std::vector<unsigned char> pack(165292);
    CHECK(std::fread(pack.data(), 1, pack.size(), pack_file) == pack.size());
    CHECK(!std::fclose(pack_file));
    compact_data = pack.data();
    compact_size = pack.size();

    int program = s1_factory_lookup(argv[3], std::stoul(argv[4]));
    CHECK(program >= 0 && !std::strcmp(s1_factory_name(program), argv[5]));
    // Use the original two-second comparison's exact start state and stimulus.
    FullPair pair(fixture, 48000, false, false, 0);
    SourceScheduler &engine = pair.s;
    s1frontend::SourceControls controls(engine);
    for (unsigned i = 0; i < S1_FACTORY_COMMANDS; ++i) {
        unsigned id;
        float physical;
        NoHeap guard;
        CHECK(s1_factory_read(program, i, &id, &physical));
        CHECK(controls.set_parameter(id, physical));
    }
    CHECK(engine.reset_sequencer());
    Output out;
    engine.trace_fn = post_fx;
    engine.trace_context = &out;
    const int notes[] = {48,55,60,64,67,72};
    FILE *pcm = std::fopen(argv[6], "wb");
    CHECK(pcm);
    for (unsigned at = 0; at < 96000;) {
        if (at % 12000 == 0) {
            CHECK(engine.stop_all());
            if (at < 72000)
                CHECK(engine.start(notes[at / 12000], 64 + 11 * (at / 12000)));
        }
        unsigned n = std::min(128u, 12000u - at % 12000);
        std::fill_n(out.left, n, 0.f);
        std::fill_n(out.right, n, 0.f);
        {
            NoHeap guard;
            CHECK(engine.process(n, 0, out.left, out.right));
        }
        float stereo[256];
        for (unsigned i = 0; i < n; ++i) {
            CHECK(std::isfinite(out.left[i]) && std::isfinite(out.right[i]));
            stereo[2*i] = out.left[i];
            stereo[2*i+1] = out.right[i];
        }
        CHECK(std::fwrite(stereo, sizeof(float)*2, n, pcm) == n);
        at += n;
    }
    CHECK(!std::fclose(pcm));
}
