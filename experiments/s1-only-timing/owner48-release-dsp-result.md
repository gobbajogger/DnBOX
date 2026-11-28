# 48-row diagnostic: release overlap plus exact drum hot paths

On [the original a54125b6 composition](owner48-combined-drum-result.md),
apply these patches in order:

1. [Early release overlap](owner48-release-prefix.patch), as explained in the [release-only receipt](owner48-release-overlap-result.md).
2. [Remove two diagnostic counters](owner48-s1-remove-diagnostic-counters.patch).
3. [Retain the exact 808 bank phase steps across chunks](owner48-808-bank-persistent-step.patch).
4. [Combine the power-of-two tanh coordinate scale for the 909 kick](owner48-909-drive16-combine.patch).

This is the best strict loaded result in this comparison, not a general scheduler
or production build. It uses six dry Synth One voices and both real kits.
The 808 cache costs 24 B of RAM; it keeps the 10,080 B display RAM saving and
the existing 8,192 B audio reserve. Neither sound settings nor the nominal
audio callback deadline change. The original controller is attached per model
before a fresh virtual-NOR run. The normal boot package without it is not a
valid same-fixture timing control.

| Canonical strict emulator | Elite | Pro |
|---|---:|---:|
| Original combined baseline dropouts | 79 | 78 |
| Early release only | 65 | 67 |
| Release and counter removal | 65 | 65 |
| **Release, counter removal and both drum paths** | **48** | **49** |
| Worst complete block, µs | 12,751 | 12,687 |
| Worst held block, µs | 12,751.21 | 12,687.83 |
| Held blocks over 11,609.98 µs | 34/128 | 39/128 |
| Held blocks over 10,809 µs | 125/128 | 126/128 |
| Free app flash | 1,376 B | 1,384 B |
| Audio-pool margin beyond reserve | 43,316 B | 43,316 B |

Both boards returned all six key events in and out and 132 real sequencer
hits per kit during the held interval. Both have 48 rows and 23,040 B canvas,
zero oversize and no emulator-skipped cycles. These are uninstrumented
timing results from [Elite](owner48-release-dsp-elite.json) and
[Pro](owner48-release-dsp-pro.json). The physical deadline remains missed.

Separate native audio runs matched every ordered pre-DMA block fingerprint
([160/160 Elite](owner48-prefix-dsp-fingerprints.txt),
[160/160 Pro](owner48-prefix-dsp-pro-fingerprints.txt)). This covers 128 held
and 32 release blocks on each board, not wet effects or other presets.
No hardware, wet FX, installer, device, or zero-dropout gate is claimed.

## Independent evidence and known gaps

- 808 bank cache alone: 74 Elite / 76 Pro dropouts on the original base.
  All four host buses and engine state matched across 8,205,582 frames,
  and [160/160 pre-DMA block hashes](owner48-808-bank-step-elite-fingerprints.txt)
  matched in a separate canonical Elite fingerprint run.
- 909 kick alone: 76 Elite / 77 Pro dropouts, with bit-exact output and
  state at two host optimization levels, plus 75,497,473 exact phase
  comparisons. See the [independent 909 receipt](owner48-909-drive16-result.md).
- [Display CPU trims](owner48-display-cpu-proof.md) are separate.
  They preserve captured pixels and the dirty-tile transport decisions
  in the tested scenes; there is no loaded XRUN result. Do not count them
  as RAM savings or include them in the best composition by default.
- [Reverse activity snapshot experiment](owner48-reverse-guard.patch):
  after the release-only patch, bypasses the second-core activity query
  when the scheduler already admitted a live 808 reverse prefix. It
  measured 66 Elite dropouts and 12,867 µs, but the last-active-prefix
  scenario and interrupted-core recovery still need dedicated tests.
  It is not part of the four-patch timing winner.
- Two master/FX micro-optimizations were rejected on the separate raw
  Elite fixture: the guarded unity-wet shortcut kept 76 XRUNs but added peak
  time; the idle send bus scan and wrap changes gave 77 vs 76. Both matched
  host DSP output/state; no evidence to keep either in the canonical build.

The next timing target is the **held** block, not another release-only
routing tweak. In the profiled precursor both cores spent roughly 10 ms
in useful work in the worst held block before about 1 ms of serial tail.
The newest measured peaks still need roughly 1.1 ms of worst-case margin
improvement just to reach the physical deadline. Profile the new exact
composition before assigning further patches to either core.
