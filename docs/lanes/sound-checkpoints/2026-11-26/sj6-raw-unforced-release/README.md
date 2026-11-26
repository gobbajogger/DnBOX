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
