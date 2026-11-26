# Recover most of the SMK-37 canvas RAM without changing pixels

Evidence pin: exact both-model dual-room ELF/source in evidence.log.
The largest canvas symbol is cv_px, 66,240 B (240×138×2) in .pool
on both. An 82-row allocation is 39,360 B, **26,880 B gross**. Retain the
separate 1,536 B tile DMA buffer and existing 540 B main tile hashes.
Deduct a single bounded page snapshot in a union, not all page snapshots,
and the additional stack high-water. The actual Home witness is 904 B,
so a Home arithmetic equivalent is 25,976 B net before extra stack, **not
a device or whole UI promise**. A 2,048 B snapshot cap would retain 24,832 B
estimated net before stack; that cap is a design target, not measured.
Changing only boot or Home recovers no static pool while other paths need the old buffer.

## All direct consumers

| Source path | Actual region | Migration/risk |
|---|---|---|
| gfx.c:431–448 stock/custom startup picture | 2×120 before normal UI loop | Replace with 3×80 and crop immutable bitmap in source coordinates. Tested 84 validated payload/palette/font-presence cases, all 57,600 RGB565 pixels exact. Same bytes, one extra boot-only window, largest 57,600→38,400 B, pending-source and 82-row canaries. |
| ui.c:9899–9938 normal draw_main | 138, semantic 69+69 | Geometry is strip-safe: bands already clip independently. Reads are not. Preserve local semantic coordinates through physical 80+58, immutable frame state. Covers Home/live Home, FX, Mix, Song, optional Perf, Drum/Drum Voice, Bass editor (S1/SJ6/Chicago/Miami), Sequencer, Gen, BREAK, overlays and readout. |
| ui.c:9880–9885 WAITING FOR LOOPY PRO | 138 constant + palette | Use same 80+58 and existing dirty hashes/windows, capture waiting owner and palette once. |
| main_fm1.c:631–701 smk_clip_lcd_draw | 138 Clip Grid or Scale/Chord/Arp page | Split rate/state check from paint. Freeze view map, in-view, velocity, queued flags, geometry and keybed. USB IRQ 2 kHz owns cell writes; consistent bounded snapshot, not long IRQ disable. ck_page_draw reads live cp/settings/keybed (different from 82-row knob panel). |
| smc_mixer.c:1245–1306 peer overlay | 20 header, 138 grid, then 82 knob | Freeze resolved grid controls, values, and palette/cursors from the existing point after header emit. Full grid/fill crosses strips; no duplicate meter/time updates or direct live reads. |
| ui.c:8359,11713–11864, SMC headers/knobs, main_fm1.c:1845 | 20 or ≤82 | Geometry already fits; keep current hash/direct-full-row DMA and cover overlay restoration. |
| gfx.c:415–428 one-shot text | ≤240 wide, font height <82 | Calibration, crash, safe/update/UBOOT and notices use this or single-row fill. No full canvas. |
| lcd.c:84–111 optional LCD_CANVAS_FILL_SCRATCH | ≤240 words | cv_px alias only after lcd_sync; otherwise dedicated row buffer. cv_blit_from has no caller. Keep all fills out of returning paint callbacks. Fatal takes over, not resumes. |

## Smallest change set worth qualifying

1. Add vertical intersection helper at gfx primitive layer. For semantic band
   b, origin=69*b−physical_base, clip_lo=max(0,origin),
   clip_hi=min(physical_rows,origin+69). Skip empty intersections, keep local
   coordinates, blend and byte order. Full-height pure painters use
   origin=−physical_base and clip [0,physical_rows). Audit line/rect/text
   blend reads and writes, hashes, backing DMA and optional fill alias.
2. Main split exactly 80+58: hash tile rows [0..4] then [5..8] at existing
   stride, preserving tail row, adjacent dirty merges/windows/order/bytes.
   A 69+69 DMA split changes the tile y=64–79, DMA windows and input cadence.
   Initialize region force/hash flags once before emit. Do **not** reset or
   rearm validity on second-strip entry. The actual foreground input path
   can reset UI/cached hashes mid-DMA. Latch initial force and generation;
   complete the frozen old frame consistently but guarantee next tick
   fully repaints on owner change or invalidation. SMC/Clip use same commit.
3. Prepare current main page once, then paint only from an immutable view.
   Capture visible fields/control words, strings and pattern glyphs plus
   palette/fonts at the existing render point, not whole project or audio
   state, and not pointers to live/stack state. High risks beyond default
   Home: MIX scope/peak/history latch, 303/S1 arp copy and temporary arp_bp,
   s1_draw_patch stack pointer/typed controls, BREAK waveform/preview,
   list BLE scroll/time, held readout, SMC resolved controls and Clip IRQ.
   Static waiting/help/ask have simpler snapshots. All stateful preparations
   run once; capture a band's distinct displayed data as needed rather than
   suppressing genuine baseline state updates. No long IRQ mask.
4. Change startup path (tested here), keep already-fitting consumers,
   set CV_ROWS=82 only after final active-path conversions. Add hard
   development assertions for requested bounds rather than depending on
   cv_begin's silent clamp. Check both native map deltas and stack
   watermark before claiming net space. Canvas belongs to display, not
   speculative audio borrowing; both active rooms retain own histories.

## Exact acceptance and target risks

- Compare every enabled page for both models with same seeded UI/event
  scripts: whole RGB565 panel, main tile hashes, ordered LCD windows,
  exact window payloads and transfer sizes. Include cold/no-op/one edit,
  all palettes/live and fallback fonts, playheads/cues, changes spanning
  rows 68/69 and 79/80, readout crossing split, SMC/Clip/normal handoff,
  stable and live list, Clip delay/refresh and USB feedback. Boot has
  intentionally different window partition, same static pixels/bytes.
- Inject real navigation/edit/reset callbacks and Clip IRQ notifications
  after first, boundary and final DMA. Frozen frame remains coherent, next
  frame matches reference/new project; cache invalidation retained across
  second-strip submission. Existing Home negative actually fails naive
  live recapture: 33,120 exact main pixels, 45 cold/0 stable/16 changed
  windows with frozen state. Host async DMA verifies the source memory
  remains immutable until fence; use existing 1,536 B interactive ceiling.
- LCD commit runs input(), division_pad_input(), smk_ck_poll() inside DMA
  chunks, at most once/ms. They may change palette/UI owner/view/project or
  apply notes. No nested ordinary draw found in callback; reset/restore
  can invalidate draw hashes and clip state can also change in IRQ.
  Never move these callbacks after the full SPI frame. Repainting second
  strip creates an extra raster interval between DMA tile rows 4 and 5,
  potentially increasing the worst input gap. Each unchanged frame
  still hashes all tiles. At 12 MHz, max tile payload is 1.024 ms on wire
  before command delays; measure real key/MIDI/audio dispatch.
- cv_begin waits even when preceding dirty DMA source is only cv_lcd_txbuf.
  Preserve fence initially; optional overlap optimization requires an
  explicit pending-source tag because full-row/fill/boot can read cv_px.
  Do not reuse the bounce buffer until completed; physical LCD scanline
  can still tear despite exact emulator pixels. On both models capture
  dirty/cold/worst SMC/Clip/Mix repaint alongside populated heavy
  audio, MIDI/keys, foreground timing, max stack/pool and zero dropouts.
  No native timing or released memory has been claimed here.
