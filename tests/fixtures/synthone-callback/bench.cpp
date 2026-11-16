// Source-bound host deadline measurement, not device XRUN or full app evidence.
#include "full_fixture.hpp"
#include "source_controls.hpp"
#define S1FX_EXISTING_PARAMETERS 1
#include "source_fx.hpp"
#include <time.h>
#include <inttypes.h>
#include <cstdlib>
#if defined(__SSE__)
#include <xmmintrin.h>
#endif
struct FxContext { SourceFx *fx; float *left, *right; float mode; };
static void callback_fx(const RuntimeState &state, unsigned phase, unsigned frame, void *opaque) {
    if (phase != 1) return;
    auto &context = *static_cast<FxContext *>(opaque);
    if (context.mode != state.parameters[isMono]) {
        context.mode = state.parameters[isMono];
        context.fx->source_reset_delays();
    }
    std::memcpy(context.fx->parameters.data(), state.parameters, 150 * sizeof(float));
    context.fx->frame(context.left[frame], context.left + frame, context.right + frame,
                      state.lfo1_0_1, state.lfo1_1_0, state.lfo2_0_1, state.lfo2_1_0);
}
static uint64_t clock_ns(clockid_t clock) {
    timespec value{};
    CHECK(clock_gettime(clock, &value) == 0);
    return uint64_t(value.tv_sec) * 1000000000ull + uint64_t(value.tv_nsec);
}
static unsigned read_word(FILE *file) {
    unsigned value;
    CHECK(std::fread(&value, sizeof value, 1, file) == 1);
    return value;
}
static bool missed(uint64_t elapsed, unsigned frames, unsigned rate) {
    return elapsed > uint64_t(frames) * 1000000000ull / rate;
}
int main(int argc, char **argv) {
    CHECK(argc == 8); // original table, pack, commands, CSV, JSONL, count, diagnostic FTZ
    const bool diagnostic_ftz = std::strtoul(argv[7], nullptr, 10) != 0;
#if defined(__SSE__)
    const unsigned original_mxcsr = _mm_getcsr();
    if (diagnostic_ftz) _mm_setcsr(original_mxcsr | 0x8040u);
    std::fprintf(stderr, "host_mxcsr_initial=0x%x effective=0x%x diagnostic_ftz=%u\n",
                 original_mxcsr, _mm_getcsr(), unsigned(diagnostic_ftz));
#else
    CHECK(!diagnostic_ftz);
#endif
    const unsigned requested = unsigned(std::strtoul(argv[6], nullptr, 10));
    CHECK(requested >= 1 && requested <= 694);
    Fixture fixture(argv[1]);
    FILE *input = std::fopen(argv[2], "rb"); CHECK(input);
    std::vector<unsigned char> pack(165292);
    CHECK(std::fread(pack.data(), 1, pack.size(), input) == pack.size());
    CHECK(!std::fclose(input)); compact_data = pack.data(); compact_size = pack.size();
    input = std::fopen(argv[3], "rb"); CHECK(input && read_word(input) == 0x53315043);
    const unsigned total_programs = read_word(input); CHECK(total_programs == 694);
    FILE *csv = std::fopen(argv[4], "w"), *json = std::fopen(argv[5], "w"); CHECK(csv && json);
    std::fprintf(csv, "preset,block,frames,rate,wall_ns,cpu_ns,wall_miss,cpu_miss\n");
    uint64_t total_wall_misses = 0, total_cpu_misses = 0, total_nonfinite = 0;
    for (unsigned preset = 0; preset < requested; ++preset) {
        case_name = "source-bound SynthOne callback deadline measurement";
        SourceScheduler synth;
        Tables metadata = fixture.tables; for (auto &row : metadata.morph) row.samples = nullptr;
        CHECK(synth.init_compact_metadata(metadata, pack.data(), pack.size(), 48000,
                                         defaults, 150, fixture.tuning, 128));
        s1frontend::SourceControls controls(synth);
        SourceFx fx(48000);
        const unsigned commands = read_word(input);
        for (unsigned i = 0; i < commands; ++i) {
            const unsigned id = read_word(input); float value;
            CHECK(std::fread(&value, sizeof value, 1, input) == 1);
            CHECK(controls.set_parameter(id, value));
        }
        CHECK(synth.reset_sequencer());
        FxContext context{&fx, nullptr, nullptr, 0};
        synth.trace_fn = callback_fx; synth.trace_context = &context;
        uint64_t wall_misses = 0, cpu_misses = 0, nonfinite = 0, max_wall = 0, max_cpu = 0;
        uint64_t hash = 14695981039346656037ull;
        double energy = 0; float peak = 0;
        for (unsigned block = 0; block < 750; ++block) {
            float left[128]{}, right[128]{}; context.left = left; context.right = right;
            const uint64_t wall_start = clock_ns(CLOCK_MONOTONIC);
            const uint64_t cpu_start = clock_ns(CLOCK_THREAD_CPUTIME_ID);
            {
                NoHeap guard;
                if (block % 96 == 0) {
                    CHECK(synth.stop_all());
                    if (block < 672) {
                        const int base = 36 + int((block / 96) % 4) * 7;
                        for (int key = 0; key < 16; ++key)
                            CHECK(synth.start(base + key * 2, 64 + key * 4));
                    }
                }
                CHECK(synth.process(128, 0, left, right));
            }
            const uint64_t cpu = clock_ns(CLOCK_THREAD_CPUTIME_ID) - cpu_start;
            const uint64_t wall = clock_ns(CLOCK_MONOTONIC) - wall_start;
            const bool wm = missed(wall, 128, 48000), cm = missed(cpu, 128, 48000);
            wall_misses += wm; cpu_misses += cm; max_wall = std::max(max_wall, wall); max_cpu = std::max(max_cpu, cpu);
            std::fprintf(csv, "%u,%u,128,48000,%" PRIu64 ",%" PRIu64 ",%u,%u\n", preset, block, wall, cpu, unsigned(wm), unsigned(cm));
            for (unsigned i = 0; i < 128; ++i) for (float value : {left[i], right[i]}) {
                uint32_t word; std::memcpy(&word, &value, sizeof word);
                for (unsigned byte = 0; byte < 4; ++byte) { hash ^= (word >> (byte * 8)) & 255u; hash *= 1099511628211ull; }
                if (!std::isfinite(value)) { ++nonfinite; continue; }
                energy += double(value) * value; peak = std::max(peak, std::fabs(value));
            }
        }
        synth.trace_fn = nullptr; synth.trace_context = nullptr;
        total_wall_misses += wall_misses; total_cpu_misses += cpu_misses; total_nonfinite += nonfinite;
        std::fprintf(json, "{\"preset\":%u,\"callbacks\":750,\"frames\":96000,\"wall_misses\":%" PRIu64 ",\"cpu_misses\":%" PRIu64 ",\"max_wall_ns\":%" PRIu64 ",\"max_cpu_ns\":%" PRIu64 ",\"nonfinite_words\":%" PRIu64 ",\"peak\":%.9g,\"energy\":%.17g,\"diagnostic_fnv64\":\"%016" PRIx64 "\"}\n", preset, wall_misses, cpu_misses, max_wall, max_cpu, nonfinite, peak, energy, hash);
        std::fflush(csv); std::fflush(json);
    }
    CHECK(!std::fclose(csv) && !std::fclose(json) && !std::fclose(input));
    std::printf("{\"programs\":%u,\"callbacks\":%u,\"frames\":%u,\"wall_misses\":%" PRIu64 ",\"cpu_misses\":%" PRIu64 ",\"nonfinite_words\":%" PRIu64 ",\"scope\":\"host control/render/common FX deadline measurement, fixed fixture tuning; not device XRUN or app qualification\"}\n", requested, requested * 750, requested * 96000, total_wall_misses, total_cpu_misses, total_nonfinite);
    return total_wall_misses || total_cpu_misses || total_nonfinite ? 1 : 0;
}
