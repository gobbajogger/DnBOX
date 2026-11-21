// SPDX-License-Identifier: GPL-3.0-only
// WIP adapter for the pinned, caller-owned original note/sequence/arp renderer.
// Requires the experiment's verified include/dependency closure. No app bank is
// activated here. Original post-note FX and full storage/resource admission are
// deliberately not claimed by this pre-FX adapter.
#pragma once
#include "source_scheduler.hpp"

namespace dnbox {
class SynthOneRenderer final {
public:
    SynthOneRenderer() = default;
    SynthOneRenderer(const SynthOneRenderer &) = delete;
    SynthOneRenderer &operator=(const SynthOneRenderer &) = delete;

    bool prepare(const synthone_shared::Tables &metadata,
                 const void *packed, size_t packed_bytes, int sample_rate,
                 const float *physical_values, size_t value_count,
                 const float *tuning, size_t tuning_count) {
        return engine_.init_compact_metadata(metadata, packed, packed_bytes,
            sample_rate, physical_values, value_count, tuning, tuning_count);
    }
    // Source-resolved physical values, not generic normalized knobs.
    bool parameter(unsigned id, float value) { return engine_.set_target(id, value); }
    bool restore(const float *values, size_t count) { return engine_.restore(values, count); }
    // Public key events must update original held-key/sequence state. The
    // lower-level on/off calls are sequencer sinks and bypass that state.
    bool note_on(int note, int velocity) { return engine_.start(note, velocity); }
    bool note_off(int note) { return engine_.stop(note); }
    bool all_notes_off() { return engine_.stop_all(); }
    bool reset() { return engine_.reset(); }
    bool render(unsigned frames, float *left, float *right) {
        return engine_.process(frames, 0, left, right);
    }
    bool notes_per_octave(int count) { return engine_.set_notes_per_octave(count); }
    int beat_count() const { return engine_.beat_count(); }

private:
    s1scheduler::SourceScheduler engine_;
};
} // namespace dnbox
