# Keep the 48-row cut and move only two 808 toms to Core 0

Diagnostic only. Start from the owner48-margin.md recipe
(7c6d5855968bbaff49e484e15487b4f7383bc768). Apply
`808-rebalance-r2.patch` after the prior three-tom split. Kick, snare
and the first two toms render on Core 0 before the six-voice Synth One;
the third tom goes with the 808 remainder to Core 1. This is a contiguous
prefix, so lane addition order and all voice/audio state are preserved.
Both drum kits remain real, all six notes remain held. Only the existing
six-voice stress fixture's reversed path selects this variant, not a new
release scheduler or a released app.

| Strict native | Elite | Pro |
| --- | ---: | ---: |
| Physical display allocation | 23,040 B | 23,040 B |
| Audio-pool margin above unchanged reserve | 43,340 B | 43,340 B |
| App flash free | 1,088 B | 1,096 B |
| Dropouts (two-tom split) | **85** | **89** |
| Owner48 preceding dropouts | 118 | 120 |
| Previously better guarded48 dropouts at 320/328 B XIP | 108 | 106 |
| Held peak | 12,950.04 µs | 12,957.17 µs |
| Held halves exceeding 11.61 ms | 63 / 128 | 64 / 128 |
| Whole-run peak | 15,319 µs | 15,352 µs |
| Oversized draws / emulator skipped cycles | 0 / 0 | 0 / 0 |

Meets both RAM and app-flash objectives and improves the loaded test.
It does not pass the 11.61 ms physical period or the 10.809 ms engineering
target and must not be released. This is the dry two-kit test, not a wet
or all-preset qualification.

## Placement control

Without changing algorithms, `layout-shift-16.patch` places 16 padding
bytes immediately after the startup section. The four sampled hot
functions moved +16 with identical symbol sizes. Native owner48 went
from 118 → 104 (Elite) and 120 → 104 (Pro). Repeat Pro in independently
isolated scratch produced identical metrics; none are physical-hardware
numbers. Moving by 48 instead yielded 112 XRUNs on Elite. Keep placement
as a diagnostic control, not a robust optimization floor: changes later
in the layout can undo it. This proves placement affects measured timing
but does not establish a particular cache-miss mechanism.

Separate same-sample Synth One control-LFO reuse
(`s1-kr-lfo-same-frame.patch`) qualified exploratory Elite timing only:
104 XRUNs, 13,073.71 µs held, 1,072 B XIP, unchanged pool. Not needed in
the better pair.

Separate strict Elite audio-fingerprint builds for the unpadded owner and
two-tom arm both completed the original full 8,000 ms virtual scenario.
The existing script counted 160 ordered pre-DMA blocks; all 160 were bit
exact. The timing numbers above are from the uninstrumented binaries.
The first fingerprint attempt shared a full scratch filesystem and was
discarded. The second short 6 s test had not accumulated 160 blocks and
was discarded. Use 8 s, fresh exclusive virtual NOR, and the native
render-fingerprints.patch. This proves the tested fixture, not all sounds.
