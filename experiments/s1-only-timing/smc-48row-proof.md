# Lossless display buffer feasibility: 64-row next target

This is a scoped diagnostic, **not a 64-row firmware build**. This patch leaves the
global canvas at 82 rows. It reduces the existing SMC immutable grid tape's live
footprint from 38,640 B to 28,560 B through a 48+21 window per main band. It keeps
the same tile dimensions, hash addresses and display coordinates by starting the
second window at row 48, a multiple of the 16-row tile height. The existing
single recorded scene survives all four renders and any foreground input during
LCD DMA. The fixed radius-8 encoder arc centred at y=36 belongs to the first
window; it is not drawn on the other windows.

At equal compact Synth One diagnostic source, the actual SMC A/B regression
passed both builds, with all 19 output PNGs byte-identical. The app also links on
Elite. Compared with the same source and preceding BREAK RAM patch, SMC proof
added 192 B app XIP (2,660 -> 2,468 B spare), no change to ordinary RAM or pool
(27,020 B pool margin). ARP and unrelated engines were not altered for this
prototype.

## Real image compression check

RGB565 conversion and exact round-trip with RLE and deflate tested on 158
actual host screenshots: 139 ordinary controls and 19 SMC scenes. The ordinary
reference matches the existing 82-row display in all 139. Maximum per-region
RGB565 palette entries: header 64; first main band 111; second main band
127; footer 77. One-byte run length plus two-byte pixel RLE maxima are
4,014 B header, 14,730 B first main, 9,855 B second main and 10,974 B
footer. These are observations, not guaranteed runtime maxima. DMA requires
linear uncompressed pixel words and the glyph compositor uses random reads
and writes of previous RGB565 pixels for exact blending. Compress-after-draw
does not remove the raster, and an unconditional 8-bit adaptive palette is
not valid without a bounded overflow fallback, notably for boot assets.

## What remains before claiming an 8,640 B global saving

- Convert 69-row main families to 48+21 and 82-row footer families to 64+18,
  including normal controls, SMC/PART and CLIP KEYS, help/confirm and boot or
  any other special stripe. Aperture and logical glyph positions must not move.
- Capture immutable scene state before first raster/commit. A naive replay of
  live UI reads after input pumping can show two different scenes; disabling
  foreground input until final draw would violate the existing service bound.
  Keep original dirty-tile hashes and DMA packetization where possible.
- Independently verify all display modes, theme variants, dynamic input
  mutation across every seam, boot fonts/bitmap, and existing screen coherence
  negatives. Time ordinary foreground input and worst dirty/cold refresh (not
  just LCD bandwidth), and check strict native audio timing on both boards.
- Only then reduce CV_ROWS from 82 to 64 (39,360 to 30,720 B). The extra render
  work, compressed palette or replay pressure could still change the best
  choice. No refresh/zero-dropout qualification is claimed here.
