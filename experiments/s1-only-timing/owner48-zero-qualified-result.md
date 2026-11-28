# Zero-dropout dry Synth One and two-kit native diagnostic

The **same strict 8-second emulator stress test** now runs without audio
dropouts on Elite and Pro while retaining the app and audio RAM reserves.
This is native firmware code on the pinned emulator, not a physical-board
or release qualification. The fixed test uses six held dry Synth One
voices, actual sequencer playback, and 132 witnessed 808 hits plus 132
909 hits. It does not test the wet Synth One engine or SuperJ.

| Canonical uninstrumented replay | Elite | Pro |
| --- | ---: | ---: |
| Full-run dropouts, original 402bd0e4 → this build | **23 → 0** | **23 → 0** |
| Audio half-buffer callbacks | 624 | 624 |
| Held audio halves exceeding physical period | 0 / 128 | 0 / 128 |
| Worst held block | 11,559.79 µs | 11,553.25 µs |
| Margin to 11,609.98 µs physical period | 50.19 µs | 56.73 µs |
| Held halves exceeding 10,809 µs engineering target | 17 / 128 | 18 / 128 |
| App flash free (1,024 B minimum) | **1,508 B** | **1,516 B** |
| Audio-pool free beyond intact 8,192 B reserve | **37,620 B** | **37,620 B** |
| Ordinary data + BSS | 95,408 / 98,304 B | 95,408 / 98,304 B |
| Display RAM recovered vs. the prior 69-row allocation | 10,080 B | 10,080 B |

[Elite timing](owner48-zero-cold-notes-elite.json),
[Pro timing](owner48-zero-cold-notes-pro.json),
[Elite fit and workload](owner48-zero-cold-notes-fit-elite.txt),
[Pro fit and workload](owner48-zero-cold-notes-fit-pro.txt).
Both exact binaries were then replayed again in fresh virtual NOR: [Elite](owner48-zero-cold-notes-elite-repeat.json) and [Pro](owner48-zero-cold-notes-pro-repeat.json) reproduced their first timing JSON byte-for-byte.
Both recorded zero emulator skips and zero oversized draw requests. The
DMA size, sample rate, CPU clock, controller template, controls, notes,
and hit counts were not altered.

**Audio:** separate native correctness builds matched all 160 ordered
pre-DMA audio blocks against 402bd0e4 on
[Elite](owner48-zero-cold-notes-fp-elite.txt) and
[Pro](owner48-zero-cold-notes-fp-pro.txt). The observer changes layout,
so those builds are *not* timing qualification. The same audio block
window includes all 128 held halves and 32 release halves, not all patches
or every control interaction. The 808 optimization additionally matched
all four output buses and the complete kit state through 8,205,582
randomized frames in 64,000 host spans, and passed 90,000 exact
phase-boundary cases with 14,369 guarded admissions.

## What moved the result

1. [Two-oscillator Synth One staging plus the contiguous three-tom split](owner48-zero-result.md)
   brought the baseline from 23 to 10 on both boards without altering sums.
2. [The exact 909 transition table and early private BREAK render](owner48-zero-imearly-result.md)
   made that 8. The decoder uses 5,696 B of persistent RAM; no precision or
   sample content is traded. BREAK still mixes and emits deferred MIDI in
   its original order.
3. [Avoiding a redundant planner search inside the existing fixed diagnostic route](owner48-zero-noplan-result.md)
   resolved six more misses without removing any DSP work.
4. [Four-frame 808 Schmitt bank batching](owner48-zero-schmitt-lean.patch)
   groups only windows proven free of duty/wrap/BLEP events and drift
   refresh; all other frames still run the original logic. Integer
   addition of the six exact square signs and the same final scaling
   preserves the output and RNG/phase state. A direct, compile-checked
   FX strip snapshot mapping also removes duplicate setup.
5. The first zero-dropout pair had just 900/908 B of free app flash,
   so it did not qualify. Size-oriented compilation of four Chicago
   note/control units (not the per-sample DSP) recovered **608 B** on
   each model, and the paired measured build still had zero dropouts.

## Reproduce and scope

Run `sh owner48-zero-fourframe-compose.sh REPOSITORY NEW_SCRATCH_DIR`;
then apply `owner48-zero-build-cold-notes.patch` in its `source` directory.
Rebuild with the existing `owner48-build.sh`, and use the established
`owner48-replay.sh` and pinned model-specific controller templates. The
recomposed source matched the tested source, excluding compiler output
and patch backups. For the exact DMA comparison independently apply
`render-fingerprints.patch` and use `compare-six-loaded-dma.py`.

The physical dropout result is now zero in this workload, but the
stricter engineering margin is not yet met. In particular, the 50–57 µs
remaining at the worst observed half is not a 100 µs margin guarantee.
The diagnostic scheduler and test construction are not proposed for
release wholesale. Nothing was installed, flashed, or deployed.
