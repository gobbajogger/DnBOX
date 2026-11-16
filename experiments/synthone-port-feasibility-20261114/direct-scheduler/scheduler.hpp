// Caller-owned direct-event pre-FX Synth One scheduler. See README and notices.
#pragma once
#include "shared_graph.hpp"
#include "parameter_ports.hpp"
#include "fixed_array.hpp"
#include "reader/direct_reader.h"
#include "compact-reader/compact.h"
#include <stddef.h>

namespace s1scheduler {
struct NoteNumber { int noteNumber, transpose, velocity; };
struct Held {
    FixedArray<NoteNumber,128> values{};
    int count = 0;
    NoteNumber &operator[](size_t i) { return values[i]; }
    const NoteNumber &operator[](size_t i) const { return values[i]; }
    void remove(size_t i);
    bool remove_checked(size_t i);
    bool insert_checked(NoteNumber value,size_t i);
    bool valid_count() const { return count>=0 && count<=128; }
    void insert(NoteNumber value, size_t i);
    void clear() { count = 0; }
};
// Deliberately only the sequencer reset(true), arp-off, empty-pending domain.
// No arp algorithm or timing is supplied by this adapter.
struct EmptySequencer {
    bool pending_empty = true;
    unsigned resets = 0;
    void reset(bool) { pending_empty = true; ++resets; }
    void process(float *, const Held &) const;
    bool process_checked(const float *,const Held &) const;
};
struct InstanceSpan { const void *data; size_t bytes; };
// Own source-zero-phase initializer; original general Soundpipe init is unchanged.
int init_morph_checked(sp_data*,sp_oscmorph2d*,sp_ftbl**,int,int,float*,SPFLOAT);
class State : public S1DSPKernel {
public:
    State(const State &) = delete;
    State &operator=(const State &) = delete;
    void initializeNoteStates();
    float tuningTableNoteToHz(int note) const;
    void heldNotesDidChange() {}
    void playingNotesDidChange() {}
    void beatCounterDidChange() {}

