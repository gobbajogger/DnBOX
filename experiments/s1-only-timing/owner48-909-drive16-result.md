# Fixed-drive 1.6 coordinate trial on the combined 48-row diagnostic

Recreated the exact a54125b6 combined diagnostic, then changed only the 909 kick's fixed-drive 1.6× interpolation coordinate. Since 128 is a power of two and attainable triangle values cannot underflow a product by 1.6, the old rounded (1.6× × triangle) × 128 equals the new rounded (1.6× × 128) × triangle, with the same sign at every attainable input. Other saturators, render order, six Synth One voices, both kits, controls and settings are unchanged.

The exhaustive test covers all 75,497,473 rounded unsigned folded-phase values. Host -O0 and -O2 each matched 23,280 blocks on all four audio buses and all 6,496 B of runtime state. Same-tree Elite native links: baseline 1,392 B free XIP, trial 1,376 B; both 43,340 B free audio pool beyond reserve. Static ordinary RAM also unchanged (95,424 B allocated). Native speaker hardware, installer, and production image were not used.

| Strict Elite | a54125b6 | Kick drive16 trial |
|---|---:|---:|
| Held peak | 12,843.46 µs | 12,807.33 µs |
| Whole-run peak | 15,147 µs | 15,153 µs |
| Held over physical budget | 57/128 | 52/128 |
| Held over engineering target | 128/128 | 128/128 |
| XRUNs | 79 | 76 |
| Processed halves | 545 | 548 |

This is small and mixed: 3 fewer XRUNs and 36.1 µs lower held peak, but 6 µs worse whole peak and 16 B less flash. Do not deem a release or material improvement. Native output fingerprint comparisons for this optional patch are not checked. The unchanged combined arm is a better control for further experiments than compounding a tiny layout-sensitive step.

| Strict Pro, matching controller | a54125b6 | Kick drive16 trial |
|---|---:|---:|
| Held peak | 12,880.67 µs | 12,790.50 µs |
| Whole-run peak | 15,129 µs | 15,122 µs |
| Held over physical budget | 57/128 | 54/128 |
| Held over engineering target | 128/128 | 128/128 |
| XRUNs | 78 | 77 |
| Processed halves | 546 | 547 |
| App flash free | 1,400 B | 1,384 B |
| Audio pool margin beyond reserve | 43,340 B | 43,340 B |

Pro remained at 48 rows / 23,040 B, zero oversized draws and zero skipped emulator cycles. Elite held improved too, but the whole maximum did not. No device write, installer change or push. Keep the patch separate and run native fingerprint parity against the intended composition before retaining it.
