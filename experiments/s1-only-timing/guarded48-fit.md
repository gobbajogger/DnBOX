# Lossless 48-row diagnostic with the large legacy fallback removed

This is a diagnostic, not a release. No production, device, SuperJ,
project format or installer changes. Same six actual Synth One voices
and two real sequenced drum kits; reserve unchanged. Global raster
is 48 × 240 × RGB565 = 23,040 B, previously 33,120 B, a genuine
10,080 B pool win. No canvas storage on user stack, which retains its
original reservation; the host legacy snapshot object is 544 bytes.

## Build composition

Start with the normal 69-row paired ADSR/808 diagnostics (native donor
873ec0a44a31e497b995c5329d10d225d4f8cf1d plus overlays through
f99f3464506bd361262c285cdf312890ee9512a4). Apply in order:
1. boot-slice-48, main-scene-48, clip-keys-48, clip-main-48,
   live-display-48-prereqs, and the corrected live-display-48-ordinary-knob.
2. main-remaining-fallback, cold-app-flash, glide-advance-outline.
3. chicago-qfit, minimax-stock-after-qfit; copy the exact audited
   chicago_fast32_minimax.json to tools/. Stdlib only at firmware build.
4. frozen48-semantic, native-canvas-48-switch, lcd48-no-large-fallback,
   lcd48-build-flags. Set DNBOX_LCD_CANVAS_48=1 and
   DNBOX_LAB_LEGACY_SEMANTIC_RENDER=1 on the original native build recipe.

A native 48 binary cannot call the old 69-row/footer memcpy fallback.
A lost/coherency-raced capture or view change during DMA retains the
previous whole scene and is retried on the next foreground frame.
The useful old 69-row fallback remains when capacity switch is off.

Host: 420 quiet and 420 mutated Chicago/Miami full raster comparisons
(48+21 per logical main band; 48+34 footer) matched the actual earlier
69-row painter. The host firmware scripted routes generated 139 exact
PNGs against the 69-row paired source in a fail-fast 48-row build, unlike
the partial snapshot builder which halted on the first legacy EDIT.
No physical unit or on-target refresh-latency guarantee.

Both loaded native links report audio-pool margin 43,340 B vs 33,260 B;
no stack reserve or BSS region increase versus the joined 69-row source.
Ordinary RAM 95,424 of 98,304 on both. App flash 320 B Elite and 328 B Pro,
well below the 1,024 B release cushion. Fast table packing remains
bit-exact (245,775 C decoder comparisons, same-or-lower per-segment work
vs original). No isolated wet sound or zero-dropout claim.

To repeat emulator use the pinned bench in prior receipt and ordinary
8,000 ms test, then read-loaded-timing.py and read-48-fit.py. The separate
legacy witness removes the scripted heavy workload with
strip-display-smoke-fixture.patch, drives PART pages for both legacy
engines with lcd48-legacy-witness.patch, and reads
read-48-legacy-witness.py after a 3,000 ms strict native run using
disposable virtual NOR. Never install an instrumented witness.

## Loaded strict emulator
| Model | Previous paired 69-row XRUNs | Guarded 48 XRUNs | Held max (48) | Oversized draws |
| --- | ---: | ---: | ---: | ---: |
| Elite | 111 | 108 | 13,124.42 µs | 0 |
| Pro | 108 | 106 | 13,105.04 µs | 0 |

The same actual input fixture (six voices, 132 hits on each kit)
ran with zero skipped emulator cycles. Musical audio is still over
the 11.61 ms physical deadline. It has not passed the 10.809 ms
engineering target, zero-dropout, full product, or hardware gates.

Native legacy witness on both models observed nine successfully drawn
Chicago frames and ten Miami frames across stepped control pages, with
zero oversized requests and no skipped cycles. This is path coverage
for the actual device-class code. Pixel-exact comparison came from
the separate host painters, not from this short scripted witness.
