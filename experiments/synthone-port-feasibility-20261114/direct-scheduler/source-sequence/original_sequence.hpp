// Mechanical binding of pinned original source; no timing/sequence arithmetic changes.
#pragma once
#include "sequence_types.hpp"
#include <array>
#include <atomic>
#include <functional>
#include <list>
#include <vector>
#include <algorithm>
#include <cmath>
namespace s1source_oracle {
using namespace s1source_sequence;
// Explicit local enum bindings avoid accidental collision with another kernel namespace.
using s1source_sequence::index1;
using s1source_sequence::index2;
using s1source_sequence::morphBalance;
using s1source_sequence::morph1SemitoneOffset;
using s1source_sequence::morph2SemitoneOffset;
using s1source_sequence::morph1Volume;
using s1source_sequence::morph2Volume;
using s1source_sequence::subVolume;
using s1source_sequence::subOctaveDown;
using s1source_sequence::subIsSquare;
using s1source_sequence::fmVolume;
using s1source_sequence::fmAmount;
using s1source_sequence::noiseVolume;
using s1source_sequence::lfo1Index;
using s1source_sequence::lfo1Amplitude;
using s1source_sequence::lfo1Rate;
using s1source_sequence::cutoff;
using s1source_sequence::resonance;
using s1source_sequence::filterMix;
using s1source_sequence::filterADSRMix;
using s1source_sequence::isMono;
using s1source_sequence::glide;
using s1source_sequence::filterAttackDuration;
using s1source_sequence::filterDecayDuration;
using s1source_sequence::filterSustainLevel;
using s1source_sequence::filterReleaseDuration;
using s1source_sequence::attackDuration;
using s1source_sequence::decayDuration;
using s1source_sequence::sustainLevel;
using s1source_sequence::releaseDuration;
using s1source_sequence::morph2Detuning;
using s1source_sequence::detuningMultiplier;
using s1source_sequence::masterVolume;
using s1source_sequence::bitCrushDepth;
using s1source_sequence::bitCrushSampleRate;
using s1source_sequence::autoPanAmount;
using s1source_sequence::autoPanFrequency;
using s1source_sequence::reverbOn;
using s1source_sequence::reverbFeedback;
using s1source_sequence::reverbHighPass;
using s1source_sequence::reverbMix;
using s1source_sequence::delayOn;
using s1source_sequence::delayFeedback;
using s1source_sequence::delayTime;
using s1source_sequence::delayMix;
using s1source_sequence::lfo2Index;
using s1source_sequence::lfo2Amplitude;
using s1source_sequence::lfo2Rate;
using s1source_sequence::cutoffLFO;
using s1source_sequence::resonanceLFO;
using s1source_sequence::oscMixLFO;
using s1source_sequence::reverbMixLFO;
using s1source_sequence::decayLFO;
using s1source_sequence::noiseLFO;
using s1source_sequence::fmLFO;
using s1source_sequence::detuneLFO;
using s1source_sequence::filterEnvLFO;
using s1source_sequence::pitchLFO;
using s1source_sequence::bitcrushLFO;
using s1source_sequence::tremoloLFO;
using s1source_sequence::arpDirection;
using s1source_sequence::arpInterval;
using s1source_sequence::arpIsOn;
using s1source_sequence::arpOctave;
using s1source_sequence::arpRate;
using s1source_sequence::arpIsSequencer;
using s1source_sequence::arpTotalSteps;
using s1source_sequence::sequencerPattern00;
using s1source_sequence::sequencerPattern01;
using s1source_sequence::sequencerPattern02;
using s1source_sequence::sequencerPattern03;
using s1source_sequence::sequencerPattern04;
using s1source_sequence::sequencerPattern05;
using s1source_sequence::sequencerPattern06;
using s1source_sequence::sequencerPattern07;
using s1source_sequence::sequencerPattern08;
using s1source_sequence::sequencerPattern09;
using s1source_sequence::sequencerPattern10;
using s1source_sequence::sequencerPattern11;
using s1source_sequence::sequencerPattern12;
using s1source_sequence::sequencerPattern13;
using s1source_sequence::sequencerPattern14;
using s1source_sequence::sequencerPattern15;
using s1source_sequence::sequencerOctBoost00;
using s1source_sequence::sequencerOctBoost01;
using s1source_sequence::sequencerOctBoost02;
using s1source_sequence::sequencerOctBoost03;
using s1source_sequence::sequencerOctBoost04;
using s1source_sequence::sequencerOctBoost05;
using s1source_sequence::sequencerOctBoost06;
using s1source_sequence::sequencerOctBoost07;
using s1source_sequence::sequencerOctBoost08;
using s1source_sequence::sequencerOctBoost09;
using s1source_sequence::sequencerOctBoost10;
using s1source_sequence::sequencerOctBoost11;
using s1source_sequence::sequencerOctBoost12;
using s1source_sequence::sequencerOctBoost13;
using s1source_sequence::sequencerOctBoost14;
using s1source_sequence::sequencerOctBoost15;
using s1source_sequence::sequencerNoteOn00;
using s1source_sequence::sequencerNoteOn01;
using s1source_sequence::sequencerNoteOn02;
using s1source_sequence::sequencerNoteOn03;
using s1source_sequence::sequencerNoteOn04;
using s1source_sequence::sequencerNoteOn05;
using s1source_sequence::sequencerNoteOn06;
using s1source_sequence::sequencerNoteOn07;
using s1source_sequence::sequencerNoteOn08;
using s1source_sequence::sequencerNoteOn09;
using s1source_sequence::sequencerNoteOn10;
using s1source_sequence::sequencerNoteOn11;
using s1source_sequence::sequencerNoteOn12;
using s1source_sequence::sequencerNoteOn13;
using s1source_sequence::sequencerNoteOn14;
using s1source_sequence::sequencerNoteOn15;
using s1source_sequence::filterType;
using s1source_sequence::phaserMix;
using s1source_sequence::phaserRate;
using s1source_sequence::phaserFeedback;
using s1source_sequence::phaserNotchWidth;
using s1source_sequence::monoIsLegato;
using s1source_sequence::widen;
using s1source_sequence::compressorMasterRatio;
using s1source_sequence::compressorReverbInputRatio;
using s1source_sequence::compressorReverbWetRatio;
using s1source_sequence::compressorMasterThreshold;
using s1source_sequence::compressorReverbInputThreshold;
using s1source_sequence::compressorReverbWetThreshold;
using s1source_sequence::compressorMasterAttack;
using s1source_sequence::compressorReverbInputAttack;
using s1source_sequence::compressorReverbWetAttack;
using s1source_sequence::compressorMasterRelease;
using s1source_sequence::compressorReverbInputRelease;
using s1source_sequence::compressorReverbWetRelease;
using s1source_sequence::compressorMasterMakeupGain;
using s1source_sequence::compressorReverbInputMakeupGain;
using s1source_sequence::compressorReverbWetMakeupGain;
using s1source_sequence::delayInputCutoffTrackingRatio;
using s1source_sequence::delayInputResonance;
using s1source_sequence::tempoSyncToArpRate;
using s1source_sequence::pitchbend;
using s1source_sequence::pitchbendMinSemitones;
using s1source_sequence::pitchbendMaxSemitones;
using s1source_sequence::frequencyA4;
using s1source_sequence::portamentoHalfTime;
using s1source_sequence::oscBandlimitIndexOverride;
using s1source_sequence::oscBandlimitEnable;
using s1source_sequence::arpSeqTempoMultiplier;
using s1source_sequence::transpose;
using s1source_sequence::adsrPitchTracking;
using s1source_sequence::S1ParameterCount;
using S1Parameter=Param;
using NoteNumber=HeldNote;
using SeqNoteNumber=SeqNote;
using BOOL=bool;
constexpr int S1_NUM_MIDI_NOTES=128;
//
//  S1ArpModes.hpp
//  AudioKitSynthOne
//
//  Created by AudioKit Contributors on 3/06/19.
//  Copyright © 2019 AudioKit. All rights reserved.
//

enum struct ArpeggiatorMode : int {
    Up = 0,
    UpDown = 1,
    Down = 2
};

//
//  S1Arpeggiator.hpp
//  AudioKitSynthOne
//
//  Created by AudioKit Contributors on 3/06/19.
//  Copyright © 2019 AudioKit. All rights reserved.
//



template<typename SeqNoteNumbers, typename NoteNumbers>
struct Arpeggiator {
    Arpeggiator() = delete;

