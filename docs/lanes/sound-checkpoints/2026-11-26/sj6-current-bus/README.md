# Current official SuperJ6: actual per-voice Normal accumulator and fan-in test

From workspace root, with the authorized current Mach-O pointer file and
Unicorn 2.1.4 + original bench on PYTHONPATH, run python sj6-current-bus/probe.py.
All recorded measurements rerun byte-identically. Pins are verified by probe.

The current compiled receiver renders Main1, Main2 and a synthetic marker
Sub (not official Sub samples) with the exact historical projected authored
controls. Hooks independently observe the left/right Normal biquad input,
its output and literal Float32 destination accumulation. On 25,908 frames
per operator (155,448 stereo words total), the zero-base single-voice
accumulator + filter output is bit-identical to the returned PCM. Held
Main1 .09759744 -> .10833786 and Main2 .09664011 -> .10202846 RMS. This
rules out a hidden global attenuation in this exact actual receiver boundary.
It does not assert the complete modern app-specific fader/profile state or
physical four-bus AVAudioMixer, nor the authentic Sub asset.

## Minimal direct native fan-in experiment, staged not executed

Run python sj6-current-bus/stage.py NEW_DIRECTORY. It verifies the archive,
original Swift source and S4 config before generating a source-only variant
of the existing standalone direct AUv3 Mac host (no Ableton or new binary).
Generate an app from the exact existing bundle flow, keep the existing
official installed AU pin, and capture all/OSC1/OSC2/Sub with repeats.
Mute room + Unison and force Crusher amount (108) to 0, leaving original
program/master/clock untouched except the intended oscillator fader (1, 3)
and Sub fader (202). The actual current Crush enable 107 and Lo-Fi 109 are
already zero but strictly verified. Reject any unexpected or changed live
setting. Main two layers are not mislabeled as one oscillator.

Run python sj6-current-bus/score_fanin.py DIRECTORY. It authenticates all
readbacks/PCM and compares the unweighted operator sum with all-on at the
real recorded timestamps. If sum null is within genuine repeat bounds, this
rejects meaningful cross-input gain/interaction on this state but cannot
identify an overall gain. If it fails, first distinguish deterministic
state/RNG changes from input-dependent output handling; do not infer an
arbitrary boost or retune any captured sound. For absolute stage attribution
one genuine instrument-private pre-fan-in PCM remains needed.

## Current actual upstream fader and pending gain

Run python sj6-current-bus/upstream_gain.py with the same oracle env. Actual
newest address0 value arrives at VoicePrep's first float as the same Float32
for authored, half and quarter controls; fresh and pending gain stores at
0x1003b4dc0 and 0x1003b4d7c respectively multiply by current or prior
velocity. Independent incoming velocities 78/82/96 make the distinction
observable and rule out accidental stale equality. Main1, Main2, marker
Sub each return exactly scaled buffers: 3×2×10,240 words, 61,440 total,
held note to chord. No additional factor appears. This verifies compiled
receiver controls, not the Swift mapping that produces their input.
