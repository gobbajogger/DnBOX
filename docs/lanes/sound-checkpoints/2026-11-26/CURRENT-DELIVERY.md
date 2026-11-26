# Current DnBOX sound and capacity handoff

No full release pass. Continue SuperJ6 sound matching, exact Synth One optimization,
and whole-firmware capacity work independently. Do not turn a source experiment,
isolated kernel timing, or partial link into a release claim.

| Lane | Verified result | Open gate |
| --- | --- | --- |
| SuperJ6 whole signal | 42 genuine Mac AUv3 waveform/level captures plus a calibrated Saw range extension. All four waveforms support a common ~7 dB difference before compressor versus scoped reconstruction; post-compressor estimate agrees with original master. | Identify the upstream gain owner and prove same-note and polyphonic levels across original and compact full processing. No fitted master boost. |
| SuperJ6 shape | Current original enables two embedded banks only for Sine/Triangle. Real source replay restores H2/H3 within .03 dB and explains the Sine quarter-cycle phase change. Saw/PWM unaffected. | Compact independent waveform model improved the original-tone match but fails held-out signed Triangle and upper Sine cases. Verify source clock, filtering and bend/retrigger; no asset copying or broad preset adjustment. |
| SuperJ6 Normal/OTA | Exact current filter generator given the same operands; strict waveform and routing witnesses, mode commits after a true fade-to-zero, history reset and fade-up. Steady path avoids extra unity work; lean paired tests one-note +36 µs, six-note −165 µs vs stock. | Actual complete public cutoff and resonance boundaries, memory/flash in real build, whole-scene timing and full audio. Lean guard has 516 B after reserve, only 4 B above the 512 B floor. |
| Synth One sound/CPU | Source-matched host reference is the baseline. Exact shared cubic geometry at current reference tap preserves all 906 renders, plus active arp/live delay controls. Isolated target leaf 61–62% faster and 8 B smaller, no history savings. | This reference win is not portable to the native 3-line H16 compact delay. Its separate pair-guard variant was rejected: +17.5% fallback and +368 B reachable text. No combined sound/fit/zero-dropout pass. |
| Firmware capacity | Exact cold code changes returned +1,300 B per board, including verified 16 B codec refactor. Guarded combined strip still 100/92 B past APP limit (Elite/Pro). | No final link or extra canvas RAM. Last live/SMC routes still hold 138 rows. SMC cadence witness rejects naïve reuse of the current strip driver; index canvas lacks a safe global reachable color bound. |
| Dropout acceptance | Verified no real-device writes; exact known timing evidence and reject logs preserved. | Pinned strict runner cannot require all voices verified before scene time-zero and record producer-frame event consumption; compared native runs sometimes start at different sample frames. Must not call weak host/timestamp evidence a gate pass. |

## Checkpoints with reproduction receipts

- [Genuine four-wave AU calibration](sj6-auv3-wave-calibration/README.md),
  [cutoff/harmonic location](sj6-auv3-pre-normal-harmonics/README.md),
  and [current Normal/control routing](sj6-current-normal-routing.md).
- [Paired filter fast-path receipts](sj6-normal-steady-fast/README.md),
  [low-cut caution](sj6-normal-low-corner/LOW-CAUTION.md).
- [Synth One exact cubic source](s1-pair-cubic-f32/README.md),
  [native fold lookup](s1-morph-native-proof/fold-leaf/README.md),
  [wet fit plan](s1-fit-options.md) and
  [dense history negative evidence](s1-dense-delay-compression/README.md).
- [Exact cold firmware cleanup](firmware-cold-capacity/README.md) and
  [SMC input/transfer cadence](smc-strip-cadence-bound/STATUS.md).
- [Strict runner admission evidence](s1-morph-native-proof/harness-admission/README.md).

Rendered reference PCM belongs in Git and Library. Do not include the original
application binary or its embedded original tables. No PSRAM, no weakened memory
reserves, no arp or controls cuts, and no production device writes. Only release
once the same combined Elite/Pro firmware satisfies the agreed sound, fit, strict
zero-dropout and audit checks.
