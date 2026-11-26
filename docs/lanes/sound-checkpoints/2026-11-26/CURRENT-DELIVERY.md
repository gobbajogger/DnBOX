# Current DnBOX sound and capacity handoff

No full release pass. Continue SuperJ6 sound matching, exact Synth One optimization,
and whole-firmware capacity work independently. Do not turn a source experiment,
isolated kernel timing, or partial link into a release claim.

## Latest measured results

- **Raw J106 chorus, same input:** the [original J60V2 leaf replay](sj6-j60-same-input-cause/README.md)
  on the identical compact pre-chorus signal closes the wet center chord
  gap from 3.67 dB to 0.15 dB, and the tail gap from 6.20 dB to 1.08 dB.
  Some held side metrics worsen. This is direct evidence to prioritize
  J60V2, not a gain/width tweak or a whole-engine pass. The affordable
  44.1 kHz implementation and strict timing are still open.

- SuperJ6: the [Sine phase witness](sj6-solidify-sine-phase/README.md)
  improves Solidify's signed opening and hold without gain changes.
  Centered dry first attack +2.887 → −2.116 dBr, hold +3.039 → −1.442.
  The late dry tail regressed +0.344 → +3.331 dBr, so do not enable this
  as a full fix. Closed hypotheses:
  [Sine gain alone](sj6-sine-gain-rejection/README.md),
  [amp envelope alone](sj6-solidify-native-amp/README.md) and
  [slow chord glide](sj6-solidify-chord-ownership/CHORD.md).
- SuperJ6 J60V2: [generated clock, filter grid and 48 kHz leaf](sj6-j60v2-clock/README.md)
  plus true host effects comparisons. The exact current instrument stages
  use a 96 kHz middle kernel; 48 kHz specialization is a host candidate,
  not a replacement for a bounded 44.1 kHz path. No native timing claim.
- Combined warm diagnostic fits Elite +252 / Pro +260 B app, with +564 B
  pool surplus above raw reserve, but measured 11,976 µs/1 late and
  14,509 µs/70 late in the unchanged external stress tape. The image has
  legacy diagnostic drum injections beyond that tape and cannot qualify.
  Pro's measured Core 0 wait for Synth One at the A-before-B barrier was
  only 8.625 µs. Do not claim release, and do not repeat the same probe.
- Synth One: the [typed 6,784 B per-owner delay lease](s1-guarded-owner-delay-lease/README.md)
  now links Elite and Pro with +364/+372 B APP free and unchanged
  +564 B pool above reserve. Disabled-use comparison preserves four S1
  renders and 146,864 stereo SuperJ6 Unison frames bit for bit. Do not
  spend the lease on the rejected H16 rate-only change. Each lease lives
  only while its slot is Synth One; hold all reserves and test timing.
- SuperJ6: source-verified [OTA fader order](sj6-ota-fader-order/README.md)
  fixes an isolated control law, not the whole shared Sub waveform.
  [Installed Crusher dispatch](sj6-crusher-source-mapped/README.md)
  identifies #108 as Amount, #109 as metadata-only Lo-Fi, and hidden
  #110 as Mix. Earlier standalone tests paired the physical setters
  with the wrong public labels; the optional #109 route is frozen.
  The [zero-wet Apple leaf](sj6-apple-dry-exact-tuple/README.md) is now
  supported directly at the tested 0 Decimation / 100% internal mix
  tuple, but it does not establish the installed graph or fit.

## Earlier checkpoints

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
