# Scheduler headroom lab: full 808 and early Synth One A

This branch is a test checkpoint, not an installer release. Build both models with `X0X_UI_SMALL_CANVAS=1 X0X_CHICAGO_FOLD_RESIDUAL_MIN=1 MODEL=elite|pro bash build.sh` and the native JieLi SDK and tools.

The realized second-core plan at every original measured Elite miss put the 909, 808 and Chicago B on Core 1. When the scheduler selects that exact plan and the applied A/B owners are ready Synth One and Chicago, retain the 909 and Chicago on Core 1, put full 808 on Core 0, and render Synth One A before the ordered mix wait. The original ordered mix and ownership are retained. Do not confuse the actual ready owner with whether the voice is currently active; tails and upcoming arp events matter.

| Model | Worst audio block | Late / XRUN | Half callbacks | App flash free |
| --- | ---: | ---: | ---: | ---: |
| Elite | 11,412 µs | 0 / 0 | 619 | 2,524 B |
| Pro | 11,412 µs | 0 / 0 | 619 | 2,536 B |

Exact uninstrumented 922-event strict 360 MHz replay; the same Synth One preset 518 on A, Chicago on B, arp on, delay mix ~0.3625 at 2/3 s and Room at 0.5 were checked at boot and the end. No skipped cycles and no simulated NOR changes in the loaded runs. Same-replay host A/B with 309 actually remapped spans matched 495,120 output and observed state bytes. Both native models still miss the 10,809 µs target by 603 µs; this is not a release pass or an all-scenario guarantee. After the fixed pool reserve, 31,496 B remains in both builds.

Expected package SHA-256:
- Elite: 2c397dd77a48ef01148e435b82553779136d669c23470e98dc80c5ae831fa49a
- Pro: b758fe6cb7f0b153acd3a0114f80c104790ce2c74257ecb16f96f871e420844a
