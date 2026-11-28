# Exact UI CPU reductions, 48-row a54125b6 diagnostic

Independent combination against a54125b6, real 48-row allocation and all of
its approved diagnostics. No audio source, canvas allocation, DMA layout,
UI refresh target, or runtime behavior was changed.

- Matrix captured windows start filled with the captured background, identical
  to SOUND_BASE, and every captured matrix call enters on a pristine window.
  The second full-frame fill took up to 105,600 repeated byte writes per
  adjacent two main bands and footer and is removed only for 48 builds. The
  larger direct/fallback raster paths keep their existing explicit fill.
- Existing frozen glyphs may be culled only outside an expanded [-4,+16]
  pixel envelope around the control-row slab. That discards two complete row
  paints per four-row, six-window refresh (12 -> 10). All UI inputs, including
  progression of glide animation, are captured before the cull.
- The exact existing 16-step FNV hash is unrolled. Both arithmetic and order
  are unchanged. Elite's generated inner tile-row body goes from 69 issue
  packets to 36; this is a native instruction count, not elapsed time.
  No changes to dirty tile decisions, buffers, or transfer geometry.

The host hard-48 matrix smoke matched 115 PNGs exactly and all output files on
59 matched-entry scenarios. 56 succeed; 3 fail identically on both sides only
because their slot image fixture was not provided. Neither overran the canvas.
Independent randomized hash proof: 921,000 matched hash values on 20,000
frames, with matching saved hashes and identical 309,826 transfer rectangles.

Elite native link with both edits: 1,312 B app XIP free (80 B less than the
1,392 B donor), 95,424/98,304 B ordinary RAM, and 43,340 B beyond the unchanged
audio-pool reserve. Physical raster still 48 x 240 x RGB565 = 23,040 B.

Artifacts: owner48-matrix48-no-redundant-fill-and-offscreen-paint.patch,
owner48-lcd-hash-unroll-exact.patch, owner48-display-hash-{reference.h,candidate.h,test.c},
owner48-hash-exact.log. Root owns integration/replay. No
emulator XRUN, hardware, or installer claim from this lane.
