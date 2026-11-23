// New comparison-only fixture/file-I/O wrapper. No voice or filter algorithm.
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <array>
#include <cfenv>
#include <cfloat>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <exception>
#include <sys/mman.h>
#include <unistd.h>
#include "full_oracle.hpp"
#include "wave_frequencies.h"

namespace {
constexpr unsigned kRate = 44100;
constexpr unsigned kBlock = 64;
constexpr unsigned kWarmBlocks = 2757;
constexpr unsigned kRecordBlocks = 1378;
constexpr unsigned kOffBlock = 1033;
constexpr unsigned kNotes[3] = {36, 60, 84};
constexpr std::uint32_t kNoteFrequencyBits[3] = {0x4282d013u, 0x4382d013u, 0x4482d013u};
constexpr unsigned kVelocity = 100;
constexpr unsigned kControls = 150;
constexpr unsigned kTuning = 128;
constexpr std::size_t kRowSamples = 4097;
constexpr std::size_t kMorphRows = 52;
constexpr const char *kBaseline = "physical-gain/100_2000_.9";
constexpr const char *kActive = "filter-modulation/1.2_2000_0.9";
static_assert(sizeof(float) == 4 && sizeof(SPFLOAT) == 4, "binary32 storage required");
static_assert(FLT_RADIX == 2 && FLT_MANT_DIG == 24 && FLT_MAX_EXP == 128,
              "IEEE754 binary32 float required");

std::uint32_t bits(float value) {
    std::uint32_t result;
    std::memcpy(&result, &value, sizeof(result));
    return result;
}

bool read_floats(const char *path, float *destination, std::size_t count) {
    FILE *file = std::fopen(path, "rb");
    if (!file) { std::perror("open numeric input"); return false; }
    const std::size_t got = std::fread(destination, sizeof(float), count, file);
    const int trailing = std::fgetc(file);
    const int read_error = std::ferror(file);
    const int close_error = std::fclose(file);
    if (got != count || trailing != EOF || read_error || close_error) {
        std::fprintf(stderr, "numeric input has an unexpected byte length\n");
        return false;
    }
    for (std::size_t i = 0; i < count; ++i) {
        if (!std::isfinite(destination[i])) {
            std::fprintf(stderr, "nonfinite numeric input at index %zu\n", i);
            return false;
        }
    }
    return true;
}

// This owns the fixture storage and descriptors, not an engine implementation.
class TableInput {
public:
    synthone_shared::Tables tables{};
    const float *tuning = nullptr;
    TableInput() = default;
    TableInput(const TableInput &) = delete;
    TableInput &operator=(const TableInput &) = delete;
    ~TableInput() { if (mapping_ != MAP_FAILED) ::munmap(mapping_, mapped_bytes_); }

