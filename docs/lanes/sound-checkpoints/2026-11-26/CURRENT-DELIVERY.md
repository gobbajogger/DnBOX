# Current DnBOX sound and capacity handoff

No full release pass. Continue SuperJ6 sound matching, exact Synth One optimization,
and whole-firmware capacity work independently. Do not turn a source experiment,
isolated kernel timing, or partial link into a release claim.

## Latest measured results

- **Downloadable preset transaction on host:** the [actual immutable
  DNL2 reader](dnl2-immutable-staging/README.md) validates all 1,546
  presets and 221,013 ordered exact parameter words, never claims
  adoption on staging alone, and rejects 200 malformed cases.
  The [real Synth One owner transaction](s1-host-bank-adoption/README.md)
  then applies and renders all 906 on host. All 14 legacy holdouts
  hash-match, and invalid dispatch preserves the prior owner.
  SuperJ6 dispatcher, mapped storage, native pool/APP and playback
  remain open; this is not a firmware bank import or full audio match.
  The [exact SuperJ6 corpus routing map](sj6-bank-dispatch-map/README.md)
  counts 76,120 values on visible typed editor routes, 6,400 covered
  only by host projection, and 8,029 without an authenticated factory
  receiver. No complete preset is currently admissible. Do not remap
  conflicting filter aliases or silently drop pad/limiter fields.
- **Authentic stage improvements are not sufficient:** the exact
  [Main pre-OTA carrier](sj6-warm-authentic-main/README.md) improves
  Raw dry and first release with the authentic Sub unchanged, but
  worsens the complete effects-on result. Separate Sub OTA
  [independent-state candidate](sj6-warm-ota-split/result.json)
  barely moves measurements, so do not spend its 96 B or widen scope.

- **Fresh played-through screen did not qualify.** [Full composed Raw
  original A/B](sj6-warm-authored-terminal/README.md) checks all 170
  public control readbacks and exact MIDI/callback schedule. The guarded
  J60 fixes improve whole signed error only from −15.907 to −15.912 dBFS;
  authored level remains 27.89 dB low and dry remains byte-identical,
  29.76 dB low. [Separate guarded centered source arithmetic](sj6-warm-center-source-trial/README.md)
  recovers 21.6 dB without master edits, but worsens signed authored
  whole error (+0.017 → +1.010 dBr) and held/chord, so is not a
  release fix. Its narrow no-clipping case does not cover dual-owner
  aggregate headroom or old FX history. It stays off.
  [Actual original Sub carrier in RAM only](sj6-warm-sub-carrier/README.md)
  improves the centered authored whole from +1.010 to −0.647 dBr
  but regresses first release from −1.191 to +1.634 dBr. Dry first
  release also regresses. No authorized original audio data was
  exported; no preset change or candidate enable.
  [Authentic Sub retirement](sj6-sub-authentic-retirement/README.md)
  leaves the cursor continuous and naturally ends 39 frames earlier
  than the compact cue. The source AHDSHR law reproduces 560 release
  words exactly, but [Sub-only application](sj6-warm-sub-amp/README.md)
  barely changes whole signed error and regresses the final wet release.
  Do not hard-code a 39-frame trim or enable a different envelope.
- **Fresh paired emulator diagnostic did not qualify.**
  [Exact warm J60 linked pair](strict-warm-j60-timing/README.md):
  Elite 14,269 µs / 63 late, Pro 14,322 µs / 61 late. Same external
  tape and project bytes, no flash writes. This test retains embedded
  legacy hits and does not establish a verified active SuperJ6 owner,
  leading owner state, or producer-frame anchoring. Do not infer the
  J60 changes caused the observed timing differences.

- **Matched combined memory:** on the guarded warm donor with both engines,
  exact shared tables and the unused Synth One lease layout, the
  [reached J60 coefficient pair](warm-j60-lease-tables-fit/README.md)
  still leaves **632 B Elite / 644 B Pro APP** and 564 B above the
  protected pool reserve. No native timing or production qualification.
- **J60 right-hand delay depth:** [the actual original tap](sj6-j60-right-stretch-witness/README.md)
  applies the left correction factor only on the left; right uses 1.0.
  At identical input and with corrected post filtering, the right-only
  change improves mode-I signed residual from −19.97 to −34.34 dBr held,
  and from −18.68 to −27.90 dBr on the chord. Independent II/Both held
  out well. [Matched linked pair](sj6-j60-right-stretch-native/README.md)
  costs 48 B APP and no pool memory, leaving 584 B Elite / 596 B Pro
  and +564 B above reserve. This does not establish native timing or
  whole-preset parity. The [third-term trial](sj6-raw-j60-right-combined/result.json)
  still leaves the substitute direct bridge overshooting; keep it off.

- **J60V2 post-delay coefficient:** the original wet integrators use
  0.2516540875; compact reused 0.533294672 from the *input* biquad.
  The isolated correction improves all five same-input 96 kHz windows
  (the final tail −13.09 → −20.79 dBr), also improves modes II and Both,
  and leaves bypass exact. [Matched guarded native links](sj6-j60-wet-native/README.md)
  show **zero** APP or pool growth, one reached F32 literal change, and
  bit-identical flag-off Elite/Pro images. No production enable, native
  playback timing, or whole-preset pass. A separate
  [actual reached host counter](sj6-j60-composed-clock/README.md)
  establishes nominal 96 kHz wet updates under 44.1 kHz Raw playback
  (323,665 updates over 148,684 frames on both coefficient arms, zero
  in exact bypass, versus 323,664 on the old original). The 44.1 kHz
  bridge quality and release gates remain; the [43-tap combined
  trial](sj6-raw-j60-combined/README.md) still overshoots and stays off.

- **Raw J106 chorus, same input:** the [original J60V2 leaf replay](sj6-j60-same-input-cause/README.md)
  on the identical compact pre-chorus signal closes the wet center chord
  gap from 3.67 dB to 0.15 dB, and the tail gap from 6.20 dB to 1.08 dB.
  Some held side metrics worsen. This is direct evidence to prioritize
  J60V2, not a gain/width tweak or a whole-engine pass. The affordable
  44.1 kHz implementation and strict timing are still open. The original
  direct bridge alone overshoots chord/tail; the [same-input 96 kHz
  middle](sj6-raw-j60-middle/README.md) also differs, so do not adopt
  a [43-tap direct-only substitute](sj6-j60-direct-sparse/README.md).

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
