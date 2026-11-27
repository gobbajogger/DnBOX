# DnBOX Widen Hot: exact app space and strict loaded optimization checks

All paired figures refer to actual PI32 Elite/Pro never-install diagnostic builds with guarded companions. Reference Widen Hot reserves audio pool 8,756 B, plus 52 B over the required 8,704 B. Raw pool and fixed XPOOL boundaries are unchanged for all rows. No original samples, firmware images, device writes, or release action.

| Arm | Elite/Pro APP free | Worst loaded µs Elite / Pro | Late/XRUN Elite / Pro |
|---|---:|---:|---:|
| Original Widen Hot source diagnostics | 24 / 36 B | 15,224 / 14,196 | 50 / 85 |
| Exact immutable table sharing/packing | 272 / 280 B | 15,141 / not run | 136 / not run |
| Tables + early S1 render and integer arp span | 32 / 40 B | 17,427 / 17,909 | 131 / 131 |
| Tables + integer arp span, no early render | 160 / 168 B | 15,011 / 14,922 | 41 / 41 |
| Tables + integer arp span + duplicate UI fill removed | 192 / 200 B | 15,258 / 15,128 | 40 / 37 |

The current release target is **≤10,809 µs, zero late/XRUN and zero skips**. None pass. These are strict loaded 922 scene results with identical scripted input; do not interpret changes as deterministic paired causality without exact applied-event correspondence. The last three paired arms confirmed five settled S1 A/arp and Chicago B readback receipts on each model, 319,488 recorded frames and zero skipped cycles. Disposable NOR is immutable across each of those tests.

- Table pack is exact across 70 dictionary words, 150 range pairs and all three shared control/IMA lookup arrays; 248/244 B recovered on actual links.
- Integer arp scope has 125,934,604 actual-scheduler-model comparison frames and 1,726,638 timed note callbacks matching state and render-span boundaries, including changed controls and gate events. Isolated strict PI32 256-frame no-edge work improved 240.96→0.48 µs; first boundary/first note 261–262→2.76–2.82 µs. This is a leaf, not deadline credit. The separate exact binary64 source clock was not changed.
- Bass PART redundant fill removal preserved 38,804,311 bytes of actual host renderer output and exact mock DMA windows across both palettes, four engine selections and both owners. Frees 32 APP B on actual links and avoids 52,800 duplicate RGB565 stores per ordinary bass PART frame. No independent native UI timing attribution claimed.
- The table-only Elite PC profile points to C0 Synth One render, compressor and 909 DSP. The 60.99% unresolved C1 time is **all protected stock-backed addresses**; it is not evidence of free scheduling slack. Do not add the two cores' occupancy or use it to assign the worst half. Need shared-timebase producer/join/owner trace.
- Separate fractional native-clock batching passes the math oracle (~95.6M batched frames for whole-span version) but overflows APP: 124/116 B at ld, plus the required 4-B pack footer. Both 808 accessor variants failed to recover space. Not adopted.

Build pins for the best observed drop-count arm: Elite `70d0a87a7322c6ea1cabd66dc83e7a2b5530e3b425bf87e48c6fe07481dcd450`, Pro `b7f8f89383172217b31df24125f932e64aeb9239eebd812580c0802d54cccf20`. The test still has dropouts and is not releasable.
