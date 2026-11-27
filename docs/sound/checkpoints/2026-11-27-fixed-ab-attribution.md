# Fixed A+B worker trace

A separate instrumented 922-event run latched the worst half on both models. This is diagnostic instrumentation, not the uninstrumented score. The audio actual timing requirement stays at ≤10809 µs and zero dropouts.

| Worst-half item | Elite | Pro |
|---|---:|---:|
| elapsed | 12435 µs | 12360 µs |
| core 0 jobs | 6647.79 µs | 6632.79 µs |
| core 1 jobs | 8395.29 µs | 8396.67 µs |
| Synth One A, core 1 | 5201.17 µs | 5234.21 µs |
| Chicago B, core 1 | 3194.13 µs | 3162.46 µs |
| core 1 ordered mix wait | 622.38 µs | 639.50 µs |

Each board latched half 392, three stretches, mask 0xC throughout. Processor totals overlap and must not be added. The trace excludes the 808 body prefix, post-mix effects/master, conversion, and other orchestration. Both test images used untouched disposable NOR and confirmed five project-owner readbacks. Trace ELF pins: Elite d2d73c49c2c3885fe378da483c027ac771e8c6294e382734e7d4912f15e54706, Pro a206a2e0251dc169dfc953dbc69f6b340411e1b4d59b35f88266bcf9c9984c40.

Independent experiments against the exact diagnostic donor:
- Moving tom DC filtering to a later pass kept all four buses and whole voice state exact in 224 PI32 cases, but ranged from a 29.57 µs win to a 10.72 µs loss per 256 frames of three toms. Not adopted. The worst half is worker-limited, not just tom-limited.
- Scaling peak meter maximum once is exact over 200000 host blocks and 256 isolated PI32 cases. 17.56 to 13.34 µs per 256 stereo frames in isolation; not an integrated gain or a primary remedy.
- Synth One descriptor specialization removes 228 B per H16 owner and produces exact stereo PCM and complete FX state over 263168 frames, but the largest member still sets the shared union size. Credit zero global RAM until a full link says otherwise. Not an audio/fit gate pass.

SuperJ6 remains a separate task. Source-order cutoff control reproduced 936 exact host oracle endpoints at 32/44.1/48/96 kHz and five real note/edit/retrigger commits. The zero-feedback model fails with nonzero live resonance: actual source uses a nonlinear feedback path, and re-entering the linear path with stale histories fails until it settles. Don't silently switch or reset history when Resonance reaches zero. Native coefficient fidelity and full-series matches remain unverified. No release changes.
