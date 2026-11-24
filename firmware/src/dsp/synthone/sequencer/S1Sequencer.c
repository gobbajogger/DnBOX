//
//  S1Sequencer.c
//  AudioKitSynthOne -> DnBOX shared Synth One / SuperJ6 core
//
//  Created by AudioKit Contributors on 3/06/19.
//  Copyright © 2019 AudioKit. All rights reserved.
//  MIT License: ../controls/frontend/notices/SynthOne-MIT.txt
//
//  C port of DSP/Sequencer/S1Sequencer.mm + S1Arpeggiator.hpp (pinned 56079bbd).
//  Textually included by app/original_runtime.c (one translation unit).
//  Changes from the source, all compact-for-chip, none in the note logic:
//   - reset lands on beat 0 after this sample's increment, so every step is exactly
//     arpSeqTempoMultiplier ticks (the double clock made the first step one sample short).
//   - mBeatTime (double) -> integer tick phase within one step + mStepCounter: the
//     source's r1 < r0 (fmod wrap) test is exactly "phase reached one step";
//     no binary64 / fmod on the device. A multiplier change keeps the phase.
//   - sequencerNotes / sequencerNotes2 / std::list: no vectors. The note at
//     seqNotePosition is computed directly from the same loops (Arpeggiator::up
//     / down), and sequencerLastNotes is a fixed array (max held keys).
//   - notes-per-octave fixed at 12 (DnBOX has no microtuning), so npof = 1.
//   - ObjC/AEArray/std::function -> plain arrays and C callbacks.
//   - SuperJ6 deltas: seqGate (turn the step's notes off early; 100 = source
//     behaviour, off at the next beat) and patternRange (+-24 instead of +-12).
//
#include "S1Sequencer.h"

void S1Sequencer_init(S1Sequencer *s)
{
    s->previousHeldNoteNumbersAECount = 0;
    s->mBeatTime = 0;
    s->mStepCounter = 0;
    s->lastPosition = -1;
    s->gateOpen = 0;
    s->sequencerLastNotesCount = 0;
}

static void turnOffLastNotes(S1Sequencer *s, void *ctx, KeyOffCallback off)
{
    for (unsigned i = 0; i < s->sequencerLastNotesCount; ++i) off(ctx, s->sequencerLastNotes[i]);
    s->sequencerLastNotesCount = 0;
    s->gateOpen = 0;
}

void S1Sequencer_reset(S1Sequencer *s, int resetNotes, void *ctx, KeyOffCallback off)
{
    s->previousHeldNoteNumbersAECount = resetNotes ? 0 : s->previousHeldNoteNumbersAECount;
    if (off) turnOffLastNotes(s, ctx, off);
    s->sequencerLastNotesCount = 0;
}

/* Arpeggiator<>::up / ::down without the vector: the noteNumber/velocity of
 * sequence entry k. sequencerNotes2 = held notes in reverse AEArray order. */
static NoteNumber arpNote(const NoteNumber *held, int heldNotesCount, int arpOctaves,
                          int interval, int mode, int k)
{
    const int m = heldNotesCount * arpOctaves;
    int up = mode == ArpeggiatorMode_Up || (mode == ArpeggiatorMode_UpDown && k < m);
    if (mode == ArpeggiatorMode_UpDown && k >= m) k = k - m + 1;    /* down, noTail */
    const int octave = up ? k / heldNotesCount : arpOctaves - 1 - k / heldNotesCount;
    const int i = up ? k % heldNotesCount : heldNotesCount - 1 - k % heldNotesCount;
    const NoteNumber note = held[heldNotesCount - 1 - i];           /* sequencerNotes2[i] */
    const int nn = note.noteNumber + octave * interval;
    NoteNumber r = { (uint8_t)(nn < 0 || nn >= S1_NUM_MIDI_NOTES ? 255 : nn), note.velocity };
    return r;
}
static int sequenceSize(const S1SequencerParams *p, int heldNotesCount)
{
    if (p->arpIsSequencer) return p->arpTotalSteps > 16 ? 16 : p->arpTotalSteps;
    const int m = heldNotesCount * (p->arpOctave + 1);
    return p->arpDirection == ArpeggiatorMode_UpDown && m > 1 ? 2 * m - 2 : m;
}
static uint32_t gateLength(const S1SequencerParams *p) { return p->gateTicks; }
void S1Sequencer_setStep(S1SequencerParams *p, uint32_t ticks)
{
    p->arpSeqTempoMultiplier = ticks;
    p->gateTicks = p->seqGate >= 100 ? UINT32_MAX : (uint32_t)((uint64_t)ticks * p->seqGate / 100u);
}

int S1Sequencer_willFire(const S1Sequencer *s, const S1SequencerParams *p, int heldCount)
{
    const int arpSeqIsOn = p->arpIsOn == 1;
    const int firstTimeAnyKeysHeld = s->previousHeldNoteNumbersAECount == 0 && heldCount > 0;
    const int firstTimeNoKeysHeld = heldCount == 0 && s->previousHeldNoteNumbersAECount > 0;
    if (arpSeqIsOn && (firstTimeNoKeysHeld || firstTimeAnyKeysHeld)) return 1;
    if (!(arpSeqIsOn || s->sequencerLastNotesCount)) return 0;
    const uint32_t next = s->mBeatTime + p->arpBeatsPerSample;
    if (next >= p->arpSeqTempoMultiplier) return 1;                 /* r1 < r0 */
    return s->gateOpen && s->sequencerLastNotesCount && next >= gateLength(p);
}

