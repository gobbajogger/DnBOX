/* Test-only. Drive the two existing comparison frontends at 44.1 kHz.
 * The firmware neither links these original helpers nor reads this fixture. */
#include "full_fixture.hpp"
#include "source_controls.hpp"
#include "oracle_controls.hpp"

static unsigned u32(FILE *file) {
    unsigned x;
    CHECK(std::fread(&x, sizeof x, 1, file) == 1);
    return x;
}
int main(int argc, char **argv) {
    CHECK(argc == 4);
    Fixture fixture(argv[1]);
    FILE *packfile = std::fopen(argv[2], "rb");
    CHECK(packfile);
    std::vector<unsigned char> pack(165292);
    CHECK(std::fread(pack.data(), 1, pack.size(), packfile) == pack.size());
    CHECK(std::fclose(packfile) == 0);
    compact_data = pack.data(); compact_size = pack.size();
    FILE *commands = std::fopen(argv[3], "rb");
    CHECK(commands && u32(commands) == 0x53315043);
    unsigned count = u32(commands);
    CHECK(count == 694);
    std::puts("index,on,feedback,seconds,mix,bpm,sync,live_min_seconds,live_max_seconds");
    for (unsigned row = 0; row < count; ++row) {
        FullPair pair(fixture, 44100, false, false, 0);
        s1frontend::SourceControls source(pair.s);
        s1frontend_oracle::OracleControls original(pair.o);
        auto set = [&](unsigned id, float value) {
            CHECK(source.set_parameter(id,value) && original.set_parameter(id,value));
            CHECK(source.target(id) == original.target(id));
        };
        unsigned n = u32(commands);
        CHECK(n == 144);
        for (unsigned i = 0; i < n; ++i) {
            unsigned id = u32(commands); float value;
            CHECK(std::fread(&value, sizeof value, 1, commands) == 1);
            set(id, value);
        }
        std::printf("%u",row);
        for (auto id : {delayOn,delayFeedback,delayTime,delayMix,arpRate,tempoSyncToArpRate}) {
            CHECK(source.target(id) == original.target(id));
            std::printf(",%.9g",original.target(id));
        }
        set(tempoSyncToArpRate,0);
        for (auto id : {delayOn,delayFeedback,delayMix}) {
            for (float limit : {minimums[id],maximums[id]}) {
                set(id,limit);
                CHECK(original.target(id)==limit);
            }
        }
        set(delayFeedback,0);
        for (float limit : {minimums[delayTime], maximums[delayTime]}) {
            set(delayTime,limit);
            CHECK(original.target(delayTime) == limit);
            std::printf(",%.9g",limit);
        }
        std::puts("");
    }
    CHECK(std::fgetc(commands) == EOF && !std::ferror(commands));
    CHECK(std::fclose(commands) == 0);
}
