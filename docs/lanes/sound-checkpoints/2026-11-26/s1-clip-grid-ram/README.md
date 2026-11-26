# Guarded native mutable-memory ownership (Elite/Pro)
Scope: exact S1 morph + compact FX guard, actual paired PI32v2 no-install ELFs. Both native models have the same addresses and sizes below. This is not a native source-matched wet build.

| Region / symbol | Live ownership | Reclaim decision |
|---|---|---|
| General POOL | 335,308 B of 344,064 B; 8,756 B raw slack, only 564 B after 8,192 B reserve | count 564 only. |
| Normal .data + .bss | ends 0x01c1ff54; 172 B placement gap | not contiguous POOL. |
| XPOOL | 9,216 of 9,216 B; workspace_b 5,120 and bass scratch 4,096 | independently active during parallel synth processing. |
| RAMTEXT; stacks; BLE; NOINIT | 1,764 nominal tail; 1,024-B Bluetooth sentinel; C1STK 4,608; BLE allocation 3,888/4,096, reset mailbox 308/1,024 | no borrowing or smaller reserves. |
| cv_px | 66,240 B POOL, 240×138×2; foreground plus asynchronous DMA | largest flagged non-audio candidate: final 82-row canvas would return 26,880 B gross. |
| cv_lcd_txbuf + hashes | 1,536 B BSS staged DMA source and 540+360+120 B hashes | retain. First dirty tile needs both buffers live; no size/truncation change. |
| live project, project stage, undo | 36,420 ordinary BSS + 36,420 POOL + 4,252 POOL | stage continuously shadows edits, not merely transfer; live save/editor/undo and canvas coexist. |
| x0x_main_scratch and st_buf | 4,096 B + 3,840 B ordinary BSS. First already shares OTA/editor/LZ/PERF lifetimes | overlaps project.c save_stream -> lz_compress -> st_put(io_buf); OTA status draws nested; PERF seq.pat stays in scratch during UI/DMA; no alias. |
| USB/ISR | capture 6,144, playback 4,096, audio DMA 8,192, command FIFO 4,096 B ordinary BSS | simultaneous with audio/ui, shrinking changes buffering. |
| original_bass, original_fx, sj6_unison_input, dly_buf | 24,424 B typed synth pair, 8,504 B per-part FX total, 8,192 B per-part Unison total, 138,601 B master delay all POOL | require proven per-slot ownership transition and retained tails; don't count for this lane. |
| Clip cell stores | 4×40 B IRQ-owned velocity/channel/flags/tick plus mapping | capture immutable 125 B foreground snapshot; never alias IRQ storage. |

## New focused host witness: actual Clip Grid rectangle body
Both arms extract the real native-pinned map/rectangles from main_fm1.c and run actual tile hash/merge/pack. They draw 138 rows or physical 80+58 with 125 B of frozen state. Native symbols anchored in native-memory.receipt. Exact raw comparison: 100 cold/stable/edit sequences, 4,656 ordered LCD windows, 13,843,920 trace bytes equal (geometry, actual RGB565 payloads, every cached tile hash). Stable frames have zero windows. Simulates mutation after physical-strip entry at the send boundary, not the race acquiring the snapshot. Test backing array 66,240 vs 38,400 B. Product still needs 82 rows for knobs, and has not adopted this layout. No static product RAM was reclaimed, no actual target display timing or device write.

Shortest next path: bind actual Clip layout/state/sig once with coherent IRQ publication, preserve throttle/wait/owner invalidation and reuse existing Home/Song physical-strip submitter. Then Clip Keys/Chord/Arp, SMC, remaining live Main pages, MIX scope, engine live controls and overlays, before lowering CV_ROWS. Allocate only one bounded foreground snapshot union at a time, no shadow framebuffer. Keep 80+58 physical split to preserve 16-row tile edge, callback order and unchanged DMA; existing proved Boot 3×80 static, Home and Song remain separate.

For scale: exact raw room replacement needs at least 193,320 B beyond current independent room slices; the 29,480-B quantified but not exact lifting experiment needs 54,760 extra across both. Even full canvas gross recovery is insufficient alone; it must work together with independent history research. No stack/BLE weakening, user RAM, or PSRAM.

Replay: python3 clip-test.py adjacent to the named same-source guard; memory/ELF receipts recorded separately.

The guarded build additionally requires a 512 B floor above the reserve. Only 52 B are spendable in this build before product-wide changes.
