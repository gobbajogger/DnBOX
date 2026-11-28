# Six voices with both drum sequencers running

Restore donor `873ec0a44a31e497b995c5329d10d225d4f8cf1d`. Apply the standard Synth One diagnostic patches, in this order: workload, dry-workload, dry-six, dry-optimize, osc-phase, morph-and-env, semitones. Apply `dry-six-drums.patch` last for the loaded arm. The no-drum comparator omits it. Use the same pinned strict 360 MHz Elite emulator, test controller, 8-second duration, generated-sample MIDI and disposable virtual NOR. Run `dry-readback.py RESULTS 6` for both and `loaded-witness.py RESULTS` for the loaded arm.

The loaded fixture uses the actual sequencer to trigger both drum kits, all drum voices every sixteenth. Only the current in-memory test pattern is edited. No playback notes from the sequencer on A; the same six sustained MIDI notes are audited through their normal acceptance path. Synth FX, BREAK and the second synth are not active. This is a scoped synthetic drum-load stress case, not the full original system workload or a wet pass.

| Elite dry | Held peak (µs) | Held halves over target | XRUNs | Free app flash | Pool margin |
|---|---:|---:|---:|---:|---:|
| Six voices, no drums | 12,140.79 | 128/128 | 147 | 3,828 B | 26,492 B |
| Six voices, two sequenced drum kits | 15,646.83 | 128/128 | 152 | 3,572 B | 26,492 B |

Zero skipped cycles in both. The loaded arm recorded 132 actual hits per kit during the held window; validated all six note ons and offs. XRUNs are full-run counters, not counts of slow held blocks. Neither case passes. This patch must not be installed or applied to a user project.
