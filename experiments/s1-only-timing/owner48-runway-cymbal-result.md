# Exact 909 sampler and staged 808 cymbal diagnostic

The reference is the four-patch composition in [the 48/49 result](owner48-release-dsp-result.md).
A canonical Elite rebuild exactly reproduced the published timing JSON:
48 full-run dropouts, 34/128 held blocks over the physical deadline,
12,751.21 µs worst, 1,376 B app XIP and 43,316 B in the audio pool beyond
the unchanged 8,192 B reserve. Pro's existing reference is 49 dropouts,
39/128 held over, 12,687.83 µs worst, 1,384 B app flash.

## Exact source changes

- [Shared 909 fractional loop](owner48-909-runway-fused.patch): combines
  the integral and near-unity fractional interior sampler loops. The
  same bounds and IMA recurrence preserve both PCM and decoder history,
  without changing playback pitch, retrigger, choke or envelope. Generic
  rates and endpoints remain on the original path. The actual fixture
  increments the fractional phase; the old unity-only path did not apply.
  [Host proof](owner48-909-runway-fused-host.txt): all four buses and complete
  engine state match in 23,280 blocks at both O0 and O2. Linked app goes
  from 976 B free for duplicate-loop runway to **1,456 B free** on Elite,
  480 B gained, or 80 B more than the unchanged reference. Exact same pool.
- [Two-pass 808 cymbal](owner48-808-cymbal-front-two-pass.patch): stage only
  two independent front filters and op amp outputs in 32-frame chunks.
  The suffix consumes the exact same outputs. If it retires before the end,
  replay only front filter state through the consumed prefix so no unplayed
  sample alters continuing state. Both native callers bound chunks at 32.
  It adds no persistent RAM, uses two 32-float automatic vectors and
  six checkpoint floats, and preserves the stack and audio-pool reservations.
  All 3,152,732 randomized samples and 496 forced intra-chunk early-stop
  checks matched output and state, as did the 8,205,582-frame full kit
  host comparison. Standalone total linked XIP increased 96 B on each.
- [Bass mix prestage](owner48-held-bass-prestage.patch) was already
  measured in the previous pass. It is confined to the stress fixture,
  borrowing a retired part's buffer and respecting all real bus stages.

## Uninstrumented comparisons

All comparisons use the same canonical controller per board, 8,000 ms
strict pinned emulator, fresh NOR, 128 held blocks, six real Synth One
voices, and both kits, 132 witnessed hits each. Untimed arms are labeled.

| Canonical Elite diagnostic | Full-run XRUNs | Held >11.61 ms | Worst held µs | Free XIP |
| --- | ---: | ---: | ---: | ---: |
| Exact d5 reference | 48 | 34 | 12,751.21 | 1,376 B |
| Bass prestage previous alternate | 43 | 34 | 12,747.92 | 1,168 B |
| 909 fused standalone | 43 | 30 | 12,784.08 | 1,456 B |
| 808 cymbal standalone | 40 | 28 | 12,718.88 | 1,472 B |
| Cymbal + 909 fused | 37 | 29 | 12,790.92 | 1,552 B |
| Cymbal + bass prestage | 31 | 23 | 12,737.25 | 1,296 B |
| Cymbal + 909 fused + bass prestage | **26** | **20** | 12,752.00 | 1,376 B |

Standalones differ from the exact combined result. For example, fused
909 alone on Pro gave 50 (not a performance win over 49), while the
cymbal alone gave 43, 33 held misses and 12,724.46 µs with 1,480 B flash.
No individual deltas were added together as a predicted timing claim.

## Rejected or untimed paths

- PCM-unit interpolation is algebraically exact (integer PCM and its
  difference are binary32-exact, and all nonzero intermediates stay
  normal under a power-of-two rescale). Full host engine/state passed
  23,280 blocks at O0 and O2. Linked size and relevant function start
  addresses are unchanged, but strict Elite shows 51/39 XRUN/held
  physical misses. Do not adopt based on a lower single peak.
- CRC-only: 1,376 B free, no size win; strict Elite 53/43. With the old
  duplicate runway the result leaves 992 B, below the 1,024 B floor.
- Exact storage copy order changed cold flash code without changing saved
  bytes or read ordering; it frees 48 B standalone on Elite. The older
  runway loop plus storage frees 64 B over the older runway alone, so
  fits Elite/Pro at 1,040/1,048 B. Elite 49/39 is still a regression
  vs d5, and the storage-only control 56/43 is slower. Don't adopt.
- Previous direct voice index brought the old runway to exactly 1,024 B
  flash, but strict Elite gave 55/44. Do not adopt. A selective generic
  sampler outline failed the floor at 912 B and was not timed.
- Zero-LFO outputs still require the exact waveform and smooth history.
  The per-sample shortcut passed 3,357,120 host stereo frames and all
  state but produced 61/48 on Elite, with 1,024 B free. Block-advancing
  silent LFOs also passed the exact host suite; 992 B free, not timed.
- 1-in-8 Synth One control-refresh outline passed 3,357,120 host stereo
  frames and full state; 1,408 B XIP, but 57/48 on Elite. Voice-local
  exact ladder/filter-mix fusion also regressed (60/49 and 1,360 B).
  These are not release optimizations.

## Paired result and when to use each composition

| Both-board strict replay, same dry Synth One + two kits | Elite | Pro |
| --- | ---: | ---: |
| Previous bass-prestage alternate dropouts | 43 | 46 |
| Cymbal + 909 fused dropouts | 37 | 32 |
| Cymbal + 909 fused + bass prestage dropouts | **26** | **25** |
| Held >11.61 ms, full combination | **20/128** | **20/128** |
| Held >10,809 µs engineering target, full combination | 128/128 | 127/128 |
| Worst held, full combination | 12,752.00 µs | 12,772.00 µs |
| App XIP free, full combination | 1,376 B | 1,384 B |
| Pool margin past retained reserve, full combination | 43,316 B | 43,316 B |

The lower-dropout full combination keeps the 10,080 B physical display
saving, restores 208 B of app margin per model relative to the old
prestage alternate, and uses no added persistent audio RAM. Technically
this is fewer misses, not appreciably faster worst-case audio: the
slowest sustained blocks still exceed physical timing by ~1.14/1.16 ms.
Both have no skipped emulator cycles or oversize requests; no wet FX,
hardware, installer, or release qualification.

Recreate the four-patch d5 composition using the earlier shell scripts,
then apply **808 cymbal**, **909 fused**, and **bass prestage** in that order.
The full composition has [Elite](owner48-runway-cy-fused-prestage-elite.json)
and [Pro](owner48-runway-cy-fused-prestage-pro.json) uninstrumented
measurements. Separate pre-DMA audio runs matched **all 160**
ordered blocks on [Elite](owner48-cy-full-fingerprints-elite.txt)
and [Pro](owner48-cy-full-fingerprints-pro.txt). These cover held and
release samples under this fixture, not every parameter/preset.

If the benchmark-only buffer reuse is unsuitable for a future general
renderer, keep the two drum patches without bass prestage:
[37/32 XRUN](owner48-runway-cy-fused-pro.json), 29/24 held deadline
misses, 1,552/1,560 B XIP free, same audio-pool margin.
It also matched all 160 ordered audio fingerprints on
[Elite](owner48-cy-fused-fingerprints-elite.txt) and
[Pro](owner48-cy-fused-fingerprints-pro.txt). Both variants are
test-only until transition, fault, peak deadline and wet workloads
are qualified.
