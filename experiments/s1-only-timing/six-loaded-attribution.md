# Six dry Synth One voices under both drum kits

Scratch diagnostics only. Use the donor, patch order, pinned native toolchain and replay in `dry-six-drums-result.md`. Do not install these, attach hardware, or apply them to an existing project.

## Verified solo improvement on one core

Apply `dry-six-stationary.patch` to the optimized six-dry baseline, without the existing parallel worker or drum-load patch. Reuse the original control-rate phase and filter arithmetic only when the corresponding inputs are stationary within an engine span. The glide/modulation and evolving envelope paths still run. The silent sub oscillator keeps advancing and adds the original signed zero, instead of interpolating its waveform unnecessarily.

Elite, 8 seconds: **10,936.17 µs held peak, zero XRUNs, zero late and zero skip**, 625 halves. One held half exceeded the engineering target of 10,809 µs; none exceeded the physical deadline. App margin 3,428 B and pool margin 26,492 B. Both caches were hit 54,684 times. Compared separate native correctness arms with `dry-six-base-fingerprints.patch` and `dry-six-stationary-fingerprints.patch`: all 320 stereo synth float output fingerprints matched (81,920 frames). Standalone host phase check covers 5,000,000 signed-zero cases with `subzero-check.c` compiled with `-DS1U_SOURCE='"path/to/s1u.c"'` and `-ffp-contract=off`. This is not all-preset, wet, or installed qualification.

## Loaded results

| Strict Elite scope | Held peak µs | XRUNs, entire run | Notes |
|---|---:|---:|---|
| Original six dry + both kits | 15,646.83 | 152 | 132 actual held hits per kit |
| Reclaim Core 1 after it does both kits | 15,636.29 | 152 | Zero useful Synth One worker jobs |
| 909 on Core 0, 808 on Core 1, then try sharing voice work | 15,995.58 | 152 | 278 worker jobs, worse peak |
| 808 meter/mix fused | 15,561.83 | 152 | Minor speed gain |
| 808 oscillator prefix signed-int sum | 15,713.00 | 152 | Rejected |
| 808 cymbal staged | 15,521.63 | 152 | Uses +2,208 B app, 3,152,732 host samples match output and full state |
| 909 local unsigned lookup | 15,653.71 | 152 | +1,344 B app, all 93,323,264 host steps equal; rejected |
| Stationary synth, unchanged drums | 17,900.71 | 151 | Solo speedup does not transfer as-is |
| Stationary synth + cymbal staging | 17,529.67 | 152 | Do not deploy standalone |
| Stationary synth + cymbal staging + reverse 808 body scheduling | **14,296.04** | **152** | 15,390 µs whole-run peak; still fails |

For the last arm, apply `dry-six-drums.patch`, `dry-six-stationary.patch`, `808-cy-staged.patch` and `808-reverse-body.patch` on the optimized six-dry baseline. Keep 909 and the rest of the 808 on Core 1, while Core 0 does the 808 kick/snare during 909, followed by the synth. The 808 output sum order is preserved; the rest only uses its stereo buses after the body publication. The strict run witnessed 314 split spans and 65,292 of 65,536 held frames using the route. App margin 724 B, pool margin 26,492 B. Separate correctness builds applied `render-fingerprints.patch` to both the original loaded and candidate arms. All 160 ordered stereo pre-DMA block fingerprints matched through held and release. Run `compare-six-loaded-dma.py BASE CANDIDATE`. Timing comes from the nonhash builds. Candidate remains far above deadline; no production, wet, Pro-loaded, full system or device claim.

Attribution per 512 generated frames in separate instrumented runs: Synth One ~10.90 ms Core 0 excluding stage waits; 909 ~5.16 ms and 808 ~7.35 ms Core 1. The 808 cymbal cost ~1.51 ms, its shared oscillator bank ~0.72 ms; the 909 sampled voices together cost ~1.14 ms including all envelope, drive and mixing. The latter decoded 202,014 nibbles in the held interval. Small lookup wins and moving whole jobs did not solve a real compute and coordination bottleneck. Do not assume incremental speedups add together. For the cymbal host check build `808-cy-check.c` with `-DD8_SOURCE='"path/to/patched/drum808.c"'` and `-ffp-contract=off`.