void S1Sequencer_process(S1Sequencer *s, const S1SequencerParams *p, const NoteNumber *held, int heldNoteNumbersAECount,
                         void *ctx, KeyOnCallback on, KeyOffCallback off)
{
    /// MARK: ARPEGGIATOR + SEQUENCER BEGIN
    const int arpSeqIsOn = p->arpIsOn == 1;
    const int firstTimeAnyKeysHeld = s->previousHeldNoteNumbersAECount == 0 && heldNoteNumbersAECount > 0;
    const int firstTimeNoKeysHeld = heldNoteNumbersAECount == 0 && s->previousHeldNoteNumbersAECount > 0;

    // reset arp/seq when user goes from 0 to N, or N to 0 held keys
    if (arpSeqIsOn && (firstTimeNoKeysHeld || firstTimeAnyKeysHeld)) {
        if (!p->clocked) {               // DnBOX: a running transport keeps its grid
            s->mBeatTime = 0u - p->arpBeatsPerSample; // reset internal beattime (0 after this sample's add)
            s->mStepCounter = 0;
        }
        // Turn OFF previous beat's notes
        turnOffLastNotes(s, ctx, off);
    }

    // If arp is ON, or if previous beat's notes need to be turned OFF
    if (arpSeqIsOn || s->sequencerLastNotesCount > 0) {
        // Compare previous beatTime to current to see if we crossed a beat boundary
        uint32_t newBeatTime = s->mBeatTime + p->arpBeatsPerSample;
        int crossed = 0;
        if (newBeatTime >= p->arpSeqTempoMultiplier) {
            newBeatTime -= p->arpSeqTempoMultiplier;
            if (newBeatTime >= p->arpSeqTempoMultiplier) newBeatTime = 0;   /* multiplier shrank */
            ++s->mStepCounter;
            crossed = 1;
        }
        s->mBeatTime = newBeatTime;
        // SuperJ6 seqGate: the step's notes end early
        if (!crossed && s->gateOpen && s->mBeatTime >= gateLength(p)) turnOffLastNotes(s, ctx, off);

        // If keys are now held, or if beat boundary was crossed
        if (firstTimeAnyKeysHeld || crossed) {
            // Turn off previous beat's notes even if arp is off
            turnOffLastNotes(s, ctx, off);
            // ARP/SEQ is ON, held notes
            if (arpSeqIsOn && heldNoteNumbersAECount > 0) {
                const int size = sequenceSize(p, heldNoteNumbersAECount);
                // At least one key is held down, and a non-empty sequence has been created
                if (size > 0) {
                    // DnBOX: SEQ follows the transport grid; ARP order starts on the press
                    if (firstTimeAnyKeysHeld || s->mStepCounter < s->mStepBase) s->mStepBase = s->mStepCounter;
                    const int seqNotePosition = (p->arpIsSequencer == 1 ? s->mStepCounter
                                                 : s->mStepCounter - s->mStepBase) % size;
                    if (p->arpIsSequencer == 1) {
                        // SEQUENCER
                        const int i = seqNotePosition;
                        const int onOff = p->sequencerNoteOn >> i & 1;
                        const int octBoost = p->sequencerOctBoost >> i & 1;
                        int nn = p->sequencerPattern[i];
                        nn = nn < -p->patternRange ? -p->patternRange : nn > p->patternRange ? p->patternRange : nn;
                        const int nnob = (nn < 0) ? (nn - octBoost * 12) : (nn + octBoost * 12);
                        if (onOff == 1) {
                            for (int h = 0; h < heldNoteNumbersAECount; ++h) {
                                const int note = held[h].noteNumber + nnob;
                                if (note >= 0 && note < S1_NUM_MIDI_NOTES &&
                                    s->sequencerLastNotesCount < S1_SEQ_MAX_HELD) {
                                    on(ctx, note, held[h].velocity);
                                    s->sequencerLastNotes[s->sequencerLastNotesCount++] = (uint8_t)note;
                                }
                            }
                        }
                    } else {
                        // ARPEGGIATOR
                        const NoteNumber snn = arpNote(held, heldNoteNumbersAECount, p->arpOctave + 1,
                                                       p->arpInterval, p->arpDirection, seqNotePosition);
                        if (snn.noteNumber < S1_NUM_MIDI_NOTES) {
                            on(ctx, snn.noteNumber, snn.velocity);
                            s->sequencerLastNotes[s->sequencerLastNotesCount++] = snn.noteNumber;
                        }
                    }
                    s->gateOpen = s->sequencerLastNotesCount > 0;
                    if (s->gateOpen) s->lastPosition = seqNotePosition;   // last sounding step (DnBOX H3 view)
                }
            }
        }
    }
    s->previousHeldNoteNumbersAECount = heldNoteNumbersAECount;
    /// MARK: ARPEGGIATOR + SEQUENCER END
}

int S1Sequencer_getArpBeatCount(const S1Sequencer *s) { return s->mStepCounter; }

// DnBOX: lock to the transport. The free-running clock above advances at the
// same rate, so normally only the phase is refreshed; a one-sample lag fires
// on the next sample, a lead waits, anything else (start, locate) jumps.
void S1Sequencer_clock(S1Sequencer *s, const S1SequencerParams *p, int step, uint32_t phase, int force)
{
    const uint32_t len = p->arpSeqTempoMultiplier;
    if (force) {
        s->mStepCounter = step; s->mBeatTime = phase; s->mStepBase = step + 1;
    } else if (step == s->mStepCounter) {
        s->mBeatTime = phase;
    } else if (step == s->mStepCounter + 1) {
        s->mBeatTime = len - 1u;                            // lag: cross on the next sample
    } else if (!(step == s->mStepCounter - 1 && s->mBeatTime < len / 2u)) {
        s->mStepCounter = step; s->mBeatTime = phase;       // jump
    }
}
