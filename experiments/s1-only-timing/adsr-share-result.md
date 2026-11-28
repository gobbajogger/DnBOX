# Exact equal-voice ADSR sharing: does not meet loaded timing

Diagnostic only. Start from the 69-row baseline described in
`display-lossless-69.md` at a17ad21b617d7166ce3ef36be18fb06d03d1c975.
Apply **one** of `adsr-share-branch.patch` or `adsr-share-compact.patch`.
Do not enable these patches in the production image.

The scope is the no-internal-sequencer native control-rate oscillator path.
All six polyphonic voices must be active and have identical gate bits and
identical amplitude and filter ADSR state at entry (including signed zeros).
Voice 0 runs both existing envelopes once per sample; each follower still
runs its own oscillator, filter, noise and mix in the original order. Its
envelope outputs are updated before its filter/mix and final envelope state
is materialized at span exit. A mode/voice-set change invalidates sharing;
unequal state takes the ordinary path. This is not a reduced voice count,
quality change or preset edit. It does not cover live changes to the same DSP
instance from a second owner; the fixture renders within audio ownership.

Run `check-adsr-span.sh` from a disposable directory containing its adjacent
C fixture and `baseline/` and `candidate/` source trees. The test compares
actual compiled renderers, using the target DSP configuration, not an
ADSR-only model. Both variants match 968,400 stereo frames and the complete
engine plus arena byte state across 10,800 spans (staggered release, retrigger,
steal, changing controls, noise, filter modes and mono excursions). It proves
72 full chord starts take two instead of twelve calls per sample, with 1,756
sharing spans and 9,044 unchanged/fallback spans. With an unchanged reference
in both arms it fails the required sharing check.

| Diagnostic | Held peak µs | Whole peak µs | XRUNs | App flash free | Pool margin after reserve |
| --- | ---: | ---: | ---: | ---: | ---: |
| 69-row Elite, unchanged | 14,065.42 | 15,293 | 151 | 2,020 B | 33,260 B |
| 69-row Pro, unchanged | 14,065.83 | 15,254 | 151 | 2,028 B | 33,260 B |
| Explicit sharing path, Elite | 14,059.96 | 15,523 | 147 | 1,124 B | 33,260 B |
| Explicit sharing path, Pro | 14,070.92 | 15,524 | 147 | 1,132 B | 33,260 B |
| Compact in-loop sharing, Elite | 14,106.04 | 15,837 | 147 | 1,332 B | 33,260 B |

Each timed candidate generated all six voices, 132 actual hits per kit, and
no emulator skips. All 128 held halves still missed both the 10,809 µs
engineering target and the 11,609.98 µs physical period. Four fewer overall
XRUNs is not a pass. Worst-case whole-run timing worsened. Retain the
unshared 69-row default. The explicit sharing arm matches all 160 ordered
pre-DMA fingerprints in a separate instrumented Elite comparison. The
session plays six dry Synth One voices and the 808 and 909 kits; it is not a
wet-effects, all-preset, or deployment result.

Separate compact witness: with `render-fingerprints.patch` and
`adsr-share-witness.patch`, all 160 ordered pre-DMA fingerprints match
the original. The guard shared all 65,536 held frames and no held frames
fell back; 9,728 other frames shared and 149,760 did not. Therefore the
unchanged held deadline is not explained by the guard failing to engage.
Do not use the instrumented timings as adoption evidence. Measure a
drum-core hot path or exact load rebalance next rather than spending more
flash solely to reduce Synth One envelope calls.
