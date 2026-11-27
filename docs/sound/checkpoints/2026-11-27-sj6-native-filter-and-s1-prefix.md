# SuperJ6 typed native filter and Synth One prefix

Diagnostic-only. No production firmware, selected factory banks, on-device writes, or full engine sound certification.

## SuperJ6

On the guarded native PI32 harness at the standard 360 MHz emulator clock, the registered two-source typed path reproduced all 148,684 signed compact-host output words including both live retargets, releases and a chord. Independent Main/Sub histories each ran 174,199 filter updates, with 4 cold resets, 32 ramp updates and 8 events. This is equality between executions of the compact candidate, not comparison to final installed-AU audio. The original/compact same-input filter comparison is scoped to the earlier taps.

Stock peak: 3835.50 µs per 128-frame chord block. Candidate peak: 7044.00 µs. Typed direct history copies retained all output words and reduced peak to 6666.75 µs (377.25 µs recovered; 2831.25 µs above stock). Keep the solver out of the product path until qualified. Source-only paired measurements/checks: Library `libfile_fde3_f-935b61e91a1481918be3bbd62a89fa53_2021479872048191acbfdca1f870a539`, SHA256 `111fc579788ef610440e8780a13ea3f899e18e7b8626b54db0d892be52056f0a` (first typed pair, before direct copy).

Cutoff oracle: the previously reported 540/1,000,000 g differences reproduce from substituting host glibc float math for separately rounded Python math, beginning in log2f for 139 and exp2f for the other 401. Original compiled ARM64 caller experiments used shim math, not independently authenticated installed Apple math. Do not fit a glibc-specific correction or claim all Apple coefficient inputs match.

Independent Raw/Main held-out keys match measured pitch drift. The known downstream transfer explains most of Raw/Sub relative H3 and H5 differences (about +0.34 and +0.37 dB residual), not the larger level/release mismatch. Warm Brainwave, same physical note, media and fixed cutoff: actual original DSP vibrato restart moves Main +0.04719→+0.02066 Hz and Sub +0.00498→−0.00816 Hz versus installed Main +0.01471 Hz and Sub −0.01164 Hz. Zero-depth restart/no-restart is bit-identical; A/B level change is <0.004 dB. Source clock startup is causal for most observed phase drift in this experiment. The native terminal captures did not allow fixed-cutoff/depth interventions. The exact installed restart condition, level and release remain open. No master-gain or per-preset tweak.

## Synth One and combined fit

The host two-thread prefix matched signed PCM and complete DSP/RNG state across 29,883 frames, 179,298 voice samples and 477 live edits. A 240 B target view and ordered 64 B/16-frame prefix output preserve noise order and summation order. Sparse variant borrowed existing stable control plan and matched 29,959 frames, 179,754 voice samples and 477 edits, with 9,528 versus 1,378,114 smoothing updates per core in this fixture. No native latency claim yet.

Moving only two cold UI temporaries (256+92 B) out of pool added 348 usable pool bytes on both models at unchanged app XIP. It leaves 120 B ordinary static gap. Subsequent joint diagnostic link rooted the earlier exact Synth One kernel as well as the full typed SJ6 filter/map and both owners' independent histories. Both link:

| Model | XIP free | Usable pool | Above protected 512 B guard |
| --- | ---: | ---: | ---: |
| Elite | 388 B | 932 B | 420 B |
| Pro | 400 B | 932 B | 420 B |

Elite SHA256 `9c736b09d41011bd286e89492042fd61114d2e7b8665b9016b4345554db49a38`; Pro `5953002fd5b1c1819e3348e975ac225afa18ca88588df04070ad9da2e1f4c700`. Component proof, no voice dispatcher and no active nonlinear integrated render. The private prefix needs an overwrite-safe arena through helper acknowledgement, and the timed decision must precede the first dependent dry sample. No use of skipped cycles or report-format gates to claim qualification.
