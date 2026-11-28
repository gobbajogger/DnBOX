# Hoist shared semitone ratios

These scratch patches follow the phase, morph and envelope experiments. Apply `semitones.patch` to the dry fixture, or `semitones-wet.patch` where `overlap64.patch` has split the render loop and the existing compact Delay and Room remain active. Keep the same synth control mappings and exact arithmetic. The control values are unsmoothed and block-owned under the guarded build flags; fallback preserves original evaluation for other configurations.

- Elite six dry: 12,140.79 µs versus 12,356.33 µs in the prior combination, 147 XRUNs, 3,828 B app free, 26,492 B pool margin. The 320,000-frame PCM capture is byte-identical to the six-voice baseline. Six voices are not qualified.
- Elite four wet: 11,528.79 µs versus 11,637.21 µs in the prior combination. Zero XRUNs and no skipped emulator cycles in 625 halves, with 324 B app free and 15,228 B pool margin. All 128 held halves remain above the 10,809 µs engineering target; none exceeded the physical deadline. Delay stride 38, Room on. This is a single S1 preset and isolated fixed-MIDI workload, not a system stress test or a parity claim. The source-aware Delay and Room8 archives were not enabled.

No installed or released firmware change. The scratch overlap scheduler is not ready for production even when a test has zero drops: worker failure/ownership and real workloads still need qualification.