    FixedArray<S1NoteState,6> poly{};
    FixedArray<S1NoteState,6> *noteStates = &poly;
    S1NoteState mono{};
    S1NoteState *monoNote = &mono;
    int polyphony = 6, playingNoteStatesIndex = 0;
    Held heldNoteNumbers{}, heldNoteNumbersAE{};
    /* Binary32 image of 440*2^(-9/12). The old double exp2 rounded to this. */
    float monoFrequency = 261.625579833984375f;
    float previousProcessMonoPolyStatus = 0;
    sp_port *monoFrequencyPort = &mono_port_;
    const float *tuningTable = nullptr;
    bool resetted = false, initializedNoteStates = false;
    float processSampleCounter = 0;
    EmptySequencer sequencer{};
    bool initialized() const { return initialized_; }
    bool valid_parameter(unsigned, float) const;
    bool valid_event(int pitch, int velocity, float frequency) const;
protected:
    template<int (*Init)(sp_data*,sp_oscmorph2d*,sp_ftbl**,int,int,float*,SPFLOAT)>
    void initialize_note_objects();
    State() = default;
    State(State &&) = delete;
    State &operator=(State &&) = delete;
    bool valid_init(const synthone_shared::Tables &, int, const float *, size_t,
                    const float *, size_t, InstanceSpan, bool full_sequence=false) const;
    bool valid_restore(const float *, size_t, InstanceSpan, bool full_sequence=false) const;
    bool valid_output_span(float *, float *, unsigned, unsigned, InstanceSpan, bool full_sequence=false) const;
    void init_graph(const synthone_shared::Tables &, int, const float *);
    void restore_graph();
    struct NoteObjects {
        sp_adsr adsr{}, fadsr{};
        sp_oscmorph2d morph1{}, morph2{};
        sp_crossfade morph_crossfade{}, filter_crossfade{};
        sp_osc sub{}; sp_fosc fm{}; sp_noise noise{};
        sp_moogladder lowpass{}; sp_butbp bandpass{}; sp_buthp highpass{};
    } objects_[7]{};
    sp_phasor phasor1_{}, phasor2_{};
    sp_port port1_{}, port2_{}, mono_port_{};
    sp_data context_{};
    float context_output_ = 0;
    sp_ftbl descriptors_[53]{};
    bool initialized_ = false;
};

// Only the compact control representation belongs to the runtime.
class RuntimeState : public State {
public:
    void initializeNoteStates();
    synthone_ports::Ports compactPorts;
    using Trace = void (*)(const RuntimeState &,unsigned,unsigned,void *);
    Trace trace_fn = nullptr;
    void *trace_context = nullptr;
    void trace(unsigned phase, unsigned frame) const {
        if(trace_fn) trace_fn(*this,phase,frame,trace_context);
    }
    bool using_reader() const { return reader_active_; }
    bool using_compact() const { return compact_active_; }
    const s1c_binding &compact_binding() const { return compact_; }
    const s1_morph_binding &reader_binding() const { return reader_; }
protected:
    RuntimeState() = default;
    bool init_runtime(const synthone_shared::Tables &, int, const float *, size_t,
                      const float *, size_t, InstanceSpan);
    bool restore_runtime(const float *, size_t, InstanceSpan, bool full_sequence=false);
    bool init_reader_runtime(const synthone_shared::Tables &, const void *, size_t,
                      int, const float *, size_t, const float *, size_t, InstanceSpan);
    bool valid_runtime_output(float *,float *,unsigned,unsigned,InstanceSpan,bool full_sequence=false) const;
    bool init_compact_runtime(const synthone_shared::Tables &,const void *,size_t,int,const float *,size_t,const float *,size_t,InstanceSpan,bool full_sequence=false);
    s1c_binding compact_{};
    bool compact_active_ = false;
    s1_morph_binding reader_{};
    bool reader_active_ = false;
};

// Concrete wrappers authenticate the entire final object, not its common base.
class Scheduler final : public RuntimeState {
public:
    Scheduler() = default;
    Scheduler(const Scheduler &) = delete;
    Scheduler &operator=(const Scheduler &) = delete;
    Scheduler(Scheduler &&) = delete;
    Scheduler &operator=(Scheduler &&) = delete;
    bool init(const synthone_shared::Tables &tables, int rate, const float *values,
              size_t count, const float *tuning, size_t tuning_count) {
        return init_runtime(tables,rate,values,count,tuning,tuning_count,{this,sizeof *this});
    }
    // First reader slice retains all real Tables pointers and validates their
    // exact packed address mapping. This is not a metadata-only codec binding.
    bool init_reader(const synthone_shared::Tables &tables,const void *packed,size_t bytes,
              int rate,const float *values,size_t count,const float *tuning,size_t tuning_count) {
        return init_reader_runtime(tables,packed,bytes,rate,values,count,tuning,tuning_count,{this,sizeof *this});
    }
    // Metadata-only compact reader stage; no runtime morph payload pointers.
    // tables has52 NULL morph samples with exact4097-count/shape/band metadata,
    // original13 bands and a real4097-float sine. packed is fully cold admitted
    // structurally; canonical digest/source authentication is the loader's job.
    // ALL borrowed inputs must be live/mapped/readable and disjoint from this
    // WHOLE final owner. Values and Tables are borrowed only during init;
    // packed/sine/tuning stay immutable/live until successful reinit or teardown.
    // False preserves the whole prior owner. Objects cannot relocate or race.
    // Output guards cover whole owner, packed/sine/tuning including offset
    // prefix; no full-block rollback is promised after illegal input mutation.
    // Caller must not forge/mutate internal pointers/metadata or binding.
    bool init_compact_metadata(const synthone_shared::Tables &tables,const void *packed,size_t bytes,
              int rate,const float *values,size_t count,const float *tuning,size_t tuning_count) {
        return init_compact_runtime(tables,packed,bytes,rate,values,count,tuning,tuning_count,{this,sizeof *this});
    }
    // Source restoreValues note/control boundary only; absent FX/rates excluded.
    bool restore(const float *values, size_t count) {
        return restore_runtime(values,count,{this,sizeof *this});
    }
    bool valid_output(float *l, float *r, unsigned n, unsigned offset) const {
        return valid_runtime_output(l,r,n,offset,{this,sizeof *this});
    }
    bool set_target(unsigned parameter, float resolved_value);
    bool start(int pitch, int velocity, float ignored_frequency = 1.f);
    bool on(int pitch, int velocity, float explicit_frequency);
    bool on(int pitch, int velocity);
    bool stop(int pitch);
    bool off(int pitch);
    bool stop_all();
    bool reset();
    bool reset_dsp();
    bool reset_sequencer();
    bool process(unsigned frames, unsigned offset, float *left, float *right);
private:
    void on_valid(int,int,float);
    void off_valid(int);
};
} // namespace s1scheduler
