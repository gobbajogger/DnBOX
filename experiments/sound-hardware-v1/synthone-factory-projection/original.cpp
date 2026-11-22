// SPDX-License-Identifier: GPL-3.0-only
#include "full_fixture.hpp"
#include "oracle_controls.hpp"
#define S1FX_EXISTING_PARAMETERS 1
#include "source_fx.hpp"

struct Output {
    SourceFx fx{44100};
    float *left{}, *right{}, mono{};
};

static void apply_effects(const OracleState &state, unsigned phase, unsigned frame, void *raw)
{
    if (phase != 1) return;
    auto &out = *static_cast<Output *>(raw);
    if (out.mono != state.parameters[isMono]) {
        out.mono = state.parameters[isMono];
        out.fx.source_reset_delays();
    }
    std::memcpy(out.fx.parameters.data(), state.parameters, 150 * sizeof(float));
    out.fx.frame(out.left[frame], out.left + frame, out.right + frame,
                 state.lfo1_0_1, state.lfo1_1_0, state.lfo2_0_1, state.lfo2_1_0);
}

static unsigned word(FILE *source)
{
    unsigned value{};
    CHECK(std::fread(&value, sizeof value, 1, source) == 1);
    return value;
}

int main(int argc, char **argv)
{
    CHECK(argc == 6);
    const unsigned selected = std::stoul(argv[4]);
    Fixture fixture(argv[1]);
    FILE *pack_file = std::fopen(argv[2], "rb");
    std::vector<unsigned char> pack(165292);
    CHECK(pack_file && std::fread(pack.data(), 1, pack.size(), pack_file) == pack.size());
    CHECK(std::fclose(pack_file) == 0);
    compact_data = pack.data();
    compact_size = pack.size();

    FullPair pair(fixture, 44100, false, false, 0);
    s1frontend_oracle::OracleControls controls(pair.o);
    FILE *commands = std::fopen(argv[3], "rb");
    CHECK(commands && word(commands) == 0x53315043 && word(commands) == 694 && selected < 694);
    CHECK(std::fseek(commands, long(selected) * (1 + 2 * 144) * 4, SEEK_CUR) == 0);
    CHECK(word(commands) == 144);
    for (unsigned i = 0; i < 144; ++i) {
        const unsigned id = word(commands);
        float physical;
        CHECK(std::fread(&physical, sizeof physical, 1, commands) == 1);
        CHECK(controls.set_parameter(id, physical));
    }
    CHECK(std::fclose(commands) == 0);
    pair.o.resetSequencer();
    Output out;
    FILE *pcm = std::fopen(argv[5], "wb");
    CHECK(pcm);
    pair.o.trace_fn = apply_effects;
    pair.o.trace_context = &out;
    for (unsigned at = 0; at < 132300;) {
        if (at == 0) pair.o.startNote(54, 100);
        if (at == 22050) pair.o.startNote(61, 93);
        if (at == 44100) { pair.o.stopAllNotes(); pair.o.startNote(66, 116); }
        if (at == 66150) pair.o.stopAllNotes();
        float left[128]{}, right[128]{}, stereo[256];
        const unsigned n = std::min(128u, 22050 - at % 22050);
        out.left = left;
        out.right = right;
        compact_oracle_window = true;
        pair.o.process(n, 0, left, right);
        compact_oracle_window = false;
        for (unsigned j = 0; j < n; ++j) {
            stereo[2*j] = left[j];
            stereo[2*j+1] = right[j];
        }
        CHECK(std::fwrite(stereo, sizeof(float), 2*n, pcm) == 2*n);
        at += n;
    }
    CHECK(std::fclose(pcm) == 0);
}
