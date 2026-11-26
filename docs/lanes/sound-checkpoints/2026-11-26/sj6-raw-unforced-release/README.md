# Raw J106: authentic release is predominantly existing filter state

This source-causal test reads the signed original Raw J106 dry render
at its pinned absolute MIDI frames. It does not align, phase-shift or
fit any waveform. Hermetic control compares a synthetic 65.4 Hz held
tone turned off at the note-off, and an otherwise identical tone
continued at a decaying amplitude. Both go through known original
scoped physical HP10, connected zero-Crusher DC and final DC poles.

An unforced output obeys the corresponding homogeneous recurrence.
Applying it at stride 64 reduces ill-conditioning, without changing
sample stamps. In the original 16–50, 50–100 and 100–200 ms release
intervals, the modal residual is about −121, −116 and −116 dBr. The
true zero-input positive control is −134 to −136 and the ongoing-tone
negative control is near −19 dBr. This demonstrates that most of the
authentic tail is compatible with **unforced existing filter state**.
It does not prove the in-app private stage state or action taken on
each individual note-off.

The first clean original 512-sample window (stride 32) starts 13.06 ms
after note-off; the previous window starts at 11.61 ms and still has
residual. The actual compact center-only input's last nonzero frame is
+597 samples, 13.537 ms after note-off. The windows are not exact gate
timestamps, but these values are incompatible with an explanation
based only on a very long original envelope missing from compact.
Separately, the compact inherited state has wrong polarity and rings
with more residual; never clear it simply because the note ends.
Match the signal driving it and measure the first release intervals.

```sh
python3 free-tail.py ../sj6-native-factory-cross
```

The generated compact cue includes exact center-only and physical-suffix
outputs, fixed PCM hashes and explicit absolute begin/end. A companion
fixed MIDI-frequency lattice measures preceding held frames 34,324–38,433
without changing time or gain. The inverse scoped original suffix gives
first harmonic (mainly Sub) −18.99 dBFS and second harmonic (mainly Main)
−18.19 dBFS. Compact after the tried center sum is −24.70 and −27.72,
respectively. The relative Main weight is about 3.8 dB weaker and its
relative phase differs (−90.16° vs +20.89°). This is a composite signal
and conditional stage de-embedding, not a true private tap or proof of
which owner changes note-start phase. Separate authenticated Saw width
test found no pulse-width response/control bug at this actual Saw
detent; do not use it to change pulse width on the Raw preset. Reconcile
actual Main/Sub signal ownership and inheritance of filter state.

```sh
python3 held-sub-and-main.py ../sj6-native-factory-cross/program-2-dry compact-generated
shasum -a 256 -c decomposition/SHA256SUMS  # run from decomposition/ for relative rows
```
