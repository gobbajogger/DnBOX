# Current DnBOX sound and capacity handoff

No full release pass. Continue SuperJ6 sound matching, exact Synth One optimization,
and whole-firmware capacity work independently. Do not turn a source experiment,
isolated kernel timing, or partial link into a release claim.

## Latest measured results

- **Bank tuning state, host witness only.** An
  [identity-bound sidecar fixture](s1-tuning-typed-host/README.md)
  distinguishes absent tuning, explicit 12ET, and real 3-/19-degree
  custom records. All 512 actual owner-frequency checks are within
  .00033 cents of the independent source math. Absent tuning
  inherits the established scale; other records change only the
  detached owner under test. No project state or A4/queue-mute
  storage change. Current DNL2 still drops metadata.
- **Alternative first-cue recorder.** The
  [zero-new-source-byte variant](909-first-cue-reuse/README.md)
  reuses all 16 B of an otherwise diagnostic-only warm marker.
  It binds two drum bus words and a final DMA word to the producer
  and half decisions. It does not bind actual DAC consumption.
  Dry apply and fail-closed parser pass; no link or replay.

- **SuperJ6 internal history storage, not pool recovery.** The
  [offset-preserving gap proof](sj6-stock-gap/GAPS.md) reserves
  264 aligned bytes per exclusive typed/Juno owner without moving
  either protected lease offset. Raw authored/dry signed audio and
  the full ARP/physical-control witness are unchanged. The simple
  shrink alone did not change the native union size because S1
  also owns its full extent. The
  [exact thin OFF/ON paired links](sj6-stock-gap-native/native-gaps-paired.json)
  retain 52 B spendable above reserve, with APP 520→504 Elite
  and 532→516 Pro (16 B less each in the same staging). No new OTA
  history active or reason to deploy the empty gaps. A minimized
  Main-only history is still theoretical, not all source owners.
  The [reduced isolated Main leaf](sj6-ota-history/README.md)
  now genuinely uses 64 B instead of 84. Aliased histories are
  consistent by recurrence, not approximate coefficients: both
  real cutoff tapes (123,515 frames each) and 104,640 synthetic
  lifecycle cases are word-exact. Isolated PI32 text 582→556 B
  originally needed fmaf/tanhf. The [tested exact shared FMA](sj6-ota-shared-fma/SHARED.md)
  eliminates fmaf without altering Chicago, and all original-cue
  and stress output words still match. The 622 B native object
  and at least 688 B of absent tanhf/expm1f code exceed APP
  before extra dependencies. No native clock or OTA admission.
  A separate [existing tanh reuse](sj6-ota-tanh-reuse/README.md)
  yields −154.7/−148.7 dBr relative to the exact leaf on the
  two rendered cutoff tapes, worst stress −143.6 dBr. Its
  promoted-helper object is 622 B (no libm), before native
  wrapper/owner costs. This is sonically promising for the
  observed zero-feedback scope but still fails measured flash
  headroom; nonzero feedback or full-engine parity not proven.

- **SuperJ6 Main physical OTA is now independently reconstructed.**
  The [same-input leaf and full typed Raw controls](sj6-raw-main-ota/README.md)
  prove the original two-substep/all-pass/filter/decimator corrects
  the isolated phase error, reaching roughly −141 dBr residual
  across four keys at original 30 kHz and −126 to −131 dBr at an
  independent lower cutoff. On the whole warm compact Raw the dry
  signed error improves from −1.44 to −1.73 dBr, but wet goes from
  +1.01 to +1.13 dBr with the current compact chorus. The isolated
  PI32 object is 582 B text plus unresolved calls and 84 B per mono
  state; no linked fit or timing proof. Keep as oracle, not adoption.
  [Separate current original J60 control](sj6-main-ota-current-j60/j60control.json)
  uses identical input within each Main arm, authentic stopped
  bypass, and 1,164 original partitions. With the original J60
  the Main-only improvement returns (whole authored −0.924 to
  −1.184 dBr). The first release regresses and absolute output
  is still about 7.4 dB low. Not a deployable combined fix.

