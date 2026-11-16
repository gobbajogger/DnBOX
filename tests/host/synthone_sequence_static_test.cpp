// Tests real validation work, not a mock: compiler instrumentation counts the
// existing tuning_valid function only during a serialized render block.
#define main retained_compact_suite_main
#include "tests/compact_scheduler_test.cpp"
#undef main
#include "source_scheduler.hpp"
static bool count_validation = false;
static unsigned validation_calls = 0;
extern "C" __attribute__((no_instrument_function))
void __cyg_profile_func_enter(void *function, void *) {
    if (count_validation && function == reinterpret_cast<void *>(&s1source_sequence::SerializedSequence::tuning_valid))
        ++validation_calls;
}
extern "C" __attribute__((no_instrument_function))
void __cyg_profile_func_exit(void *, void *) {}
int main(int argc, char **argv) {
    CHECK(argc == 4);
    const unsigned expected_calls = unsigned(std::strtoul(argv[3], nullptr, 10));
    Fixture fixture(argv[1]);
    FILE *file = std::fopen(argv[2], "rb"); CHECK(file);
    std::vector<unsigned char> pack(165292);
    CHECK(std::fread(pack.data(), 1, pack.size(), file) == pack.size()); CHECK(!std::fclose(file));
    Tables metadata = fixture.tables; for (auto &row : metadata.morph) row.samples = nullptr;
    s1scheduler::SourceScheduler synth;
    float parameters[150]; std::memcpy(parameters, defaults, sizeof parameters);
    parameters[arpIsOn] = 1; parameters[arpIsSequencer] = 1;
    CHECK(synth.init_compact_metadata(metadata, pack.data(), pack.size(), 48000, parameters, 150, fixture.tuning, 128));
    CHECK(synth.start(60, 100)); CHECK(synth.start(67, 90));
    float left[128]{}, right[128]{};
    validation_calls = 0; count_validation = true;
    { NoHeap guard; CHECK(synth.process(128, 0, left, right)); }
    count_validation = false;
    std::printf("static_sequence_validation_calls=%u expected=%u frames=128\n", validation_calls, expected_calls); std::fflush(stdout);
    CHECK(validation_calls == expected_calls);
    // Changes between blocks require fresh admission, not a stale persistent cache.
    CHECK(synth.set_target(sequencerPattern00, 12));
    validation_calls = 0; count_validation = true;
    { NoHeap guard; CHECK(synth.process(128, 0, left, right)); }
    count_validation = false; CHECK(validation_calls == expected_calls);
    // Invalid static controls must reject before changing owner or output bytes.
    const float valid_rate = synth.parameters[arpRate]; synth.parameters[arpRate] = NAN;
    std::vector<unsigned char> before(sizeof synth); std::memcpy(before.data(), &synth, sizeof synth);
    float old_left[128], old_right[128]; std::memcpy(old_left, left, sizeof left); std::memcpy(old_right, right, sizeof right);
    CHECK(!synth.process(128, 0, left, right));
    CHECK(!std::memcmp(before.data(), &synth, sizeof synth));
    CHECK(!std::memcmp(old_left, left, sizeof left) && !std::memcmp(old_right, right, sizeof right));
    synth.parameters[arpRate] = valid_rate;
    // A clock domain can become invalid after a valid first sample. Per-sample
    // dynamic validation must still reject rather than make an out-of-range cast.
    synth.sequence.previous_held = synth.heldNoteNumbersAE.count;
    const double dt = (synth.parameters[arpRate] / 60.f) / synth.sp->sr;
    synth.sequence.beat_time = 2147483648.0 * synth.parameters[arpSeqTempoMultiplier] - 1.5 * dt;
    struct View { const s1scheduler::NoteNumber *values; size_t count; };
    const View held{synth.heldNoteNumbersAE.values.begin(), size_t(synth.heldNoteNumbersAE.count)};
    CHECK(synth.sequence.can_process(synth.parameters, 150, held));
    CHECK(!synth.process(128, 0, left, right));
    std::puts("PASS: static sequence admission and unchanged boundary checks");
}
