# Direct SuperJ6 AUv3: calibrated Jupiter Sine loudness references

Seven renders from the genuine factory Jupiter Sine record in the Mac AUv3
0.999.8 build 9, with an explicitly neutralized wet path and an authored
A440 held note (MIDI 69, velocity 87, frame 16,384–60,483).
The two solo oscillator paths differ only in the public oscillator faders;
the half-level arms change only the master control. Both authored arms
have repeats; both-muted is exactly silent. Nothing is normalized,
sample-shifted or gain-fitted. Exact readbacks and hashes are included.

The late-held native RMS is approximately .0824 and .0855 per channel for
OSC1 and OSC2 at the authored master; half-master levels are about 5.61
and 5.92 dB lower. Signed repeat residuals on this long cue include
phase drift, so they should not be treated as level repeatability.
This is an independent source reference to discriminate the old
Brainwave-only projection hypothesis. It is not a SuperJ6 parity pass.

Run `python3 score_gain.py .` with NumPy. The same seven raw references are
backed up in ChatGPT Library. The adjacent reusable bench build includes
`capture_gain.swift` without changing any existing queue profiles.