    static int up(SeqNoteNumbers &sequencerNotes, NoteNumbers &sequencerNotes2,
                   const int heldNotesCount, const int arpOctaves, const int interval) {
        int index = 0;
        for (int octave = 0; octave < arpOctaves; octave++) {
            for (int i = 0; i < heldNotesCount; i++) {
                NoteNumber& note = sequencerNotes2[i];
                const int nn = note.noteNumber + (octave * interval);
                const int velocity = note.velocity;
                SeqNoteNumber snn{nn, 1, velocity};
                std::vector<SeqNoteNumber>::iterator it = sequencerNotes.begin() + index;
                sequencerNotes.insert(it, snn);
                ++index;
            }
        }
        return index;
    }

    static void down(SeqNoteNumbers &sequencerNotes, NoteNumbers &sequencerNotes2,
                          const int heldNotesCount, const int arpOctaves, const int interval, const bool noTail, int index = 0)
    {
        for (int octave = arpOctaves - 1; octave >= 0; octave--) {
            for (int i = heldNotesCount - 1; i >= 0; i--) {
                const bool firstNote = (i == heldNotesCount - 1) && (octave == arpOctaves - 1);
                const bool lastNote = (i == 0) && (octave == 0);
                if ((firstNote || lastNote) && noTail) {
                    continue;
                }

                NoteNumber& note = sequencerNotes2[i];
                const int nn = note.noteNumber + (octave * interval);
                const int velocity = note.velocity;
                SeqNoteNumber snn{nn, 1, velocity};
                std::vector<SeqNoteNumber>::iterator it = sequencerNotes.begin() + index;
                sequencerNotes.insert(it, snn);
                ++index;
            }
        }
    };
};

//
//  OriginalSequence.hpp
//  AudioKitSynthOne
//
//  Created by AudioKit Contributors on 3/06/19.
//  Copyright © 2019 AudioKit. All rights reserved.
//




using DSPParameters = std::array<float, S1Parameter::S1ParameterCount>;
using BeatCounterChangedCallback = std::function<void()>;
using KeyOnCallback = std::function<void(int, int)>;
using KeyOffCallback = std::function<void(int)>;

class OriginalSequence {

public:
    OriginalSequence() = delete;
    OriginalSequence(KeyOnCallback keyOnCb, KeyOffCallback keyOffCb, BeatCounterChangedCallback beatChangedCb);

