# Trace the remaining slow audio halves

The starting diagnostic is [ca711d03, staged cymbal + fused 909 + bass prestage](owner48-runway-cymbal-result.md).
The exact strict Elite reference reproduced **26 dropouts and 20/128 held
physical misses**, 12,752 µs worst, 1,376 B app flash free and 43,316 B pool
margin above the existing reserve. Pro's paired reference was 25/20,
12,772 µs and 1,384 B free. No modes or samples changed.

## What is still late

The [fresh per-core probe](owner48-ca711d03-barriers-elite.json) shows the
same worst held generated frame 129,024 as the older d5 source. At that
frame Core 0 does 2,503.54 µs of 808 prefix and 7,347.13 µs active bass A
work, plus a 711.08 µs stage wait. Core 1 does 5,205.79 µs in 909 followed
by 4,791.50 µs in the rest of 808. The post stage is 986.46 µs. Moving
everything waiting on the 808 stage is not enough to close the deadline.
It is not sound to call wait time CPU available on both cores.

The [matched-frame ledger](owner48-peak-gap-elite.json) reconciles total
time without double counting (engine-level intervals are disjoint;
sub-setup, planner and launch are nested in render_sub). The extra
sequencer time over a quiet neighboring held block is **472.13 µs**.
The nested planner accounts for only ~48 µs at the slow frame, PCM
packing ~63 µs and scope/meters ~30 µs. The gap is not primarily USB,
a slow planner or a hidden post-mix volume pass.

A [further trigger split](owner48-peak-gap-triggers-elite.json) measures
the real 22 hits at that frame: **83.46 µs in 909 trigger DSP and 161.96 µs
in 808 trigger DSP**, none in the quiet comparison, and ~226 µs of the
extra time still outside both trigger calls. At early-release frame
171,520 it finds the same 22-hit pattern and similar trigger costs.
Don't treat all 472 µs as DSP trigger time, or assume eliminating all
trigger work would meet the deadline.

The probes keep original inputs but perturb timing/layout (28/29/41
XRUNs on three instrumented variants, vs 26 without probes). Never
install the instrumented images: the extra instrumentation is only a
scientific tool and doesn't meet the 1,024 B XIP candidate floor.
Rebuild the ca711 composition, then apply
`heavy-barrier-timing.patch`, `owner48-peak-gap-profile.patch`, and
optionally `owner48-peak-trigger-profile.patch`. Decode with
`owner48-peak-read-gap.py [--trigger-split] RUN`.
The [PC weighting](owner48-ca711d03-pc-held.txt) still highlights
Synth One voice/morph and 909, but uses a wall-time window that includes
foreground/interrupts and is not a per-sample cost comparison.

## Not adopted

| Strict Elite arm | Full-run dropouts | Held physical misses | Held worst µs | Free app XIP |
| --- | ---: | ---: | ---: | ---: |
| Exact ca711 | 26 | 20 | 12,752.00 | 1,376 B |
| Shift from two to one 808 tom on Core 0 | 117 | 92 | 13,055.75 | 1,376 B |
| Shift from two to three toms | 96 | 83 | 13,074.08 | 1,376 B |
| Skip overwritten planner search in forced spans | 26 | 21 | 12,785.33 | 1,344 B |
| Stage same-sample 909 hits for its render owner | 34 | 25 | 12,746.54 | 1,040 B |
| 808 cymbal front + envelope staging | 36 | 28 | 12,721.88 | 1,344 B |
| 808 local bank phases | 32 | 24 | 12,747.00 | 1,296 B |
| Factor exact cold project duplicate scans | 29 | 23 | 12,692.08 | 1,632 B |
| Remove redundant post-master PCM clamp | 27 | 21 | 12,711.08 | 1,572 B |
| Cold scans plus PCM clamp removal | 29 | 22 | 12,733.04 | 1,828 B |

The exact cold refactor and redundant clamp removal are source/flash
alternatives, not verified timing fixes. The cold scan proof checked
2,359,296 exact cases and the clamp keeps 160/160 native Elite audio
fingerprints. The stage-909 variant is lab-only and uses a bounded
ordered register of accepted hits; it did not pay off (nor has it been
fault-tested). Do not turn those experiments on in normal firmware.
The four-frame Synth One oscillator tile also matched host state but
left just 624 B XIP and was not eligible for timed evaluation.

The next high-leverage serial experiment should isolate sequencer
metadata, MIDI/event dispatch and lane setup at actual simultaneous
hits, then compute repeated trigger setup off the critical path only
when exact state/automation order can be proven. Subtracting any single
one of the measured small bins is not a full deadline solution.

The 808 bank trigger ratio cache also passed exact host output and old-state
comparison in all 8,205,582 frames (excluding the private 4-byte memo key),
but was 29 XRUN / 23 held misses, 12,759.79 µs, 1,344 B XIP on Elite.
Do not include it simply because it removes repeated arithmetic.

## Measured output-pack and 909 envelope composition

Apply [the PCM and scope fusion](owner48-peak-scope-pcm.patch) and
[the local 909 sample envelope](owner48-909-sampled-envelope-local.patch)
on exact ca711d03. The PCM loop consumes already-bounded master samples
for conversion and scope capture in a single pass, publishing the
scope index once as before. The rendered output and scope index both
matched the bounded predecessor in 100,000 host cases. The 909 interior
sample renderer caches its envelope accumulators only inside the span,
materializes them at the existing state boundary, and preserves the
envelope sequence and decoder math. Both O0 and O2 host
comparisons matched 23,280 four-bus/full-state test blocks. Individually,
the full scope fusion yielded 25 XRUNs on Elite and the 909 arm 24;
their standalone results do not predict the stacked outcome.

| Canonical strict loaded test | Elite before → after | Pro before → after |
| --- | ---: | ---: |
| Dropouts | 26 → **23** | 25 → **23** |
| Held blocks over physical | 20/128 → 19/128 | 20/128 → 19/128 |
| Worst held, µs | 12,752.00 → 12,647.75 | 12,772.00 → 12,700.29 |
| Held over 10,809 µs target | 128/128 → 126/128 | 127/128 → 126/128 |
| App flash free | 1,376 → 1,268 B | 1,384 → 1,276 B |
| Audio-pool margin past retained 8,192 B reserve | 43,316 B | 43,316 B |

[Elite timing](owner48-peak-scope-env-elite.json) and
[Pro timing](owner48-peak-scope-env-pro.json) were run without probes.
Both observed six actual Synth One voices, 132 actual hits per kit,
0 skip and 0 oversize with 48 rows, preserving the 10,080 B raster saving.
All 160 ordered pre-DMA audio block fingerprints matched separately on
[Elite](owner48-peak-scope-env-fp-elite.txt) and
[Pro](owner48-peak-scope-env-fp-pro.txt). This is a small paired speed
improvement at a 108 B app flash cost. The worst held section is still
about 1.04/1.09 ms over the physical period. No wet, hardware, installer,
normal-scheduler or release qualification is asserted.
