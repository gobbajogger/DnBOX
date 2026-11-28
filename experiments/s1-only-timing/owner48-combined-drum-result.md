# 48-row Synth One diagnostic: combined drum hot paths

Start with the two-tom recipe in [owner48-scheduler-result.md](owner48-scheduler-result.md), commit `4661a5d`. Add [808 fixed-product reuse](owner48-808-metal-products.patch), [808 kit pan-mask reuse](owner48-808-panmap.patch), and [909 fixed-drive phase](owner48-909-fixed-drive-phase.patch) in the same source tree. The four patches touch independent source hunks. Keep `DNBOX_LCD_CANVAS_48=1` and `DNBOX_LAB_LEGACY_SEMANTIC_RENDER=1` from the two-tom run. No build-time alignment padding, stack relocation, timing override, reverb change, or loss of controls is included.

| Strict native loaded replay | Elite | Pro |
| --- | ---: | ---: |
| Compact baseline dropouts (7c6d5855) | 118 | 120 |
| Lower-dropout baseline (f755207e; only 320/328 B XIP free) | 108 | 106 |
| Two-tom split dropouts (4661a5d) | 85 | 89 |
| Combined dropouts | **79** | **78** |
| Combined worst held block (µs) | 12,843.46 | 12,880.67 |
| Combined worst whole-playback block (µs) | 15,147 | 15,129 |
| Held blocks over physical deadline | 57/128 | 57/128 |
| Free app XIP bytes | **1,392** | **1,400** |
| Free audio pool beyond the retained 8,192 B reserve | 43,340 B | 43,340 B |
| Display canvas (48 rows) | 23,040 B | 23,040 B |
| Oversize display requests / skipped emulator cycles | 0 / 0 | 0 / 0 |

The no-stack display cut remains **10,080 B** compared with the 33,120 B canvas. The loaded fixture retains six dry Synth One voices and both drum kits. The audio check was a separate 8-second strict Elite replay of both original guarded compact and combined sources with the same pre-DMA instrumentation: **0 of 160 fingerprints differed**. It does not claim sample-by-sample equality for other scenes or for Pro. Host tests for the 808 paths matched 8,205,582 generated full-kit frames, all four output buses and state; the 909 fixed-drive change matched all 75,497,473 tested phases and 23,280 full-state blocks.

Results: [Elite timing](joined-hot-elite-timing.json), [Pro timing](joined-hot-pro-timing.json), [Elite pre-DMA fingerprints](joined-hot-fingerprint.txt).

A separate placement-only Elite control shifted native code by 16 bytes without changing hot routine sizes or DSP; strict dropouts fell from 118 to 104. In a short emulator profile over the same 3,200–4,100 ms guest-time window, modeled flash instruction misses by core were 70,600/30,852 unshifted vs. 66,738/33,741 shifted. The windows are not normalized by rendered frame count. This supports checking placement sensitivity; it does not establish the hardware mechanism. The combined result above does not rely on padding.

The 10,809 µs engineering target and approximately 11,610 µs physical block deadline are still missed under load. No zero-dropout, wet-effects, physical-device, installer, or release qualification.
