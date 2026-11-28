# Delay input prefilter sharing: exact DSP, rejected native timing

Diagnostic only. Pinned app e00f689d with the full-cycle sine lazy assembly; no changes to production, Nightly, SuperJ6 or the memory reserve.

## Measured Pro comparisons

| Same scene, per pair | Peak µs | Completed halves | Stale spans | Free app flash B |
|---|---:|---:|---:|---:|
| Standard both-kits control | 15,430 | 341 | 49 | 4,372 |
| Per-frame equal delay-input share | 16,133 | 340 | 49 | 4,084 |
| Settled centered whole-span share | 16,249 | 340 | 49 | 4,052 |
| Forced-wet six held with both kits, control | 15,920 | 340 | 49 | 4,260 |
| Forced-wet six held with both kits, per-frame share | 15,977 | 340 | 49 | 4,004 |

No pool change: 27,392 B beyond the 8,192 B reserve. Both native forced-wet arms verified actual delay and reverb enabled at 0.5 mix, 0.30 s delay. The independent traced forced-wet candidate skipped all **512 duplicate calls** on its worst completed half (ordinal 304, spans 8/248/256). This proves that the gain predicted from call count did not translate to the measured deadline; do not infer a whole-block gain from recurrence count or promote either variant. Trace timings are not qualification. Panel-empty solo Pro retains the exact prior PCM hash with 391 completed halves and zero stale, but peak increases from 10,433 to 10,617 µs.

The host checked 694 factory presets plus INIT in each implementation, **8,573,419 exact stereo frames and complete engine and arena state**. INIT was held wet with warm pan/phaser changes and deliberately unequal filter histories including signed zero. The per-frame version compares five history words at ingress, splits at the first unequal post-phaser sample before running both ladders and mirrors the left state on return when they stay equal. The alternate version only uses the span-wide guard when the sequencer cannot mutate controls and pan depth/phaser mix are both zero in current and target values. Neither uses delay partition mono as a proxy. The coefficient cadence, both delay wet paths, and Room stay unchanged.

Reproduce by building the parent sine-work full-cycle control, then applying one patch to clean assembled control source, adding `-DDNBOX_S1_DELAY_PREFILTER_SHARE=1` and rebuilding unchanged Pro flags. Reuse the same Pro controller, seed from the new package, and strict owner B both-kits replay. For the separate wet arm, run `force-wet.py` on both wet sources. For counters only, additionally apply the parent qualification lazy trace then the local counter patch to the forced per-frame arm. Run `read-twin-count.py` on its matched ELF and RAM. Use the paired host script against assembled baseline and candidate roots with ordinary source/pack assets present.

## Structural boundary for a future parallel effects diagnostic

The older serial stage split at f636ac31df2d8d22cc9aedbb6f59b864b94b7f4a proved a useful stage boundary, but it should not be ported as-is. Its tile records individual falling smoothed delay times, then bulk-advances the live smoother. A partition call reading live final smoothed time instead of the current row can allocate different rings after attach or Mono reset. The binary32 model here gives a concrete 2,300 B pool counterexample: frame-zero state chooses decimation 36 and rings 736/384, tile-end smoothed time chooses 33 and 736/368. Pass the correct current row and immutable target snapshots to partition or exclude any reset/repartition in a first experiment. Before moving wet work, prove a fallback that cannot partially mutate the committed delay/Room/widen history; copying the engine struct alone leaves aliased arena pointers.

An 88 B × 512 tape is 45,056 B per block direction; instead an exact 32-frame dry-sum tile is 128 B before metadata when modulation selectors and controls are proven stationary. A two-tile queue only helps if the core is available: older same-source attribution put actual post-voice FX at 23–26% of Synth One's stage ticks, not the higher fixed-stage share that also includes voice-dependent smoothing and LFO. Current trace has roughly 25.3 core-ms before final tail and the late peer is already doing about 2.9 ms of useful upper-voice work. Merely moving FX is not evidence for a 4–5 ms saving. A bounded next comparison would quantify a single phase-stable 32-frame tape after dual-kit retirement *versus* the current upper-voice peer allocation, with no output rounding changes or fixture cuts.
