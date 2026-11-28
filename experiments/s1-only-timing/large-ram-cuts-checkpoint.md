# RAM cuts after the paired Synth One bench

Base is diagnostic `f99f3464506bd361262c285cdf312890ee9512a4`
(native donor `873ec0a44a31e497b995c5329d10d225d4f8cf1d` plus the
69-row and paired ADSR/808 overlays). Everything here is diagnostic and
unpromoted. No devices, main, installer or release changed.

## Immediately usable pool relocation experiment

Apply `project-adjacent-bss-append.patch` alone. Existing unused ordinary
RAM now backs the selected BREAK metadata and foreground key bookkeeping.
Both models gain 2,280 B usable audio pool beyond the unchanged reserve
(33,260 → 35,540 B), XIP -16 B and ordinary RAM slack 632 B. This is
real pool headroom, not a reduction in total RAM and not a stack loan.
Read `project-pool-relocation.md` for ownership, pointer and project proof.

Separate strict native emulator runs confirmed six held Synth One voices,
two kits (132 genuine hits each), zero skipped cycles. The relocation
experiment gave 113 XRUNs on both versus 111 Elite / 108 Pro on the
pre-relocation diagnostic. Held peaks were 13,137.50 µs Elite and
13,148.96 µs Pro. Treat the timing difference as real for these links;
do not promote it as a timing improvement or a zero-dropout pass.

## 48-row display conversion (partly implemented, no full-pool claim)

Layer these in the following order onto the above original diagnostic
base (no RAM relocation required):
1. `boot-slice-48.patch`.
2. `main-scene-48.patch` for immutable ordinary main scenes and HELP/ASK.
3. `clip-keys-48.patch` then `clip-main-48.patch` for clip grid/page and
   full-width Clip Keys packet boundaries.
4. `live-display-48-prereqs.patch` and
   `live-display-48-ordinary-knob.patch` for LIST/SMC, ordinary and BREAK
   footer windows. The LIST and SMC tapes sit outside the 32-row live
   raster; ordinary screens keep RGB565 drawing and original hash tiles.

Cold boot: the actual gfx renderer, all 84 validated theme/font/picture
test cases matched 57,600 pixels per panel, DMA source was preserved
until sync, and the maximum window went from 28,800 B to 23,040 B.
Clip Keys: actual extracted painter, 64 combinations, all 2,519,040 B
matched; 28 transfer windows and one simulated state/theme mutation
at the first packet in every case. Unsupported live-state variants not
claimed. Main/combined ordinary route: 139 native-source host PNG
pairs matched pixel for pixel. LIST actual-48 local probe passed 270
mutation scenarios; SMC 19 prior fixtures matched after its change.
The initially joined subset linked on Elite with 212 B XIP and global
canvas still 69 rows. Legacy bass PART matrix and any other unsliced
paths must be bounded before reducing the global allocation, and more
flash headroom must be recovered before production. The 10,080 B
total canvas target has not been counted.

For ordinary A/B use `canvas-64-stack-routes.txt` with the actual
small-canvas host. Build with `X0X_S1C=1 X0X_SYNTHONE_PATCH=1`,
small-canvas/preview/tagged flags, and a host-only C object declaring
`volatile unsigned int bench_owner;` to link the native workload
diagnostic. Use `X0X_HOST_LOAD=0 X0X_HOST_DISPLAY_LOAD=0`.

For boot A/B, apply only the gfx portion of display-lossless-69.patch
to the donor gfx.c; use that as the `gfx.c` included by
`check-boot48.c`. Make `boot-candidate.inc` by extracting the patched
`draw_boot_screen` and renaming it to `draw_boot_screen_sliced`; include
this in the same test. Use matching donor headers and generated fonts.
For Clip Keys, in both sides run test extractor
`tests/host/smk_clip_knob_test.sh --extract-only` and
`tests/host/prepare_clip_keys_render.py`. Give the isolated test gfx
82 rows for its unsliced reference. Run `clip-actual.py` on the
extraction output directory, then compile `clip-actual.c` with the
extractions, actual `smk_clip_arp_share.c`, `-ffunction-sections
-fdata-sections -Wl,--gc-sections` and compare both binary outputs.
