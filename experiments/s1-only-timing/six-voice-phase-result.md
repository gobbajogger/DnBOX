# Six-voice dry oscillator phase cache

Apply `osc-phase.patch` after the dry workload, optimization and six-note patches in the disposable native Synth One fixture. It caches the same cycles-per-sample values for BLEP and Q32 steps for the oscillators and FM modulator at the existing eight-sample control refresh. It does not cache the sample-dependent FM carrier. Voice order and preset controls are unchanged. This is not enabled in the firmware tree.

Pinned Elite emulator, 360 MHz, strict no-skip, generated-sample workload, 8 seconds:

| Variant | Held peak | Held halves over 10,809 µs | XRUNs | Free app flash | Pool margin |
|---|---:|---:|---:|---:|---:|
| Optimized six-voice dry baseline | 14,400.92 µs | 128/128 | 147 | 4,148 B | 26,492 B |
| + control-rate phase cache | 12,813.92 µs | 128/128 | 147 | 4,100 B | 26,492 B |

Both generated 478 measured halves, without skipped emulator cycles. Captured signed 16-bit PCM was byte-for-byte identical, SHA-256 `5f0a4e8ed68e2c3d3ccd79574a816f68c07dc523db7e1a27e2bd1d7527f5448e`. The PCM is an equivalence check for this underrunning fixture, not a sound-parity or timing pass. The candidate remains 2,004.92 µs over the project held-block target. Pro and wet timing are not qualified by this result.
