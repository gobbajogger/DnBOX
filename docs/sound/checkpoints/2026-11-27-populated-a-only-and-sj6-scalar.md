# Populated Synth One core assignment

Same 922+six-note-edge input and owner-query tape, isolated guard and exact silent Sub shortcut, with actual on-device voice processing counters in stopped emulator RAM.

| Fixed assignment | Elite worst/XRUN | Pro worst/XRUN |
|---|---:|---:|
| Core 1 A+B | 17620 µs / 175 | 17658 µs / 175 |
| Core 1 A only | 15180 µs / 186 | 15236 µs / 179 |

The A-only variant physically links with 960/968 B available XIP and 8756 B raw audio pool; terminal worker mask=4, stack high-water 1972/4352 B on both. Active voice samples 318328/319864, each eligible for zero-effective-Sub skip in this one project. Both runs returned five confirmed ownership reads and no physical or virtual flash writes. Best observed worst block improves, dropout count does not, and applied note timing differs across binary layouts. Neither is a release or even a strict timing gate pass. Both remain >4.3 ms over 10809 µs. Testing source changes against the unvoiced 922 fixture alone is not sufficient.

## SuperJ6 nonlinear high feedback

Regular firmware fastmath is within −111.20 dBr residual over the target-math cold-high host cue, not a whole-preset claim. Exact zero-feedback specialization retained a continuous 52 B state and the complete baseline PCM in approved scoped captures; strict PI32 −40.35% at sustained zero, −29.79% on a warm high excursion, small overhead at all-positive resonance. +334 B isolated text. Do not generalize to the whole bank or sustained high resonance.

A scalar unroll of the positive-k solver matched the compact baseline bit-for-bit across all 90440 host words including warm and cold source paths. Strict single-voice 512-frame isolated timing: k3.15 2423.88→1988.36 µs; k4.12 2932.49→2532.65; warm excursion 1718.32→1681.57. +822 B isolated text and k0 +6.7%. Sharing the tanh body costs only +146 B relative to the generic compact baseline, but gives up most of the speed and slows the mixed warm trajectory. Neither fits whole firmware execution. The current exact approximation benchmark is available to future workers without reusing the original assets.

Main/Sub level and release are still distinct from the filter source fix, and verified receiver note-off routing and stem additivity rule out a hidden release parameter or major mute interaction as the cause. No blanket master or per-preset gain workaround.
