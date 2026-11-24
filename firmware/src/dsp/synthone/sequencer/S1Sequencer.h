//
//  S1Sequencer.h
//  AudioKitSynthOne -> DnBOX shared Synth One / SuperJ6 core
//
//  Created by AudioKit Contributors on 3/06/19.
//  Copyright © 2019 AudioKit. All rights reserved.
//  MIT License: ../controls/frontend/notices/SynthOne-MIT.txt
//
//  C port of DSP/Sequencer/S1Sequencer.hpp + S1SeqNoteNumber.hpp + S1ArpModes.hpp
//  (pinned source 56079bbd). Same structure and names; compact where the chip
//  needs it (see S1Sequencer.c for every change). Used by BOTH the Synth One and
//  the SuperJ6 owner (the SuperJ6 app is "Synth One J6", same sequencer); the
//  SuperJ6-only differences are the two delta fields in S1SequencerParams.
//
#pragma once
#include <stdint.h>

#define S1_NUM_MIDI_NOTES 128
#define S1_SEQ_MAX_HELD 8          /* DnBOX: held keys tracked (source: 128) */
#define S1_SEQ_STEPS 16
#define S1_SAMPLE_TICKS 256u      /* beat-clock ticks per sample (DnBOX: Q8 samples, as seq_t) */

typedef struct { int noteNumber; int onOff; int velocity; } SeqNoteNumber;
typedef struct { uint8_t noteNumber, velocity; } NoteNumber;     /* heldNoteNumbersAE entry */

/* S1ArpModes.hpp */
enum ArpeggiatorMode { ArpeggiatorMode_Up = 0, ArpeggiatorMode_UpDown = 1, ArpeggiatorMode_Down = 2 };

/* The S1Parameter values S1Sequencer::process reads (S1Parameter.h ids). */
typedef struct {
    uint8_t arpIsOn;                    /* 62 */
    uint8_t arpIsSequencer;             /* 65 */
    uint8_t arpDirection;               /* 60  ArpeggiatorMode */
    uint8_t arpOctave;                  /* 63  0..3 */
    uint8_t arpInterval;                /* 61  0..12 */
    uint8_t arpTotalSteps;              /* 66  1..16 */
    int8_t sequencerPattern[S1_SEQ_STEPS];  /* 67..82 */
    uint16_t sequencerOctBoost;         /* 83..98, bit per step */
    uint16_t sequencerNoteOn;           /* 99..114, bit per step */
    /* Beat clock in fixed ticks: arpRate / 60 / sampleRate per sample and the
     * 147 multiplier per step, both scaled by one factor (only the ratio
     * matters to the source's fmod test). DnBOX: S1_SAMPLE_TICKS per sample. */
    uint32_t arpBeatsPerSample;
    uint32_t arpSeqTempoMultiplier;
    /* SuperJ6 deltas (Synth One: 100, 12) */
    uint8_t seqGate;                    /* % of a step a note sounds (SJ6 seqGate) */
    uint8_t patternRange;               /* |sequencerPattern| limit: S1 12, SJ6 24 */
    uint32_t gateTicks;                 /* seqGate % of arpSeqTempoMultiplier (UINT32_MAX = 100 %) */
    uint8_t clocked;                    /* DnBOX: transport clock owns mStepCounter / mBeatTime */
} S1SequencerParams;

typedef void (*KeyOnCallback)(void *ctx, int note, int velocity);
typedef void (*KeyOffCallback)(void *ctx, int note);

typedef struct {
    int previousHeldNoteNumbersAECount;
    uint32_t mBeatTime;                 /* fmod(beatTime, arpSeqTempoMultiplier), ticks */
    int mStepCounter;
    int lastPosition;                   /* DnBOX H3: sequence position that last sounded */
    int mStepBase;                      /* DnBOX: ARP order restarts here (press / transport start) */
    uint8_t gateOpen;
    uint8_t sequencerLastNotesCount;
    uint8_t sequencerLastNotes[S1_SEQ_MAX_HELD];
} S1Sequencer;

void S1Sequencer_init(S1Sequencer *);
void S1Sequencer_reset(S1Sequencer *, int resetNotes, void *ctx, KeyOffCallback off);
/* 1 if process() would turn notes on or off at this sample (lets the caller
 * render up to the event first). Pure. */
int S1Sequencer_willFire(const S1Sequencer *, const S1SequencerParams *, int heldCount);
/* One sample of S1Sequencer::process. held = heldNoteNumbersAE (newest first). */
void S1Sequencer_process(S1Sequencer *, const S1SequencerParams *, const NoteNumber *held, int heldCount,
                         void *ctx, KeyOnCallback on, KeyOffCallback off);
int S1Sequencer_getArpBeatCount(const S1Sequencer *);
/* Set the step length (and its gate) in ticks. */
void S1Sequencer_setStep(S1SequencerParams *, uint32_t ticks);
/* DnBOX: follow the transport. step/phase = clock position before this sample's
 * increment (step -1 just before the first step); force = transport (re)start. */
void S1Sequencer_clock(S1Sequencer *, const S1SequencerParams *, int step, uint32_t phase, int force);
