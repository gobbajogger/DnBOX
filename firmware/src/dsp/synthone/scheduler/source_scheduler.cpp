#include "source_scheduler.hpp"
#include "reader/runtime_note.hpp"
#include "compact_note.hpp"
#include "float_math.h"
#include <math.h>
#include <string.h>
namespace s1scheduler {
bool SourceScheduler::valid_source_event(int pitch,int velocity,float frequency)const{
 return initialized_ && heldNoteNumbers.valid_count() && heldNoteNumbersAE.valid_count() && pitch>=0 && pitch<128 && velocity>=0 && velocity<=127 && s1_finitef(frequency) && frequency>=0 && frequency<=sp->sr*.5f;
}
bool SourceScheduler::set_notes_per_octave(int n){
    if(!initialized_ || !s1source_sequence::SerializedSequence::tuning_valid(parameters,150,n))return false;
    sequence.set_notes_per_octave(n);return true;
}
bool SourceScheduler::set_target(unsigned p,float v) {
    if(!initialized_ || p>=150 || !s1_finitef(v))return false;
    const float resolved=clampedValue(static_cast<S1Parameter>(p),v);
    float candidate[150];memcpy(candidate,parameters,sizeof candidate);candidate[p]=resolved;
    if(!s1source_sequence::SerializedSequence::tuning_valid(candidate,150,sequence.notes_per_octave()))return false;
    return compactPorts.set_resolved_target(p,resolved);
}
bool SourceScheduler::start(int pitch,int velocity,float ignored_frequency) {
    if(!valid_source_event(pitch,velocity,ignored_frequency)) return false;
    for(int i=0;i<heldNoteNumbers.count;++i) if(heldNoteNumbers[i].noteNumber==pitch) { if(!heldNoteNumbers.remove_checked(i))return false; break; }
    if(!heldNoteNumbers.insert_checked({pitch,static_cast<int>(parameters[transpose]),velocity},0))return false;
    heldNoteNumbersAE=heldNoteNumbers;
    if(parameters[arpIsOn]==1.f)return true;
    on_valid(pitch,velocity,tuningTableNoteToHz(pitch+static_cast<int>(parameters[transpose])));
    return true;
}
bool SourceScheduler::on(int pitch,int velocity) {
    if(!valid_source_event(pitch,velocity,0)) return false;
    on_valid(pitch,velocity,tuningTableNoteToHz(pitch+static_cast<int>(parameters[transpose]))); return true;
}
bool SourceScheduler::on(int pitch,int velocity,float frequency) {
    if(!valid_source_event(pitch,velocity,frequency)) return false;
    on_valid(pitch,velocity,frequency); return true;
}
void SourceScheduler::on_valid(int pitch,int velocity,float frequency) {
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
bool SourceScheduler::stop(int pitch) {
    if(!valid_source_event(pitch,0,0)) return false;
    for(int i=0;i<heldNoteNumbers.count;++i) if(heldNoteNumbers[i].noteNumber==pitch) {if(!heldNoteNumbers.remove_checked(i))return false;break;}
    heldNoteNumbersAE=heldNoteNumbers; if(parameters[arpIsOn]!=1.f)off_valid(pitch); return true;
}
bool SourceScheduler::off(int pitch) { if(!valid_source_event(pitch,0,0)) return false; off_valid(pitch); return true; }
void SourceScheduler::off_valid(int pitch) {
    initializeNoteStates();
    if(parameters[isMono]>0) {
        if(parameters[arpIsOn]==1.f || heldNoteNumbersAE.count==0) {
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
bool SourceScheduler::stop_all() {
    if(!initialized_) return false;
    heldNoteNumbers.clear(); heldNoteNumbersAE=heldNoteNumbers;
    if(parameters[isMono]>0) return stop(60);
    for(int i=0;i<128;++i) stop(i);
    return true;
}
bool SourceScheduler::reset() {
    if(!initialized_) return false;
    for(auto &note:poly) note.clear(); mono.clear(); resetted=true; return true;
}
bool SourceScheduler::reset_dsp() {
    if(!initialized_) return false;
    heldNoteNumbers.clear(); heldNoteNumbersAE=heldNoteNumbers; sequence.reset(true);
    set_target(arpIsOn,0); mono.clear(); for(auto &note:poly) note.clear(); return true;
}
bool SourceScheduler::reset_sequencer() { if(!initialized_) return false; sequence.reset(false); beatCounterDidChange(); return true; }
bool SourceScheduler::process(unsigned n,unsigned offset,float *l,float *r) {
#if defined(S1_HARMONIC_ONLY)
    if(!compact_active_ || compact_.bytes || compact_.byte_count || compact_.format_tag!=0x48334d31u)return false;
#endif

    if(!valid_output(l,r,n,offset) || !heldNoteNumbersAE.valid_count()) return false;
    struct View{const NoteNumber*values;size_t count;};
    const View held{heldNoteNumbersAE.values.begin(),static_cast<size_t>(heldNoteNumbersAE.count)};
    if(!sequence.can_process(parameters,150,held))return false;
    // All owner/control/trace calls are serialized and non-reentrant; callbacks
    // do not mutate controls. Ports.tick is the only in-block parameter writer.
    // Reuse static validation only when its inputs cannot be smoothed by that
    // actual port map. A future map change safely selects full sample checks.
    bool reuse_static_admission=true;
    for(unsigned slot=0;slot<synthone_ports::selected_count;++slot)
        if(s1source_sequence::SerializedSequence::uses_parameter(synthone_ports::selected_indices[slot]))
            reuse_static_admission=false;
    initializeNoteStates();
    if(parameters[isMono]>0) {
        if(mono.stage==S1NoteState::stageRelease && mono.amp<.01f) mono.clear();
    } else for(auto &note:poly) if(note.stage==S1NoteState::stageRelease && note.amp<.01f) note.clear();
    processSampleCounter+=n;
    if(processSampleCounter>2048.f) {playingNotesDidChange();processSampleCounter=0;}
    trace(0,0);
    for(unsigned i=0;i<n;++i) {
        l[offset+i]=r[offset+i]=0;
        compactPorts.tick();
        monoFrequencyPort->htime=parameters[glide];
        sp_port_compute(sp,monoFrequencyPort,&monoFrequency,&monoFrequencySmooth);
        if(parameters[isMono]!=previousProcessMonoPolyStatus) {
            previousProcessMonoPolyStatus=parameters[isMono]; reset(); sequence.reset(true);
        }
        processLfo();
        Sink sink{*this};
        if(!(reuse_static_admission ? sequence.process_static_admitted(parameters,held,sink)
                                   : sequence.process(parameters,150,held,sink)))return false;
        if(parameters[isMono]>0) {
            if(mono.rootNoteNumber!=-1 && mono.stage!=S1NoteState::stageOff) {
#if defined(S1_HARMONIC_ONLY)
                if(!s1scheduler_compact::run_note(mono,offset+i,l,r,compact_))return false;
#else
                if(compact_active_) { if(!s1scheduler_compact::run_note(mono,offset+i,l,r,compact_))return false; }
                else if(reader_active_) { if(!s1scheduler_reader::run_note(mono,offset+i,l,r,reader_))return false; }
                else mono.run(offset+i,l,r);
#endif
            }
        } else for(auto &note:poly)
            if(note.rootNoteNumber!=-1 && note.stage!=S1NoteState::stageOff) {
#if defined(S1_HARMONIC_ONLY)
                if(!s1scheduler_compact::run_note(note,offset+i,l,r,compact_))return false;
#else
                if(compact_active_) { if(!s1scheduler_compact::run_note(note,offset+i,l,r,compact_))return false; }
                else if(reader_active_) { if(!s1scheduler_reader::run_note(note,offset+i,l,r,reader_))return false; }
                else note.run(offset+i,l,r);
#endif
            }
        trace(1,i);
    }
    return true;
}
#if defined(S1_NO_DOUBLE)
extern "C" void s1_scheduler_keep(void) {
    /* Address only. The firmware calls this when X0X_SYNTHONE=1 so
     * --gc-sections keeps this scheduler and the DSP it calls. */
    static bool (SourceScheduler::*method)(unsigned, unsigned, float *, float *);
    method = &SourceScheduler::process;
    __asm__ __volatile__("" : : "m"(method));
}
#endif
}