- **Raw active-voice root localization.** The
  [same-input native and compact taps](sj6-raw-voice-stage/README.md)
  isolate two distinct discrepancies. Sub held fundamental is
  already −9.52 dB/−115.29° before OTA, and ends +0.32 dB/−133.00°
  after voice processing. Main held is −1.85 dB/+0.13° pre-OTA,
  with relative H2–H8 within .001 dB, but +0.19 dB/−10.24° after it.
  Held individual post-voice RMS differs only +0.06/+0.21 dB.
  Not a master-volume fix. Native Sub cursor/onset and Main voice
  processing need separate source-faithful fixes; terminal parity is open.
  The [authentic Sub cursor](sj6-sub-current-cursor/README.md)
  reaches 123,515 real Raw frames across four keys. An independent
  host leaf reproduces all 8,192 cursor and signed-sample test pairs
  exactly. Independent input tapes with identical clocks produce
  different phase and waveform; clock alone cannot match source
  audio. Exact data from the original would amount to over 7.8 MB
  of uncompressed PCM16 across 12 zones; it is not included. A
  [bounded read-only source adapter](sj6-sub-pool-provider/README.md)
  now replaces just the current source sampler leaf. With the
  separately held decoded RAM source, all 123,515 source reads
  and complete 148,684-frame authored/dry host renders match the
  unmodified source adapter word-for-word; refused reads are atomic.
  This qualifies the data/clock boundary, not a compact media model
  or installed parity. Total conditional authored/dry level is
  still about 13 dB low, and some release windows regress.
- **Whole guarded 909 trial failed.** The new
  [paired replay](drum909-warm-replay/README.md) saved 112 B in
  each native link but did not qualify: 14,278 µs/79 late Elite and
  14,374/66 Pro (frozen old 14,269/63 and 14,322/61). Output first
  differs at frame 65,600 on both. Without producer phase proof,
  these outcomes are not attributable; park the patch. The additive
  exact Q15 scale leaf was too small to change the gate conclusion.
  [Exact first-cue 909 isolation](drum909-onset-proof/README.md)
  compared all 69,216 direct bytes of active four-bus output and
  owner evolution (only relocatable stream pointer words masked).
  Those bytes matched. The archived Pro candidate has 1,024
  extra zero frames at onset, not proof of exactly two DMA
  misses or an external tape issue. The mixed/ALNK boundary
  remains unattributed.
  A [bounded one-shot recorder](909-first-cue-recorder/README.md)
  would add 48 B and bind a first-cue producer to its rendered
  ALNK half. It cannot attest DAC consumption by itself. The native
  attempt did not link (compiler output I/O), so size is estimated.
  A separate [whole-owner overlap](owner909-early-chicago/README.md)
  runs 909 on C1 after the existing 808 prefix and advances bass on
  C0, preserving sum order. The guarded engine TU compiles (+346 B
  named text, +20 B BSS); twelve extracted C cases reject unsafe
  takeover and nine abstract orders preserve barriers. No paired
  target PCM or timing yet. The
  [paired native thin links](owner909-early-chicago-link/NATIVE-THIN.md)
  now fit with +24/+36 B APP, same 52 B spendable pool and
  unchanged XPOOL. Exact same thin OFF baselines are +520/+532,
  so scheduler delta costs 496 B. The only
  [matched isolated Elite replay](909-owner-overlap-elite-replay/README.md)
  records 14,288 µs/86 late OFF and 14,736 µs/64 ON with no
  NOR writes, skipped cycles, or partial owner trap. Changed
  first healthy read time and PCM prevent causal comparison.
  Neither arm passes, no Pro timing, hold. A new
  [four terminal sampler split](owner909-terminal-group/README.md)
  preserves complete 909 owner and four-bus words across 256
  active host spans, including not-ready and stale tokens.
  Its 4,660 B private workspace fits the native 5,120 B A area
  only with a proven active-A barrier. [Host extracted scheduler](owner909-terminal-integration/README.md)
  now verifies A workspace release before a hostile active-A
  consumer, 40 nonwaiting serial decisions, and twelve guard
  faults. It uses actual embedded kernels but an A test stub,
  not a product S1/timing claim. Native relink pending. Separate
  [active host DSP proof](owner909-early-chicago/owner-fixture.log)
  compares 6.81 MB of exact outputs/states over 192 active
  spans, with actual host overlap on 124; it does not cover
  the linked product scheduler, S1 owner, or final master.
  Never combine the two memory
  estimates within the same 52 B of available reserve headroom.

