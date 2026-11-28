# Lossless live display: 69-row diagnostic allocation

Apply only `display-lossless-69.patch` after rebuilding the preferred six-voice
stack in `across-renderers-result.md`, without the alternate renderer trials.
It already includes the constituent CLIP, main tape and footer changes; do
not apply those a second time. No hardware, installer, release, or default
SKU was changed. This is native firmware in the diagnostic lane.

The old 240×82 RGB565 array was 39,360 B. The new 240×69 array is 33,120 B,
a **6,240 B reduction on each model**, with all RGB565 calculations unchanged.
Uses localized lossless raster windows, not post-draw RLE with a second buffer.
Legacy main painters retain their original 69-row raster. Boot uses four
60-row strips. CLIP KEYS uses 63+19 in its original full-width DMA chunks,
with one immutable snapshot. SMC ordinary and legacy PART footers retain
their previous pixels through bounded windows. LIST and SMC command tapes
move behind a 48-row window; only the LIST-owned footer uses 48+34 so its
first window doesn't trample the tape. Ordinary ROI and original tile
coordinates remain separate.

| Native diagnostic | Elite | Pro |
| --- | ---: | ---: |
| Linked app flash free | 2,020 B | 2,028 B |
| Audio pool beyond unchanged 8,192 B reserve | 33,260 B | 33,260 B |
| Strict two-kit, six-S1-voice held peak | 14,065.42 µs | 14,065.83 µs |
| Whole-run peak | 15,293 µs | 15,254 µs |
| XRUNs | 151 | 151 |
| Oversized canvas requests | 0 | 0 |

Compared with the 82-row composition, ordinary RAM stayed at 95,392 B and
the display stack cost 1,168 B of app flash per model. Both timed runs
witnessed all six actual held voices, 132 genuine hits per drum kit, and
zero emulator skipped cycles. Elite LCD transmission was 646 ms of 8 s
(82-row Elite was 650 ms); Pro was 647 ms. These measurements are from the
specific workload, not a guarantee about every page's refresh deadline.
All 128 held halves still exceeded budget on each model. **Not a zero-dropout
or release pass.**

Host checks used the actual 69-row allocation and matched 1,987 full-frame
outputs against the prior 82-row reference. The standalone boot harness
matched all 24 complete frames (six themes and three custom-picture
variants plus factory); CLIP matched 64 mutation scenes and 18 original
captures; 270 LIST cases at 69 rows were pixel-exact under injected input
changes. SMC special-panel seam tests use real bounded LCD service and
include an intentional broken-arm check.

Use `read-display-fit.py` alongside the usual native replay and
`read-loaded-timing.py`. Additional host-only fixtures are separated and
never compiled into deliverables. Next size step is 64 rows, another
2,400 B, after removing the last coherent 69-row painter dependencies. Do
not lower the global capacity until that work is verified.

Separate native Elite fingerprint builds of 82 vs 69 rows matched all 160
ordered pre-DMA audio fingerprints. The 151 deadline misses are a
performance failure, not a detected audio math regression. The normal
timing figures above are from nonfingerprinted builds.
