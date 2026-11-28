# Synth One qualification boundaries and rejected follow-ups

Parent: overlap experiment 662013f8, unmodified production and installer. This folder holds replay evidence, diagnostic-only code, and negative findings. Do not apply rejected patches to a release.

## Solo, strict 360 MHz emulation

Using the exact same lazy builds as the parent (no drum hits or transport, six actual held B roots), both models complete all 391 halves at the 5.3 s cutoff, no late parity and no long stale audio. Pro 10,755 µs; Elite 10,807 µs. The Elite run is only 2 µs inside the 10,809 µs engineering threshold. This does not qualify more scenes or arp/event traffic. Reproduce with parent build-project.sh (SYNTH_OWNER=B, default SYNTH_KIT=-1), model-matched lazy package and seed, and parent replay-solo.sh with channel-3 MIDI.

With kits unmuted, the previous Pro heavy case (16,047 µs and 49 long stale PCM spans) is reproducible, including complete raw WAV payload hash. Neither kit in isolation passes with the parent's restricted overlap guard: 909 alone peaks at 16,941 µs/47 stale spans; 808 alone peaks at 16,406 µs/56. These plan masks differ from the both-kits run. There is no new Nightly artifact.

## Heavy Pro experiments, each compared to matching exact lazy control

| Trial | Peak µs | Late parity / stale spans | Decision |
|---|---:|---:|---|
| Exact unchanged lazy repeat | 16047 | 50 / 49 | Reference |
| Ladder leaf relocated to guarded SRAM, standard indirect jump | 16117 | 50 / 49 | Raw PCM hash equal; reject, +70 µs |
| Shared unchanged ladder coefficients, exact guarded path | 16664 | 50 / 49 | Host proof exact; reject |
| 2+4 voice strip (instead of 1+5), no other change | 16204 | 50 / 49 | Reject |
| 3+3 voice strip, no other change | 16744 | 50 / 49 | Reject |
| 808 cymbal reversible staged frontend | 16081 | 50 / 49 | Host proof exact; reject |
| 808 oscillator-bank pending-wrap hoist | 16161 | 50 / 49 | Host proof exact; reject |
| 909 ADPCM cursor held in locals, stores removed in native | 16113 | 50 / 49 | Host proof exact; reject |
| Inserts stereo compressor quarter-rate dispatch sharing | 16394 | 50 / 49 | Host proof exact; reject |
| Inserts exact LFO rate and smoother complement hoist | 16159 | 50 / 49 | Host proof exact; reject |

For the strip comparison set DNBOX_S1_SPLIT_START to 2 or 3 on parent build-overlap.sh in lazy mode (leave tile length 112). For the shared ladder comparison add DNBOX_S1_LADDER_SHARED=1 after applying its standalone patch to the same lazy assembly.

## Architecture evidence

- Per-subspan trace on lazy Pro, worst ordinal 305, 16,178 µs diagnostic (different instrumentation from uninstrumented 16,047). Subspans 8/248/256. For long spans, core 1 publishes 808 at ~4.76/4.82 ms and only then enters the upper voice tile at frames 152/160; each long span has just 96 worker frames (one tile, below existing capacity 112). ~1.45 ms useful worker and ~337/331 µs consumer spin on respective long spans, joins ~0.21 µs. No faults. Bigger tiles and removing all polling cannot recover the several milliseconds missing. Rough total timed work ~25.3 core-ms including some instrumentation/nesting, so merely balancing the cores would still exceed the deadline.
- 909 hot PCs were mapped to the actual selected tom code with a line-table build's nonrelocated text matching. Oscillator, filter, envelope, shaping all material; no single branch/store dominates. An isolated smaller hot path has repeatedly failed to change cadence.
- The display codec is a foreground/UI cost (about 7.4 ms pack + 1.69 ms unpack excluding observed nested audio), not a demonstrated audio-DMA blocker. Audio actually preempted the codec 109 times. The worst load block occurred outside codec, and late-count was unchanged by instrumentation. Do not count foreground PC samples as audio wait.
- On both boards, independently composed cold outline and storage read deduplication add 96 B app flash with no pooled-RAM change and exact storage tests. This is a retained headroom patch, not an audio fix or part of the current binary.

The next productive direction is structural removal of actual whole-render cost, along with earlier overlapping where it demonstrably exists. Preserve audio event/subspan boundaries; compact effects use local control-rate clocks. Do not shave the safety reserve, relax the deadline, cut controls/effects, or infer success from a solo receipt.

## One-kit overlap qualification

Under the parent lazy recipe, apply single-kit-overlap.patch, add DNBOX_S1_SINGLE_KIT_OVERLAP=1, and retain the strict emulator replay inputs. For either single-kit fixture B's private render can start while the kit owns the peer, still with the usual ordered mix. The 909-only prefetch checks the no-808-split case; 808-only begins B at A's existing wait; arp and stale-owner fallback remain guarded. This is still diagnostic.

| Pro scene, six actual B voices | Parent peak / stale | Single-kit overlap peak / stale |
|---|---:|---:|
| 909-only | 16,941 µs / 47 | 13,709 µs / 30 |
| 808-only | 16,406 µs / 56 | 13,606 µs / 35 |

This is substantial overlap recovery in the individual scenes, not zero dropouts. Keep as source evidence, not an installer promotion.

Same modified binary with both kits: 16,264 µs / 49 stale spans (versus 16,047 / 49), so the two-kit test still favors the parent. Pro candidate fit: 4,868 B app flash and 27,392 B pool above the unchanged reserve.
