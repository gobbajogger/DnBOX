# Populated firmware: 808 body scheduling

Reusing the compatible original 808 unsplit render on C0 allows S1 C1 to start sooner when the diagnostic is S1 A + Chicago B. Across 9216 blocks, 1085372 frames, 16005 simultaneous/randomized triggers and 26 block lengths, the actual masked render and separately staged BODY/metal/mid implementations match signed audio on all four buses and every byte of voice state. Both serial call orders and actual pthread concurrency were exercised; target worker recovery is separate.

| Board | Regular 808 body on C1 worst / XRUNs | C1 S1 priority worst / XRUNs |
|---|---:|---:|
| Elite | 14917 / 74 | 14429 / 72 |
| Pro | 14729 / 70 | 14547 / 73 |

Only the initialized diagnostic priority enable byte differs between emitted ELF copies (same code/layout and configured project). Enabled moved 493/491 eligible stretches. Both scopes retain the actual S1 voice admission, signed playback and saved 922+explicit voice tape. Native results are still well over the 10809 us/zero limit. Peak improved on both tested boards but dropout count mixed; no product enable and no claim of deterministic applied event alignment.

## SuperJ6 sound

Two real independent Main/Sub history adapters now run the Raw S4 exact 148684-frame warm cue, with typed controls converted using the reached original source-order mapping, not direct normalized k. Both routers independently match all 90440 predicted g/k words in the scoped same-input event tape. The per-source original post-OTA comparison residual is −115.33 dBr for Main and −111.39 dBr for Sub. This is not full oscillator or installed mixed-output parity. Independent Sub storage requires 312 B per six-voice part before flags or control-ramp state. Both target builds regained 648 B audio pool and passed a populated integrity replay of the old audio path; active filter fit/CPU is not yet claimed.

Fastmath g/k preserves the 3072 coarse reference grid high-mode classifications but flips 159/15360 adjacent-F32 boundary stress cases. Even when k selects correctly, a one-ULP g difference changes the high-resonance waveform: cold whole residual −101.75→−64.29 dBr, warm −111.75→−90.81. Host exact g on high restores the demonstrated fixed-cutoff cases. Do not adopt target fastmath g as-is. Root bounded no-transcendental series correctly matched k on 1 million finite host comparisons but still differed in g on 540, including 37/66561 high-state cases. Standalone target object still requires binary64 support helpers, and its timing isn't established. A bounded search is not correctly-rounded source libm for all controls.

Brainwave hydration: authenticated installed state rewrites stored #130 and #184 to the live linked resonance value. An incorrect raw serialized #184 injection moves held/release roughly +0.9 dB; prior warm A/B already used correctly hydrated values. Treat as a direct import hazard, not the source of the remaining match gap. Do not retune any individual patch or master/role gains.
