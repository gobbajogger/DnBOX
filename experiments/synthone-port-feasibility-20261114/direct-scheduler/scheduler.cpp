#include "scheduler.hpp"
#include "reader/runtime_note.hpp"
#include "compact_note.hpp"
#include "wave_frequencies.h"
#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

namespace s1scheduler {
void Held::remove(size_t i) {
    assert(i < static_cast<size_t>(count));
    for (size_t n=i+1;n<static_cast<size_t>(count);++n) values[n-1]=values[n];
    --count;
}
void Held::insert(NoteNumber value,size_t i) {
    assert(count<128 && i<=static_cast<size_t>(count));
    for(size_t n=count;n>i;--n) values[n]=values[n-1];
    values[i]=value; ++count;
}
void EmptySequencer::process(float *p,const Held &) const {
    assert(p[arpIsOn]==0.f && pending_empty);
}
namespace {
bool overlap(const void *a,size_t an,const void *b,size_t bn) {
    const auto x=reinterpret_cast<uintptr_t>(a), y=reinterpret_cast<uintptr_t>(b);
    if(x>UINTPTR_MAX-an || y>UINTPTR_MAX-bn) return true;
    return x<y+bn && y<x+an;
}
bool payload(const float *p,size_t n,float bound,InstanceSpan self) {
    if(!p || n!=4097 || overlap(p,n*sizeof(float),self.data,self.bytes)) return false;
    for(size_t i=0;i<n;++i) if(!isfinite(p[i]) || p[i]<-bound || p[i]>bound) return false;
    return p[4096]==0.f && !signbit(p[4096]);
}
bool values_valid(const float *p,size_t n,bool full_sequence=false) {
    if(!p || n!=150 || (!full_sequence && p[arpIsOn]!=0)) return false;
    for(unsigned i=0;i<150;++i)
        if(!isfinite(p[i]) || p[i]<minimums[i] || p[i]>maximums[i]) return false;
    return true;
}
}
bool State::valid_init(const synthone_shared::Tables &tables,int rate,const float *values,
                       size_t count,const float *tuning,size_t tuning_count,InstanceSpan self,bool full_sequence) const {
    if((rate!=44100 && rate!=48000 && rate!=96000) ||
       overlap(values,150*sizeof(float),self.data,self.bytes) || !values_valid(values,count,full_sequence) ||
       !tuning || tuning_count!=128 || overlap(tuning,128*sizeof(float),self.data,self.bytes) ||
       overlap(&tables,sizeof tables,self.data,self.bytes)) return false;
    for(unsigned i=0;i<128;++i)
        if(!isfinite(tuning[i]) || tuning[i]<=0 || tuning[i]>rate*.5f) return false;
    for(unsigned i=0;i<52;++i) {
        const auto &r=tables.morph[i];
        if(r.band!=i/4 || static_cast<unsigned>(r.shape)!=i%4 ||
           !payload(r.samples,r.count,2.f,self)) return false;
    }
    for(unsigned i=0;i<13;++i) if(tables.frequency_band[i]!=wave_frequencies[i]) return false;
    return payload(tables.sine,tables.sine_count,1.f,self);
}
void State::init_graph(const synthone_shared::Tables &tables,int rate,const float *tuning) {
    // Each concrete sibling validates all inputs before replacing a live instance.
    context_={}; context_output_=0; context_.out=&context_output_;
    context_.sr=rate; context_.nchan=1; context_.len=5ul*static_cast<unsigned>(rate);
    memcpy(context_.filename,"test.wav",sizeof "test.wav"); sp=&context_;
    tuningTable=tuning;
    for(unsigned i=0;i<53;++i) {
        auto &d=descriptors_[i]; d={}; sp_ftbl_init(sp,&d,4096);
        d.tbl=const_cast<float *>(i<52?tables.morph[i].samples:tables.sine); d.del=0;
        if(i<52) ft_array[i]=&d; else sine=&d;
    }
    memcpy(ft_frequencyBand,tables.frequency_band,sizeof ft_frequencyBand);
    lfo1_0_1=lfo2_0_1=lfo3_0_1=0; lfo1_1_0=lfo2_1_0=lfo3_1_0=0;
    lfo1Smooth=lfo2Smooth=0; monoFrequencySmooth=261.6255653006f;
    monoFrequency=440.f*exp2((60.f-69.f)/12.f);
    phasor1_={}; phasor2_={}; port1_={}; port2_={}; mono_port_={};
    lfo1Phasor=&phasor1_; lfo2Phasor=&phasor2_; lfo1Port=&port1_; lfo2Port=&port2_;
    monoFrequencyPort=&mono_port_;
    sp_phasor_init(sp,lfo1Phasor,0); sp_phasor_init(sp,lfo2Phasor,0);
    sp_port_init(sp,lfo1Port,.0053125f); sp_port_init(sp,lfo2Port,.0053125f);
    sp_port_init(sp,monoFrequencyPort,.05f);
    poly={}; mono={}; noteStates=&poly; monoNote=&mono;
    playingNoteStatesIndex=0; heldNoteNumbers={}; heldNoteNumbersAE={};
    resetted=false; processSampleCounter=0; sequencer={};
    initialized_=true;
}
bool State::valid_restore(const float *values,size_t count,InstanceSpan self,bool full_sequence) const {
    return initialized_ &&
        (values==parameters || !overlap(values,150*sizeof(float),self.data,self.bytes)) &&
        values_valid(values,count,full_sequence);
}
bool RuntimeState::init_runtime(const synthone_shared::Tables &tables,int rate,const float *values,
                                size_t count,const float *tuning,size_t tuning_count,InstanceSpan self) {
    if(!valid_init(tables,rate,values,count,tuning,tuning_count,self)) return false;
    reader_={};reader_active_=false;compact_={};compact_active_=false;
    compactPorts.teardown();
    init_graph(tables,rate,tuning);
    // Validation already establishes valid disjoint outputs/controls and rate.
    if(!compactPorts.init(parameters,150,values,150,rate)) return false;
    restore_graph();
    return true;
}
bool RuntimeState::init_reader_runtime(const synthone_shared::Tables &tables,const void *packed,size_t bytes,
        int rate,const float *values,size_t count,const float *tuning,size_t tuning_count,InstanceSpan self) {
    if(overlap(&tables,sizeof tables,self.data,self.bytes) || !packed ||
       bytes!=S1_MORPH_DIRECT_BYTES || count!=150 || tuning_count!=128 ||
       overlap(packed,bytes,self.data,self.bytes) || overlap(packed,bytes,&tables,sizeof tables) || tables.sine_count!=4097 ||
       overlap(packed,bytes,values,150*sizeof(float)) || overlap(packed,bytes,tuning,128*sizeof(float)) ||
       overlap(packed,bytes,tables.sine,4097*sizeof(float)) ||
       reinterpret_cast<uintptr_t>(packed)%alignof(float) ||
       reinterpret_cast<uintptr_t>(values)%alignof(float) ||
       reinterpret_cast<uintptr_t>(tuning)%alignof(float) ||
       reinterpret_cast<uintptr_t>(tables.sine)%alignof(float)) return false;
    // Independent expected topology, not the reader's mapping implementation.
    static const unsigned char logical[52]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,
        40,41,42,43,44,45,46,47,44,45,48,49};
    const auto *data=static_cast<const unsigned char*>(packed);
    for(unsigned i=0;i<52;++i)
        if(tables.morph[i].samples!=reinterpret_cast<const float*>(data+static_cast<size_t>(logical[i])*4097*sizeof(float)))return false;
    if(!valid_init(tables,rate,values,count,tuning,tuning_count,self))return false;
    s1_morph_binding candidate{};
    if(!s1_morph_bind_direct(&candidate,packed,bytes))return false;
    // All replacement inputs are admitted before any live state is retired.
    if(!init_runtime(tables,rate,values,count,tuning,tuning_count,self))return false;
    reader_=candidate;reader_active_=true;return true;
}
bool RuntimeState::init_compact_runtime(const synthone_shared::Tables &tables,const void *packed,size_t bytes,
        int rate,const float *values,size_t count,const float *tuning,size_t tuning_count,InstanceSpan self,bool full_sequence) {
    // Authenticate the WHOLE final owner before dereferencing candidate metadata.
    if(overlap(&tables,sizeof tables,self.data,self.bytes) || !packed || count!=150 || tuning_count!=128 ||
       (rate!=44100&&rate!=48000&&rate!=96000) ||
       overlap(packed,bytes,self.data,self.bytes) || overlap(packed,bytes,&tables,sizeof tables) ||
       overlap(values,150*sizeof(float),self.data,self.bytes) || overlap(tuning,128*sizeof(float),self.data,self.bytes) ||
       overlap(packed,bytes,values,150*sizeof(float)) || overlap(packed,bytes,tuning,128*sizeof(float)) ||
       reinterpret_cast<uintptr_t>(values)%alignof(float) || reinterpret_cast<uintptr_t>(tuning)%alignof(float) ||
       !values_valid(values,count,full_sequence) || !tuning || !tables.sine || tables.sine_count!=4097 ||
       reinterpret_cast<uintptr_t>(tables.sine)%alignof(float) ||
       overlap(packed,bytes,tables.sine,4097*sizeof(float)))return false;
    for(unsigned i=0;i<128;++i)if(!isfinite(tuning[i])||tuning[i]<=0||tuning[i]>rate*.5f)return false;
    for(unsigned i=0;i<52;++i){const auto &r=tables.morph[i];
        if(r.samples||r.count!=4097||r.band!=i/4||static_cast<unsigned>(r.shape)!=i%4)return false;}
    for(unsigned i=0;i<13;++i){uint32_t actual,expected;memcpy(&actual,&tables.frequency_band[i],4);memcpy(&expected,&wave_frequencies[i],4);if(actual!=expected)return false;}
    if(!payload(tables.sine,tables.sine_count,1.f,self))return false;
    s1c_binding candidate{};if(!s1c_bind(&candidate,packed,bytes))return false;
    // Container band metadata must agree with the original immutable topology.
    const auto *data=static_cast<const unsigned char*>(packed);
    for(unsigned i=0;i<13;++i){uint32_t w=static_cast<uint32_t>(data[1316+4*i])|
        (static_cast<uint32_t>(data[1317+4*i])<<8)|(static_cast<uint32_t>(data[1318+4*i])<<16)|
        (static_cast<uint32_t>(data[1319+4*i])<<24),expected;memcpy(&expected,&wave_frequencies[i],4);if(w!=expected)return false;}
    // All fallible cold admission is complete before retiring a live instance.
    compactPorts.teardown();init_graph(tables,rate,tuning);
    if(!compactPorts.init(parameters,150,values,150,rate))return false;
    restore_graph();reader_={};reader_active_=false;compact_=candidate;compact_active_=true;return true;
}
bool RuntimeState::valid_runtime_output(float *l,float *r,unsigned n,unsigned offset,InstanceSpan self,bool full_sequence) const {
    if(n>65536||offset>65536-n)return false;
    const size_t bytes=(static_cast<size_t>(n)+offset)*sizeof(float);
    if(compact_active_&&(reinterpret_cast<uintptr_t>(l)%alignof(float)||reinterpret_cast<uintptr_t>(r)%alignof(float)||
       overlap(l,bytes,compact_.bytes,compact_.byte_count)||overlap(r,bytes,compact_.bytes,compact_.byte_count)))return false;
    if(reader_active_&&(overlap(l,bytes,reader_.bytes,reader_.byte_count)||
                       overlap(r,bytes,reader_.bytes,reader_.byte_count)))return false;
    return valid_output_span(l,r,n,offset,self,full_sequence);
}
bool RuntimeState::restore_runtime(const float *values,size_t count,InstanceSpan self,bool full_sequence) {
    if(compact_active_&&reinterpret_cast<uintptr_t>(values)%alignof(float))return false;
    if(!valid_restore(values,count,self,full_sequence)) return false;
    float saved[150]; memcpy(saved,values,sizeof saved);
    if(!compactPorts.restore(saved,150,sp->sr)) return false;
    restore_graph();
    return true;
}
void State::restore_graph() {
    previousProcessMonoPolyStatus=parameters[isMono];
    initializedNoteStates=false;
    // Original restore's sequencer.init creates 1024 pending entries. The caller
    // must explicitly establish reset(true) before this direct-event domain.
    sequencer.reset(true);
}
void State::initializeNoteStates() {initialize_note_objects<sp_oscmorph2d_init>();}
void RuntimeState::initializeNoteStates() {initialize_note_objects<init_morph_checked>();}
template<int (*Init)(sp_data*,sp_oscmorph2d*,sp_ftbl**,int,int,float*,SPFLOAT)>
void State::initialize_note_objects() {
    if(initializedNoteStates) return;
    initializedNoteStates=true;
    for(unsigned i=0;i<7;++i) {
        auto &v=i<6?poly[i]:mono; auto &o=objects_[i]; o={};
        v.kernel=this;
        v.adsr=&o.adsr; v.fadsr=&o.fadsr; v.oscmorph1=&o.morph1; v.oscmorph2=&o.morph2;
        v.morphCrossFade=&o.morph_crossfade; v.filterCrossFade=&o.filter_crossfade;
        v.subOsc=&o.sub; v.fmOsc=&o.fm; v.noise=&o.noise;
        v.loPass=&o.lowpass; v.bandPass=&o.bandpass; v.hiPass=&o.highpass;
        sp_adsr_init(sp,v.adsr); sp_adsr_init(sp,v.fadsr);
        Init(sp,v.oscmorph1,ft_array,4,13,ft_frequencyBand,0);
        v.oscmorph1->freq=0; v.oscmorph1->amp=0; v.oscmorph1->wtpos=0;
        v.oscmorph1->enableBandlimit=parameters[oscBandlimitEnable]; v.oscmorph1->bandlimitIndexOverride=-1;
        Init(sp,v.oscmorph2,ft_array,4,13,ft_frequencyBand,0);
        v.oscmorph2->freq=0; v.oscmorph2->amp=0; v.oscmorph2->wtpos=0;
        v.oscmorph2->enableBandlimit=parameters[oscBandlimitEnable]; v.oscmorph2->bandlimitIndexOverride=-1;
        sp_crossfade_init(sp,v.morphCrossFade); sp_crossfade_init(sp,v.filterCrossFade);
        sp_osc_init(sp,v.subOsc,sine,0); sp_fosc_init(sp,v.fmOsc,sine);
        sp_noise_init(sp,v.noise); sp_moogladder_init(sp,v.loPass);
        sp_butbp_init(sp,v.bandPass); sp_buthp_init(sp,v.hiPass);
        // Source setup initializes these four fields, not clear()'s amp.
        v.stage=S1NoteState::stageOff; v.internalGate=0; v.rootNoteNumber=-1; v.transpose=0;
    }
}
double State::tuningTableNoteToHz(int note) const { return tuningTable[clamp(note,0,127)]; }
bool State::valid_parameter(unsigned p,float v) const {
    return initialized_ && p<150 && isfinite(v) && (p!=arpIsOn || v==0.f);
}
bool State::valid_event(int pitch,int velocity,float frequency) const {
    return initialized_ && heldNoteNumbers.valid_count() && heldNoteNumbersAE.valid_count() && parameters[arpIsOn]==0 && sequencer.pending_empty && pitch>=0 && pitch<128 &&
        velocity>=0 && velocity<=127 && isfinite(frequency) && frequency>=0 && frequency<=sp->sr*.5f;
}
bool State::valid_output_span(float *l,float *r,unsigned n,unsigned offset,InstanceSpan self,bool full_sequence) const {
    if(!initialized_ || !l || !r || n>65536 || offset>65536-n || (!full_sequence && (parameters[arpIsOn]!=0 || !sequencer.pending_empty))) return false;
    const size_t bytes=(static_cast<size_t>(n)+offset)*sizeof(float);
    if(overlap(l,bytes,r,bytes) || overlap(l,bytes,self.data,self.bytes) || overlap(r,bytes,self.data,self.bytes) ||
       overlap(l,bytes,tuningTable,128*sizeof(float)) || overlap(r,bytes,tuningTable,128*sizeof(float))) return false;
    for(const auto &d:descriptors_) if(overlap(l,bytes,d.tbl,4097*sizeof(float)) || overlap(r,bytes,d.tbl,4097*sizeof(float))) return false;
    return true;
}
bool Scheduler::set_target(unsigned p,float v) {
    return valid_parameter(p,v) && compactPorts.set_resolved_target(p,clampedValue(static_cast<S1Parameter>(p),v));
}
bool Scheduler::start(int pitch,int velocity,float ignored_frequency) {
    if(!valid_event(pitch,velocity,ignored_frequency)) return false;
    for(int i=0;i<heldNoteNumbers.count;++i) if(heldNoteNumbers[i].noteNumber==pitch) { if(!heldNoteNumbers.remove_checked(i))return false; break; }
    if(!heldNoteNumbers.insert_checked({pitch,static_cast<int>(parameters[transpose]),velocity},0))return false;
    heldNoteNumbersAE=heldNoteNumbers;
    on_valid(pitch,velocity,tuningTableNoteToHz(pitch+static_cast<int>(parameters[transpose])));
    return true;
}
bool Scheduler::on(int pitch,int velocity) {
    if(!valid_event(pitch,velocity,0)) return false;
    on_valid(pitch,velocity,tuningTableNoteToHz(pitch+static_cast<int>(parameters[transpose]))); return true;
}
bool Scheduler::on(int pitch,int velocity,float frequency) {
    if(!valid_event(pitch,velocity,frequency)) return false;
    on_valid(pitch,velocity,frequency); return true;
}
void Scheduler::on_valid(int pitch,int velocity,float frequency) {
    initializeNoteStates();
    if(parameters[isMono]>0) {
        monoFrequency=frequency;
        if(parameters[monoIsLegato]==0) {
            mono.internalGate=0; mono.stage=S1NoteState::stageRelease;
            sp_adsr_compute(sp,mono.adsr,&mono.internalGate,&mono.amp);
            sp_adsr_compute(sp,mono.fadsr,&mono.internalGate,&mono.filter);
        }
        mono.startNoteHelper(pitch,velocity,frequency);
    } else {
        int selected=-1;
        for(int i=0;i<6;++i) if(poly[i].rootNoteNumber==pitch) { selected=i; break; }
        if(selected<0) for(int i=0;i<6;++i) {
            int index=(playingNoteStatesIndex+i)%6;
            if(poly[index].rootNoteNumber==-1) { selected=index; break; }
        }
        playingNoteStatesIndex=selected<0?(playingNoteStatesIndex+1)%6:selected;
        poly[playingNoteStatesIndex].startNoteHelper(pitch,velocity,frequency);
    }
    heldNotesDidChange();
}
bool Scheduler::stop(int pitch) {
    if(!valid_event(pitch,0,0)) return false;
    for(int i=0;i<heldNoteNumbers.count;++i) if(heldNoteNumbers[i].noteNumber==pitch) {if(!heldNoteNumbers.remove_checked(i))return false;break;}
    heldNoteNumbersAE=heldNoteNumbers; off_valid(pitch); return true;
}
bool Scheduler::off(int pitch) { if(!valid_event(pitch,0,0)) return false; off_valid(pitch); return true; }
void Scheduler::off_valid(int pitch) {
    initializeNoteStates();
    if(parameters[isMono]>0) {
        if(heldNoteNumbersAE.count==0) {
            if(mono.stage!=S1NoteState::stageOff) {mono.stage=S1NoteState::stageRelease;mono.internalGate=0;}
        } else {
            const auto &head=heldNoteNumbersAE[0];
            mono.rootNoteNumber=head.noteNumber; mono.transpose=static_cast<int>(parameters[transpose]);
            mono.velocity=head.velocity; mono.amp=pow2(head.velocity/127.f);
            mono.oscmorph1->amp=mono.oscmorph2->amp=mono.subOsc->amp=mono.fmOsc->amp=mono.amp;
            monoFrequency=tuningTableNoteToHz(head.noteNumber+static_cast<int>(parameters[transpose]));
            mono.oscmorph1->freq=mono.oscmorph2->freq=mono.subOsc->freq=mono.fmOsc->freq=monoFrequency;
            // The original fallback intentionally does not update noise->amp.
            if(parameters[monoIsLegato]==0) {
                mono.internalGate=0; mono.stage=S1NoteState::stageRelease;
                sp_adsr_compute(sp,mono.adsr,&mono.internalGate,&mono.amp);
                sp_adsr_compute(sp,mono.fadsr,&mono.internalGate,&mono.filter);
            }
            mono.stage=S1NoteState::stageOn; mono.internalGate=1;
        }
    } else for(auto &note:poly) if(note.rootNoteNumber==pitch) {
        if(note.stage!=S1NoteState::stageOff) {
            note.stage=S1NoteState::stageRelease; note.internalGate=0;
            sp_adsr_compute(sp,note.adsr,&note.internalGate,&note.amp);
            sp_adsr_compute(sp,note.fadsr,&note.internalGate,&note.filter);
        }
        break;
    }
    heldNotesDidChange();
}
bool Scheduler::stop_all() {
    if(!initialized_) return false;
    heldNoteNumbers.clear(); heldNoteNumbersAE=heldNoteNumbers;
    if(parameters[isMono]>0) return stop(60);
    for(int i=0;i<128;++i) stop(i);
    return true;
}
bool Scheduler::reset() {
    if(!initialized_) return false;
    for(auto &note:poly) note.clear(); mono.clear(); resetted=true; return true;
}
bool Scheduler::reset_dsp() {
    if(!initialized_) return false;
    heldNoteNumbers.clear(); heldNoteNumbersAE=heldNoteNumbers; sequencer.reset(true);
    set_target(arpIsOn,0); mono.clear(); for(auto &note:poly) note.clear(); return true;
}
bool Scheduler::reset_sequencer() { if(!initialized_) return false; sequencer.reset(false); beatCounterDidChange(); return true; }
bool Scheduler::process(unsigned n,unsigned offset,float *l,float *r) {
    if(!valid_output(l,r,n,offset)) return false;
    initializeNoteStates();
    if(parameters[isMono]>0) {
        if(mono.stage==S1NoteState::stageRelease && mono.amp<.01f) mono.clear();
    } else for(auto &note:poly) if(note.stage==S1NoteState::stageRelease && note.amp<.01f) note.clear();
    processSampleCounter+=n;
    if(processSampleCounter>2048.) {playingNotesDidChange();processSampleCounter=0;}
    trace(0,0);
    for(unsigned i=0;i<n;++i) {
        l[offset+i]=r[offset+i]=0;
        compactPorts.tick();
        monoFrequencyPort->htime=parameters[glide];
        sp_port_compute(sp,monoFrequencyPort,&monoFrequency,&monoFrequencySmooth);
        if(parameters[isMono]!=previousProcessMonoPolyStatus) {
            previousProcessMonoPolyStatus=parameters[isMono]; reset(); sequencer.reset(true);
        }
        processLfo(); if(!sequencer.process_checked(parameters,heldNoteNumbersAE))return false;
        if(parameters[isMono]>0) {
            if(mono.rootNoteNumber!=-1 && mono.stage!=S1NoteState::stageOff) {
                if(compact_active_) { if(!s1scheduler_compact::run_note(mono,offset+i,l,r,compact_))return false; }
                else if(reader_active_) { if(!s1scheduler_reader::run_note(mono,offset+i,l,r,reader_))return false; }
                else mono.run(offset+i,l,r);
            }
        } else for(auto &note:poly)
            if(note.rootNoteNumber!=-1 && note.stage!=S1NoteState::stageOff) {
                if(compact_active_) { if(!s1scheduler_compact::run_note(note,offset+i,l,r,compact_))return false; }
                else if(reader_active_) { if(!s1scheduler_reader::run_note(note,offset+i,l,r,reader_))return false; }
                else note.run(offset+i,l,r);
            }
        trace(1,i);
    }
    return true;
}
} // namespace s1scheduler
