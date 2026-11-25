# Full-graph fit: lossless RAM-only source change

Exact donor: all-controls-remote-join/ui-fit-outline/tree, guarded link recipe and
same control flags. No RAM/flash regions resized and no guard changed. The
second actual JieLi pair reached linker map output on all original geometry
guards; XIP still fails. There are no qualified ELFs, images, or audio tests.

| Item | Donor | RAM candidate, both boards |
|---|---:|---:|
| POOL used | 0x52a10 | 0x51fe0 |
| POOL available (capacity 0x54000) | 5,616 B | 8,224 B |
| Required POOL reserve | 8,192 B | 8,192 B (+32 margin) |
| BSS end | 0x1c1fac8 | 0x1c1ff54 |
| BSS capacity boundary | 0x1c20000 | 0x1c20000 (172 B free) |
| _c1_end and _rt_end | 0x1c0111c | 0x1c0111c |
| XPOOL used | 0x2400 | 0x2400 |
| Guarded XIP overflow, Elite/Pro | 27,932 / 27,924 B | 29,380 / 29,372 B |

Differences:
- Fixed per-line capacity packing: the four independent 16-bit room line
  buffers now allocate their exact observed maximum lengths 61/79/97/113.
  Both mode lengths, history indexing, feedback, and per-owner histories
  are unchanged. The Fatten 441-sample right line is untouched. Saves 408 B
  across both owners.
- Remove the 909 noise buffer's spare block: exactly 256 history + 256 maximum
  new words remain, same PRNG and float precision. Existing forward copy is
  safe even if overlapping because old source is at or above the destination
  (position never below 256). Filter history and sample order are unchanged.
  May perform a copy one block sooner; it needs later timing qualification.
- Move 634-byte PADLED SPI buffer, 256-byte live key queue, 32-byte owner
  descriptor, and 256-byte waveform scratch into measured ordinary BSS
  slack. No capacities, addresses exported on wire, or state owners change.
  Both RAM regions are zeroed on boot. SPI config writes a full ordinary-RAM
  physical address directly into FM1_LCD_SPI_ADR; its buffer is consumed
  synchronously under the same DMA fences. Main foreground fills PADLED and
  enqueues the keyboard FIFO, but the FIFO is drained only on core 0 before
  scheduling any core 1 work, with original barriers and locks. Waveform is
  a core-0 post-render write/foreground read as before.
- No new screen scratch alias: original donor already enables the LCD fill
  reuse in x0x.c. Existing full fills fence before reuse and the next content
  prepares a fresh canvas. No PMEM or guard scope change here.

A total 2,608 B is recovered from POOL including alignment. The observed XIP
penalty is +1,448 B vs this donor; many externally unrelated functions change
size between LTO layouts, so carry the measured cost, not an estimate that
this is flash-neutral. Before grafting donor-specific anchors, check current
combined build's BSS margin and link both exact boards again.
