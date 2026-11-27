# 2026-11-27: loaded timing and SuperJ6 cutoff

## Synth One: native emulator, fixed A+B worker diagnostic

Both models link with both compact-engine configurations and the matched Widen behavior under this diagnostic setup. The firmware branch is not modified by this result. The guarded prototype uses projected tables and init-only presets, not a production storage layout.

| Board | XIP free | Raw audio pool | Worst render | XRUNs | Late | Skipped |
|---|---:|---:|---:|---:|---:|---:|
| Elite | 944 B | 8756 B | 12155 µs | 7 | 7 | 0 |
| Pro | 952 B | 8756 B | 12351 µs | 8 | 8 | 0 |

These are 922-event loaded 360 MHz emulator runs, 319488 rendered frames, five confirmed load-owner readbacks, unchanged disposable flash images. Forced mask 12 is the terminal mask on both (Synth One A and Chicago B on core 1), and each observed worker stack watermark is 3384/4352 B. The fixed planner is for attribution, not a shipping selection policy; XIP benefit includes removal of the generic planner from the diagnostic binary. Strict requirement: <=10809 µs and no dropouts. Both fail.

ELFs: Elite c73558aec7ae04f601c2a02901087b983ce89e20dceb0c91be9b3e938ccc8296; Pro 9a64c4657660c389cef8d5f6d19e355bc2432b7fc37c25fbf5628f5e3e717ec4.

## SuperJ6: live cutoff, guarded zero-feedback leaf

The real source filter accumulates a binary32 coefficient step for 16 samples and uses the exact target on the last tick. The same continuous 52 B four-pole state is preserved during the control change. On two actual source control commands, all 32 predicted coefficient words matched. First 128 samples: downward -131.23 dBr (frozen initial -46.29 dBr); upward -136.71 dBr (frozen initial -49.21 dBr). No native runtime timing or full effect/voice match is claimed. Reports SHA256: downward 1199f20a72fde0e338ce38cc2428df83a236d517945e0cf785344a79f2b6a703; upward 0046582813eaee672d0989f3b3815c544c9aeb83317a317244eeee0a54cc6866.

Do not restore master gain as a candidate. Isolated 48 kHz Raw proves different held discrepancies on Main and Sub, already before a shared gain could explain them. The fixed factor pair that helped Raw did not transfer to Brainwave. Next: derive cutoff endpoints from reached source math, independently trace the loaded source routing, remove the remaining deadline misses and requalify both boards. Neither lane is release-ready.