- **Synth One filter-envelope sharing, guarded.** The
  [paired native link and signed-PCM matrix](s1-shared-fenv/README.md)
  recover 48 B APP (+300/+308 B vs +252/+260 B) on this fitted donor.
  The protected pool is unchanged and all 24 focused host PCM cases
  are exact. On the [newer matching J60-right donor](s1-shared-fenv-right/right-readme.md)
  the same single patch recovers 48 B in each paired native link
  (+584/+596 to +632/+644); both entire 906-program signed host
  logs are byte-identical, including 14 legacy and 8 wet rollback
  witnesses. In [isolated qualified PI32 cycles](s1-fenv-native-speed/scope.txt)
  the renders are exact and finite for all 24 tested cases. Four
  active notes save 24–38 µs / 64 frames on routed cases, but a
  single note costs 13–16 µs more. Keep the guard off pending a
  selective variant and actual heavy playback qualification.
  A distinct [bounds table change](s1-control-bounds-only/table-only.md)
  gives +48 B on each paired build with **no FENV optimization**,
  keeping +632/+644 B APP and the existing pool. All 150 bounds
  checks and 906 signed host fingerprints are unchanged. It has
  no DSP hot-path use; no full playback timing test yet.
- **What remains in SuperJ6 Raw.** In the reconstructed pre-chorus
  path with the genuine current J60 leaf, the [relative chorus level
  is within 0.17 dB](sj6-postvoice-split/README.md). The absolute dry
  waveform and level still differ. Inserting the original Unison-off
  and settled AutoPan does not close the gap: together they make the
  absolute output 4.59 dB lower and leave waveform correlation
  effectively unchanged. Continue through the active voice path.

- **New guarded 909 state promotion, not yet full-scene.** The
  [plain-decoder and Huffman traces](drum909-state-promotion/README.md)
  match complete owner state and four buses on 112,000 host spans.
  Eight isolated real PI32 sampler cases stay exact and improve
  2.96–9.34%, 1.66–11.68 µs per 128-frame sampler, with 80 B less
  leaf text/table. This is not whole-firmware size or timing.
- **Measured heavy replay attribution.** In the [instrumented trial](strict-j60-applied-cost-diagnostic/README.md)
  Elite was 14,327 µs / 75 late; Pro was 14,618 µs / 73 late.
  Measured 909 + Chicago account for 71.4% / 70.5% of those
  specific worst blocks respectively. Timer interruptions and recorder
  effects are not separated; these are not causal or release numbers.
- **Raw negative evidence.** Actual current Raw
  [oscillator 2 and noise](sj6-raw-osc2-noise/README.md) are silent in
  their own original leaves, and
  [compressor mute](sj6-raw-comp-muted/README.md) leaves no stale wet
  path. Neither is a plausible remaining level source in this cue.

