# Age-dependent RevSC state, rejected host-only
Source clock, row sizes, cubic fractional reads, modulation and feedback
operation order remain authentic. Eight latest samples per line are exact;
older state retains Float32 anchors each 64 source samples, linear reconstruction
with no future unwritten anchor, eight independent age clocks. No source signal,
write or read is decimated at the DSP graph boundary. This differs from older
whole-network downsample arms. Exact newest state is not an exact audible early
return: the first echo already reads an old cell (the original line lengths are
1979–4198 samples in this configuration). At least the first such interval on
each line must survive before the control can make a faithful first echo.

Host configured history 1912 B/owner plus original host descriptor 440 B/owner,
4704 B combined, only 60 B below the modeled 4764 B two-owner capacity after
preserving delay and width. The numbers are illustrative; no native ABI/link
or CPU claimed. Actual code is an isolated bounded bench (counter wrap not
qualified).

Replays use the same Pad candidate room send, authentic source Organ room send,
and independently generated in-phase-opposed 1037/2913 Hz burst with four live
feedback settings. Both renderers read the exact same input waveform/controls.
Scored the raw room return before dry or compression makeup, no gain/time fit.
Pad loses 16.1–16.5 dB at 960–3200 Hz in the first-echo window, then 8.75–10.63
dB total in the final second. Organ loses 15.86–16.93 dB total in the final
second and 52–58 dB in that frequency band. Anti-phase loses 8.52–9.93 dB on
held return and introduces spurious upper-band energy (held +6.37–9.04 dB).
Window channel metrics where the authentic reference is silent are null.
Reject; no adoption and no production changes. This does not prove an
information-theoretic minimum. It specifically rules out using full-precision
low-cadence anchors while keeping a tiny recent region to recover original
first echoes or dense late modulation. The exact earlier E6M9 result remains
the measured larger representation, not a fitted waveform claim.

Run bash run.sh on archived workspace with input hashes in results.json.
Source, host report and statistics only; no PCM, plugin binary or tables.
