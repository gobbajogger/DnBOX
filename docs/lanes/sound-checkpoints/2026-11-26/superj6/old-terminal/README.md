# Original old SuperJ6 terminal DC DSP

Run from workspace root with authorized old ARM64 bench on PYTHONPATH and
original old signed file set as SJ6_BIN: python sj6-old-terminal/probe.py.
The authorized binary's SHA is pinned and private media is not bundled.

This directly invokes original createDCBlockDSP, setDCBlockDSPGain and
AKDCBlockDSP::process with the same deterministic asymmetric signed input.
No Apple graph, Limiter, master fader, room or reference app PCM is inferred.
Only a fixed non-formatting Soundpipe name sprintf import and platform libc
are bridged. Four arms at 44.1/48k, default or explicit .999, verify that the
actual working pole is the identical Float32 .9990000128746033 in both channels.
Across all 4,104 output samples the original is bit-exact to
FMA(pole,previousY,Float32(currentX-previousX)). Real stops copy samples
bit-exact, including amplitudes >1, and preserve both per-channel histories
until restart. Fixture alternating output changes only +0.124 dB (includes
settling from the preceding asymmetric waveform), not a hidden safety gain.
A low-frequency DC-like step attenuates as designed; not a broadband level
control. No change to new Mac harness, native controls, candidate or production.