    void setSampleRate(double sampleRate);
    void init();
    void reset(bool resetNotes);
    void setNotesPerOctave(int notes);

    int getArpBeatCount();
    void process(DSPParameters &params, const s1source_sequence::HeldView &held);

public:
    double mSampleRate = 0;
    const int maxSequencerNotes = 1024; // 128 midi note numbers * 4 arp octaves * up+down
    void reserveNotes(); // Allocate notes before rendering

    // Array of midi note numbers of NoteState's which have had a noteOn event but not yet a noteOff event.
    int previousHeldNoteNumbersAECount; // previous render loop held key count

    // Beattime Counter
    double mBeatTime = 0;
    std::atomic<int> mStepCounter = 0;

    ///once init'd: sequencerNotes can be accessed and mutated only within process and resetDSP
    std::vector<SeqNoteNumber> sequencerNotes;
    std::vector<NoteNumber> sequencerNotes2;

    ///once init'd: sequencerLastNotes can be accessed and mutated only within process and resetDSP
    std::list<int> sequencerLastNotes;

    std::atomic<int> mNotesPerOctave{12};

    // Change notifications
    BeatCounterChangedCallback mBeatCounterDidChange;
    KeyOnCallback mTurnOnKey;
    KeyOffCallback mTurnOffKey;
};


//
//  OriginalSequence.mm
//  AudioKitSynthOne
//
//  Created by AudioKit Contributors on 3/06/19.
//  Copyright © 2019 AudioKit. All rights reserved.
//




inline OriginalSequence::OriginalSequence(KeyOnCallback keyOnCb,
    KeyOffCallback keyOffCb, BeatCounterChangedCallback beatChangedCb) :
    mTurnOnKey(keyOnCb),
    mTurnOffKey(keyOffCb),
    mBeatCounterDidChange(beatChangedCb)
    {
        static_assert(decltype(mStepCounter)::is_always_lock_free, "StepCounter not lockfree!");
    }

inline void OriginalSequence::init() {
    previousHeldNoteNumbersAECount = 0;
    reserveNotes();
}

inline void OriginalSequence::reset(bool resetNotes) {
    previousHeldNoteNumbersAECount = resetNotes ? 0 : previousHeldNoteNumbersAECount;
    sequencerLastNotes.clear();
    sequencerNotes.clear();
    sequencerNotes2.clear();
}

inline void OriginalSequence::process(DSPParameters &params, const s1source_sequence::HeldView &held) {
    
    /// MARK: ARPEGGIATOR + SEQUENCER BEGIN
    const int heldNoteNumbersAECount = held.count;
    const BOOL arpSeqIsOn = (params[arpIsOn] == 1.f);
    const BOOL firstTimeAnyKeysHeld = (previousHeldNoteNumbersAECount == 0 && heldNoteNumbersAECount > 0);
    const BOOL firstTimeNoKeysHeld = (heldNoteNumbersAECount == 0 && previousHeldNoteNumbersAECount > 0);
    
    // reset arp/seq when user goes from 0 to N, or N to 0 held keys
    if ( arpSeqIsOn && (firstTimeNoKeysHeld || firstTimeAnyKeysHeld) ) {
        
        mBeatTime = 0; // reset internal beattime
        mStepCounter = 0;

        // Turn OFF previous beat's notes
        std::for_each(sequencerLastNotes.begin(), sequencerLastNotes.end(), [&] (const auto &note) {
            mTurnOffKey(note);
        });
        sequencerLastNotes.clear();
        
        mBeatCounterDidChange();
    }
    
    // If arp is ON, or if previous beat's notes need to be turned OFF
    if ( arpSeqIsOn || sequencerLastNotes.size() > 0 ) {
        
        // Compare previous beatTime to current to see if we crossed a beat boundary
        const auto newBeatTime = mBeatTime + (params[arpRate] / 60.f) / mSampleRate;
        const double r0 = fmod(mBeatTime, params[arpSeqTempoMultiplier]);
        mBeatTime = newBeatTime;
        const double r1 = fmod(mBeatTime, params[arpSeqTempoMultiplier]);
        
        // If keys are now held, or if beat boundary was crossed
        if ( firstTimeAnyKeysHeld || r1 < r0 ) {
            
            // Turn off previous beat's notes even if arp is off
            std::for_each(sequencerLastNotes.begin(), sequencerLastNotes.end(), [&] (const auto &note) {
                mTurnOffKey(note);
            });
            sequencerLastNotes.clear();
            
            // ARP/SEQ is ON
            if (arpSeqIsOn) {
                
                // Held Notes
                if (heldNoteNumbersAECount > 0) {
                    // Create Arp/Seq array based on held notes and/or sequence parameters
                    sequencerNotes.clear();
                    sequencerNotes2.clear();
                    
                    // Only update "notes per octave" when beat counter changes so sequencerNotes and sequencerLastNotes match

                    if (mNotesPerOctave.load() <= 0) mNotesPerOctave.store(12);
                    const float npof = (float)mNotesPerOctave.load()/12.f; // 12ET ==> npof = 1
                    
                    if ( params[arpIsSequencer] == 1.f ) {
                        
                        // SEQUENCER
                        const int numSteps = params[arpTotalSteps] > 16 ? 16 : (int)params[arpTotalSteps];
                        for(int i = 0; i < numSteps; i++) {
                            const int onOff = params[(S1Parameter)(i + sequencerNoteOn00)];
                            const int octBoost = params[(S1Parameter)(i + sequencerOctBoost00)];
                            const int nn = params[(S1Parameter)(i + sequencerPattern00)] * npof;
                            const int nnob = (nn < 0) ? (nn - octBoost * mNotesPerOctave.load()) : (nn + octBoost * mNotesPerOctave.load());

                            // sequencer note velocity is reassigned below when constructed sequence is played
                            sequencerNotes.push_back({nnob, onOff, 127});
                        }
                    } else {
                        
                        // ARPEGGIATOR
                        for(const NoteNumber *note=held.begin();note!=held.end();++note) {
                            std::vector<NoteNumber>::iterator it = sequencerNotes2.begin();
                            sequencerNotes2.insert(it, *note);
                        }
                        const int heldNotesCount = (int)sequencerNotes2.size();
                        const int arpIntervalUp = params[arpInterval] * npof;
                        const int arpOctaves = (int)params[arpOctave] + 1;
                        const auto arpMode = static_cast<ArpeggiatorMode>(params[arpDirection]);

                        switch(arpMode) {
                            case ArpeggiatorMode::Up: {
                                Arpeggiator<decltype(sequencerNotes), decltype(sequencerNotes2)>::up(
                                     sequencerNotes, sequencerNotes2, heldNotesCount, arpOctaves, arpIntervalUp);
                                break;
                            }
                            case ArpeggiatorMode::UpDown: {
                                int index = Arpeggiator<decltype(sequencerNotes), decltype(sequencerNotes2)>::up(
                                    sequencerNotes, sequencerNotes2, heldNotesCount, arpOctaves, arpIntervalUp);
                                const bool noTail = true;
                                Arpeggiator<decltype(sequencerNotes), decltype(sequencerNotes2)>::down(
                                    sequencerNotes, sequencerNotes2, heldNotesCount, arpOctaves, arpIntervalUp, noTail, index);
                                break;
                            }
                            case ArpeggiatorMode::Down: {
                                Arpeggiator<decltype(sequencerNotes), decltype(sequencerNotes2)>::down(
                                    sequencerNotes, sequencerNotes2, heldNotesCount, arpOctaves, arpIntervalUp, false);
                                break;
                            }
                        }
                    }
                    
                    // At least one key is held down, and a non-empty sequence has been created
                    if ( sequencerNotes.size() > 0 ) {
                        
                        // Advance arp/seq beatCounter, notify delegates
                        mStepCounter.store(static_cast<int>(mBeatTime / params[arpSeqTempoMultiplier]));
                        const int seqNotePosition = mStepCounter.load() % sequencerNotes.size();
                        mBeatCounterDidChange();
                        
                        //MARK: ARP+SEQ: turn ON the note of the sequence
                        SeqNoteNumber& snn = sequencerNotes[seqNotePosition];
                        
                        if (params[arpIsSequencer] == 1.f) {
                            
                            // SEQUENCER
                            if (snn.onOff == 1) {
                                for(const NoteNumber *noteStruct=held.begin();noteStruct!=held.end();++noteStruct) {
                                    const int baseNote = noteStruct->noteNumber;
                                    const int note = baseNote + snn.noteNumber;
                                    const int velocity = noteStruct->velocity;
                                    if (note >= 0 && note < S1_NUM_MIDI_NOTES) {
                                        mTurnOnKey(note, velocity);
                                        sequencerLastNotes.push_back(note);
                                    }
                                }
                            }
                        } else {
                            
                            // ARPEGGIATOR
                            const int note = snn.noteNumber;
                            const int velocity = snn.velocity;
                            if (note >= 0 && note < S1_NUM_MIDI_NOTES) {
                                mTurnOnKey(note, velocity);
                                sequencerLastNotes.push_back(note);
                            }
                        }
                    }
                }
            }
        }
    }
    previousHeldNoteNumbersAECount = heldNoteNumbersAECount;
    
    /// MARK: ARPEGGIATOR + SEQUENCER END
}

inline void OriginalSequence::reserveNotes() {
    sequencerNotes.reserve(maxSequencerNotes);
    sequencerNotes2.reserve(maxSequencerNotes);
    sequencerLastNotes.resize(maxSequencerNotes);
}

// Getter and Setter

inline int OriginalSequence::getArpBeatCount() {
    return mStepCounter.load();
}


inline void OriginalSequence::setSampleRate(double sampleRate) {
    mSampleRate = sampleRate;
}

inline void OriginalSequence::setNotesPerOctave(int notes)
{
  mNotesPerOctave.store(notes);
}

}