- **Original Raw post-voice owners, guarded host only.** The actual
  [current OTA cutoff setter](sj6-warm-post-voice-sum/README.md) produces physical 30,000.0078125 Hz for
  this cue. Separately captured Main and Sub state and program
  chronology were joined at the app's proven unity mixer before the
  compact instrument HP. Dry solo summation matches the joined result
  below −143 dBr. Original post-voice substitution improves held and
  complete comparisons, but worsens the first signed release. A
  [separately guarded HP10/zero-Crusher](sj6-warm-post-voice-physical-chain/README.md) before J60 and final DC after
  the unchanged master recovers that first release (authored +1.090
  → −6.048 dBr, dry +0.827 → −5.272), but worsens authored chord,
  final release and whole. No adoption. Source-only archive receipts:
  10,072 B, SHA256 `ad08cba363dd5dfd05a4330722c1d74ea23c95233fe5c42f3598a46870ff05ed`;
  12,011 B, SHA256 `33338b9df37da383b76489a7d67f7857b647303106fe642f4d8a56b8bb162fcd`.
  The [current original J60 leaf](sj6-postvoice-j60/README.md)
  on that exact same upstream PCM/partition sequence improves authored
  full signed error from −0.452 to −3.923 dBr and final release from
  +0.608 to −3.651, with exact dry passthrough. First release regresses
  and absolute authored level remains 8.47 dB low. Full current J60
  contributes materially; it does not explain dry or level remaining.
  Do not infer the complete installed graph or export original audio.

- **SuperJ6 distinct factory controls.** #203/#204 select independent
  [performance XY destinations](sj6-factory-xy-pad-owners/README.md), not sequencer pads. Original setter
  dispatch and Pad Lock restore keep the two fields separate; every
  one of the 640 original presets stores distinct values. Existing AU
  setter/loaded-state proof does not verify XY movement or enable a
  firmware editor. [Reached XY gesture source](sj6-xy-gesture/README.md)
  uses independent contexts and forwards each gesture's last mapped
  values on release in the verified leaf. Do not invent a common
  musical reset or a sequencer mapping. Archive SHA256
  `e448e6f9837bced7de5c55576270d134ea9e15ac50134cc341419f2b047267a9`.
  Original cold order sends #7/#8 before #129/#130, but that is not
  alias precedence. Actual #7 writes cutoff address 36; the distinct
  #129 harmonic cutoff uses address 6. #8 writes resonance on four
  nodes plus a separate compensation mixer. Complete public #130
  dispatch is not yet proven. Keep all fields separate and refuse
  unsupported factory restores.
- **Downloadable Synth One tuning is not carried yet.** The [906-preset
  metadata audit](s1-download-tuning-metadata/proof.txt) finds 16
  real custom tunings, 672 explicit equal temperament and 218 absent.
  Removing tuning leaves every DNL2 bank byte-identical. The existing
  owner-local oscillator can reproduce the known custom scale, but
  standard and absent metadata must never silently retune the project.

- **Downloadable preset transaction on host:** the [actual immutable
  DNL2 reader](dnl2-immutable-staging/README.md) validates all 1,546
  presets and 221,013 ordered exact parameter words, never claims
  adoption on staging alone, and rejects 200 malformed cases.
  The [real Synth One owner transaction](s1-host-bank-adoption/README.md)
  then applies and renders all 906 on host. All 14 legacy holdouts
  hash-match. The [late-field rollback proof](s1-host-bank-late-rollback/README.md)
  rejects a bad final field after staging and preserves held note,
  live wet history and exact active identity. Eight independent
  held/FX cases have 66,048 identical continuation stereo frames,
  and the 14 legacy holdout UIDs are independently unique.
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
  J60 changes caused the observed timing differences. A separate
  [24-byte recorder build](strict-j60-applied-busy-link/README.md)
  preserves the hard margin and matches both flag-off images exactly.
  Its [instrumented diagnostic](strict-j60-applied-busy-diagnostic/README.md)
  finds active 909/808/BREAK/Chicago and completed C1 808 prefix
  in both worst halves. This is lower-bound job occupancy, not timing
  by job; missing endpoint-active Synth One/SuperJ6 bits do not rule
  out work inside a span. The instrumented image cannot qualify timing.

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
