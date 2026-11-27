# Scheduler headroom lab

This is a development checkpoint, not an installer release. Build with `X0X_UI_SMALL_CANVAS=1 X0X_CHICAGO_FOLD_RESIDUAL_MIN=1 MODEL=elite|pro bash build.sh` using the native SDK. It contains the previous exact-mask 909/Chicago worker and whole-808/early-Synth-One main-core split.

At a provably eventless final long span, resume the FX/master pipeline after a short raw first span. Use one logical master call and one GR meter commit. Keep the old deferred path on control/clock boundaries or while the global filter is active/settling. The full host sequencer test with compressor, pump and limiter admitted 128 safe and rejected 128 eventful boundaries; all 529,152 tested PCM, peak, sequence and GR bytes were identical. The master math also passed 32,768 signed block tests and 128 explicitly settled-zero cases.

Motion can skip a second lane scan when it has no relevant current lane. The callback-scoped empty path cannot skip if an active **or parked** lane exists, since a gated sound change can reactivate a parked lane. Passive note/chord storage does not affect the decision. Four existing suites and 16,384 complete passive/empty state comparisons passed; all four parked banks stay on the normal path.

| MIDI offset | Elite worst block | Pro worst block | Late / XRUN (both) |
| ---: | ---: | ---: | ---: |
| 0 ms | 11,181 µs | 11,169 µs | 0 / 0 |
| 2 ms | 11,012 µs | 11,133 µs | 0 / 0 |
| 5 ms | 10,908 µs | 11,315 µs | 0 / 0 |

Strict 360 MHz, same 922-event tape, no skipped cycles, unchanged loaded NOR. Both sides confirm Synth One preset 518 with ARP, delay and Room on and Chicago B. Measured worst 11,315 µs still exceeds 10,809 by 506 µs. These are three tested offsets, not a release guarantee. Elite/Pro app flash free 1,948/1,960 B; shared pool 31,496 B after reserve on both.

Package digests:
```
72a18eea701d4ed659dbff7db0d452281870c2a3a0f3ed75aabf2e7276d1ed4d  x0x-smk37-elite.fwsc
b5f96fd2506143aecfe6316e36c03b7b31e24b490b9e73d8afefa6d18f9903de  x0x-smk37.fwsc
```