    bool load(const char *table_path, const char *tuning_path) {
        const long page = ::sysconf(_SC_PAGESIZE);
        if (page <= 0) { std::fprintf(stderr, "page size unavailable\n"); return false; }
        const std::size_t payload = ((kMorphRows + 1) * kRowSamples + kTuning) * sizeof(float);
        const std::size_t page_size = static_cast<std::size_t>(page);
        mapped_bytes_ = (payload + page_size - 1) / page_size * page_size;
        mapping_ = ::mmap(nullptr, mapped_bytes_, PROT_READ | PROT_WRITE,
                          MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (mapping_ == MAP_FAILED) { std::perror("allocate fixture mapping"); return false; }
        float *data = static_cast<float *>(mapping_);
        if (!read_floats(table_path, data, kMorphRows * kRowSamples)) return false;
        for (unsigned band = 0; band < 13; ++band) {
            tables.frequency_band[band] = wave_frequencies[band];
            for (unsigned shape = 0; shape < 4; ++shape) {
                const unsigned row = band * 4 + shape;
                tables.morph[row] = {data + row * kRowSamples, kRowSamples,
                                     band, static_cast<synthone_shared::Shape>(shape)};
            }
        }
        float *sine = data + kMorphRows * kRowSamples;
        tables.sine = sine;
        tables.sine_count = kRowSamples;
        sp_data context{};
        context.sr = static_cast<int>(kRate);
        sp_ftbl descriptor{};
        descriptor.tbl = sine;
        // Use the original public fixture helpers; implement no waveform generator here.
        if (sp_ftbl_init(&context, &descriptor, 4096) != SP_OK ||
            sp_gen_sine(&context, &descriptor) != SP_OK) {
            std::fprintf(stderr, "original public sine-table initialization failed\n");
            return false;
        }
        sine[4096] = 0.0f;
        float *frequencies = data + (kMorphRows + 1) * kRowSamples;
        if (!read_floats(tuning_path, frequencies, kTuning)) return false;
        for (unsigned i = 0; i < 3; ++i) {
            if (bits(frequencies[kNotes[i]]) != kNoteFrequencyBits[i]) {
                std::fprintf(stderr, "played-note tuning identity differs\n");
                return false;
            }
        }
        tuning = frequencies;
        if (::mprotect(mapping_, mapped_bytes_, PROT_READ) != 0) {
            std::perror("protect fixture mapping");
            return false;
        }
        return true;
    }
private:
    void *mapping_ = MAP_FAILED;
    std::size_t mapped_bytes_ = 0;
};

struct OutputFile {
    FILE *file = nullptr;
    ~OutputFile() { if (file) std::fclose(file); }
    bool close() {
        FILE *current = file;
        file = nullptr;
        return current && std::fclose(current) == 0;
    }
};

int capture(int argc, char **argv) {
    if (argc != 6) {
        std::fprintf(stderr, "usage: capture_original_dry TABLES.f32 CONTROLS.f32le TUNING.f32le NEW_OUTPUT.f32le CASE_ID\n");
        return 2;
    }
    const bool baseline = std::strcmp(argv[5], kBaseline) == 0;
    const bool active = std::strcmp(argv[5], kActive) == 0;
    if (!baseline && !active) {
        std::fprintf(stderr, "case is outside the two prepared numeric fixtures\n");
        return 2;
    }
    const std::uint32_t endian_check = 1;
    if (*reinterpret_cast<const unsigned char *>(&endian_check) != 1 ||
        std::fegetround() != FE_TONEAREST) {
        std::fprintf(stderr, "little-endian host and round-to-nearest mode required\n");
        return 2;
    }
    std::array<float, kControls> controls{};
    if (!read_floats(argv[2], controls.data(), controls.size())) return 1;
    if (bits(controls[16]) != 0x44fa0000u || bits(controls[17]) != 0x3f666666u ||
        bits(controls[19]) != (baseline ? 0x00000000u : 0x3f99999au)) {
        std::fprintf(stderr, "case cutoff/resonance/depth identity differs\n");
        return 1;
    }
    TableInput fixture;
    if (!fixture.load(argv[1], argv[3])) return 1;
    OutputFile output;
    output.file = std::fopen(argv[4], "wbx");
    if (!output.file) { std::perror("create new mono output"); return 1; }

    unsigned warmup_frames = 0, retained_frames = 0, process_calls = 0;
    unsigned warmup_nonfinite = 0, output_nonfinite = 0, notes_completed = 0;
    std::array<float, kBlock> left{}, right{};
    for (unsigned note_index = 0; note_index < 3; ++note_index) {
        // Fresh original instance for every note; never construct a candidate engine.
        s1scheduler::FullOracle original;
        if (!original.init(fixture.tables, static_cast<int>(kRate), controls.data(),
                           controls.size(), fixture.tuning, kTuning)) {
            std::fprintf(stderr, "original public initialization rejected note %u\n", kNotes[note_index]);
            return 1;
        }
        for (unsigned block = 0; block < kWarmBlocks; ++block) {
            original.process(kBlock, 0, left.data(), right.data());
            ++process_calls;
            warmup_frames += kBlock;
            for (float value : left) if (!std::isfinite(value)) ++warmup_nonfinite;
        }
        original.startNote(static_cast<int>(kNotes[note_index]), static_cast<int>(kVelocity));
        for (unsigned block = 0; block < kRecordBlocks; ++block) {
            if (block == kOffBlock) original.stopNote(static_cast<int>(kNotes[note_index]));
            original.process(kBlock, 0, left.data(), right.data());
            ++process_calls;
            retained_frames += kBlock;
            for (float value : left) if (!std::isfinite(value)) ++output_nonfinite;
            if (std::fwrite(left.data(), sizeof(float), left.size(), output.file) != left.size()) {
                std::perror("write original mono output");
                return 1;
            }
        }
        ++notes_completed;
    }
    if (!output.close()) { std::perror("close original mono output"); return 1; }
    const unsigned total_frames = warmup_frames + retained_frames;
    const int status = warmup_nonfinite == 0 && output_nonfinite == 0 ? 0 : 1;
    std::printf("{\"classification\":\"NEW_RECONSTRUCTED_ORIGINAL_DRY_CAPTURE\","
                "\"case_id\":\"%s\",\"sample_rate_hz\":%u,\"mono_channel\":\"original_left\","
                "\"notes\":[36,60,84],\"velocity\":100,\"notes_completed\":%u,"
                "\"fresh_original_per_note\":true,\"controls_from_pinned_physical_vector\":true,"
                "\"warmup_frames\":%u,\"retained_frames\":%u,\"total_reference_frames\":%u,"
                "\"process_calls\":%u,\"block_size\":64,\"note_off_frame_per_note\":66112,"
                "\"warmup_nonfinite_count\":%u,\"output_nonfinite_count\":%u,\"status\":%d}\n",
                baseline ? kBaseline : kActive, kRate, notes_completed, warmup_frames,
                retained_frames, total_frames, process_calls, warmup_nonfinite,
                output_nonfinite, status);
    return status;
}
} // namespace

int main(int argc, char **argv) {
    try { return capture(argc, argv); }
    catch (const std::exception &error) {
        std::fprintf(stderr, "reference wrapper exception: %s\n", error.what());
        return 1;
    }
}
