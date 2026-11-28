# Skip the plan search discarded by the forced reverse diagnostic

On the [eight-dropout source composition](owner48-zero-imearly-result.md),
apply the existing `owner48-peak-skip-forced-planner.patch`. Within the
lab's already-forced two-kit reverse path only, select its predetermined
mask directly. Outside that path the unmodified adaptive planner still
runs. This disables neither core, nor any engine, drums, keys or effects
that were active in the earlier same-input diagnostic.

| Strict uninstrumented replay | Elite | Pro |
| --- | ---: | ---: |
| Full-run dropouts | **2** | **2** |
| Held halves above physical | **1/128** | **1/128** |
| Worst held | **11,629.625 µs** | **11,620.375 µs** |
| Remaining worst held over 11,609.98 µs | **19.645 µs** | **10.395 µs** |
| Held halves above 10,809 µs target | 19/128 | 19/128 |
| App XIP free | 1,028 B | 1,036 B |
| Audio-pool margin beyond unchanged 8,192 B reserve | 37,620 B | 37,620 B |

[Elite timing](owner48-zero-noplan-elite.json), [Pro timing](owner48-zero-noplan-pro.json).
Actual workload remained six dry Synth One voices and precisely 132
hits per kit, with 48 rows / 23,040 B, 10,080 B display saving, zero
oversized requests and zero skipped emulator cycles. The independent
pre-DMA proofs show 0/160 different ordered audio blocks on
[Elite](owner48-zero-noplan-fp-elite.txt) and
[Pro](owner48-zero-noplan-fp-pro.txt) versus the exact 402bd0e4 base.
The observer's binaries are solely for waveform proof, not timing/fit
qualification. Neither board has a zero-dropout pass. No normal
scheduler, wet engine, hardware, installer or release claim.
