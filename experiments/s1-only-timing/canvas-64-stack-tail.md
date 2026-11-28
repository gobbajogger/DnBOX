# Experimental lossless 64-row canvas with scoped foreground tail

The main frame and legacy bass footer still need exactly 69 logical rows.
Instead of repainting a live UI after input and changing a scene mid-frame,
the candidate draws those 69 once. Rows 0–63 remain in the static RGB565
canvas; 64–68 use a scoped 2,400 B core-0 user-stack array. The owners are
sequential, so their arrays do not coexist. Overlays and tape renderers are
unchanged. The renderer still clips in logical coordinates, paints full-precision
RGB565 (including blended glyphs), and hashes physical tiles with the original
hash table positions and packet ordering. The tail is copied by the CPU;
DMA always sees the same dedicated static TX buffer.

Do not alias the existing main cold scratch: input pumped between display
transfers may perform a SAVE which immediately uses that entire scratch for LZ.
The UI pointer's live interval covers only its one synchronous draw/commit, not
a deferred USB callback. This alternative does increase foreground stack use
and consumes app flash, so it is not yet a promotion candidate.

## Measured receipts

- Fixed source Elite: pool margin 33,260 -> 35,660 B (+2,400 B); ordinary
  static RAM unchanged (95,392 B), XIP free 2,020 -> 1,812 B (+208 B image).
- 139 ordinary 69 vs 64+tail host PNGs byte exact, including menu and legacy
  bass-footer paths. No claimed full route gate.
- Separate marked-stack emulator candidate adds an early user-stack watermark
  below the startup live frame. The exact plain patch excludes instrumentation.
  Compare stack guard, canvas overflow counter, strict render, and audio separately.
  Even successful strict replay does not cover worst save/transfer/painter
  nesting or cold worst-case display refresh on all routes.
- Fixed source Pro: 33,260 -> 35,660 B pool margin, 2,028 -> 1,820 B
  XIP free, same 95,392 B ordinary RAM.
- Marked-stack Elite: 128 held halves, peak 14,058.71 us, 151 XRUNs
  (not a dropout fix), 0 skipped cycles, 0 oversize. The 10,496-byte
  foreground user stack retained 2,444 untouched marked bytes; deepest
  observed written byte was 8,052 B below the top. This is a limited
  witness, not full stack qualification or an increase in total RAM.
  LCD wire occupancy was 651 ms over 8 s; the earlier fingerprinted 69-row
  baseline shows 647 ms but uses different instrumentation. This is not
  a controlled refresh-latency A/B, and no timing/refresh approval follows.

## Native witness and reproduction

Both plain builds use the same published 69-row recipe plus
`canvas-64-stack-tail.patch`. Plain Pro: 1,820 B free app flash; both boards
have 35,660 B audio-pool margin past the unchanged reserve. Ordinary RAM
stays 95,392 B on each. For the separate Elite witness, apply
`canvas-64-stack-witness.patch` on top. Run the pinned strict 8-second
two-kit replay and `read-canvas-64-stack.py RUN_DIR`. It measured 8,052 B
touched out of 10,496 B of reserved foreground stack (2,444 B untouched),
zero oversized canvas requests, 651 ms LCD wire time, and no emulation
skips. Held peak 14,058.71 µs, 151 XRUNs. These are scoped diagnostics, not
all-route refresh or worst-case stack evidence.

For host reproduction, add only `volatile unsigned int bench_owner;`
to `host/x0x_host.c` in both builds (disabled diagnostic link shim). Compile
both with `X0X_S1C=1 X0X_SYNTHONE_PATCH=1
X0X_HOST_DEFS="-DX0X_S1C=1 -DX0X_UI_SMALL_CANVAS=1"
X0X_HOST_OUT=/tmp/path/to/sim bash host/build_host.sh`. Invoke both simulators
with `X0X_HOST_LOAD=0 X0X_HOST_DISPLAY_LOAD=0`, the supplied
`canvas-64-stack-routes.txt`, and separate screenshot directories. A recursive
binary comparison of those directories yielded 139 identical PNGs.
