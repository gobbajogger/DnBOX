# DnBOX sound investigation: ruled out, confirmed, and still open

Read this before starting another sound investigation. A result applies
to its named capture, engine and stage. Do not revive a rejected idea
unless the test changes the specific condition that made it fail.
Do not fit individual presets, a master gain, or an arbitrary time shift.

## SuperJ6: ruled out or insufficient

| Do not retest as the root cause | What actually ruled it out |
| --- | --- |
| **Master volume. Leave it unchanged.** | The authenticated current original maps the raw master linearly to `.404 × raw`; the separate late Booster is .85. The matched dry and compressed native outputs agree with the already documented output-scaling evidence. Master changes cannot restore source waveform, phase or missing bandwidth. [Source](superj6/master-boundary/CURRENT-MASTER.md), [forced-state comparison](sj6-crusher-compressor-cross/README.md). |
| Oscillator fader slope or velocity slope alone | In the controlled native cue, relative fader and velocity deviations were within approximately .32/.18 dB of the examined projection. That doesn't rule out an absolute internal scale or all expressive behavior. [Receipts](sj6-jupiter-source-replay/jupiter-gain/README.md). |
| A fixed boost across all presets, or one that only activates on a solo voice | Three authenticated compact projections have different dry deficits (roughly 46, 29 and 38 dB) and substantial spectral residual. This is not a fully mapped compact build, so the numbers don't establish the final firmware deficit. Separately the solo-only gain diagnostically produced a 21.6 dB discontinuity as voice count changed; the original held note did not. |
| Treating the bare Apple Crusher at 100% as the original | #108 is **Amount**, #109 Lo-Fi has no DSP setter on the tested route, #110 Mix is private, and Enable sets actual mix to 30% plus one 1.3× multiplier per channel. Same-setting Apple replay reproduces the original complex response, within .204 dB magnitude across three faders. Don't treat enabled Amount zero as dry or count the Booster twice. [Corrected test](sj6-crusher-source-mapped/README.md). |
| Stale Crusher/compressor startup state, as the remaining inter-stage discrepancy | Forcing #107 and #17 through their physical transitions still gives the detector difference; colder default register state alone does not explain it. Active original and bare compressor share the same ~160.50° phase and identified output scale. Do not attach that conditional result to a specific upstream stage without a witness. [Forced-state comparison](sj6-crusher-compressor-cross/README.md). |
| AutoPan at neutral as an upstream gain booster | The current original active leaf was verified at ~.7071 per channel, with no channel sum or amplification. Presence of the leaf in a given original graph state is still a separate question. |
| Random Vintage detune, post-filter EQ, or final output shaping as the origin of stable Sine/Triangle extra harmonics | Genuine Normal cutoff sweeps place the relative harmonics **before** the filter, with about .001 dB/.004° transfer agreement. Fader sweep preserves harmonic ratios; actual populated factory generator reproduces them, while disabling its specialized bank removes them. [Transfer](sj6-auv3-pre-normal-harmonics/README.md), [bank trace commit](https://github.com/gobbajogger/DnBOX/commit/6620992b). |
| Applying a stock unity voice sum through existing Q15 histories without checking headroom | The compact source divides the layer voice sum by six and multiplies each centered leg by .5. The original single-voice scope supports raw summing, but the host bypass of both reductions can overflow the short Classic history before the master. The history alternatives already tried were too slow on PI32. This is a real compact difference, not proof that it explains every patch. [Fixed history rejection](sj6-classic-fixed-native/README.md), [same-byte alternative, host only](sj6-classic-e4m11-headroom/README.md). |
| Turning up the long-delay wet output to recover treble | The sampled history aliases the carrier at long settings; boosting output cannot reconstruct it. At the ~496 ms cue, a prototype preserving more history bandwidth made the first actual 3 kHz carrier −.71 dB vs the original, but BFP4 round-to-nearest froze sustained tails at max feedback for all 201 measured repeats. The fixed stride also only covers ~.505 s, not the 1.5 s free or 16 s synced endpoint (120 BPM). **Reject the prototype as a replacement.** |

**Long-delay rejection follow-up:** a deterministic error state per
channel (not per cell) does not repair the 4-bit block format. At .99
feedback over 201 echoes, `u=x+e`, nearest `q` using the block step, then
`e=clip(u−q, −step/2, +step/2)` removed exact DC lock but musical periods
still locked. The one-sided 3 kHz RMS stayed .6201 versus .000047 in the
floating reference, and stereo/fuller chord tests also failed to decay.
Do not reopen simple rounding, truncation or one serial error state as a
release fix. The model's fixed ~.505 s coverage independently fails the
full delay range. Independent reproducible stochastic rounding also
removed exact repeated states but remained unusable: the one-sided
3 kHz echo was 0.024–0.026 RMS instead of .000047 at repeat 201, about
54 dBr above the linear reference, and quiet DC noise and near-full
overshoot grew. Do not retry four-bit recursive history simply by
changing the rounding rule.

## SuperJ6: confirmed mechanisms and active tests

- **Oscillator:** source Sine/Triangle factory bank interpolation, signed
  phase advance and oversampling halfband explain a real dry mismatch.
  A packed halfband prototype using the existing history bytes was wired
  into the actual compact typed oscillator (before its envelope/filter).
  With identical constructed suffix both ways, median signed phase across
  blind holds went from −79.45° to ~0° for Sine and +4.14° to ~0° for Triangle;
  all 24 Saw/PWM guards were bit-identical. This is evidence for the
  oscillator repair, not a direct observation of a private in-app stage.
  The packed-float implementation is **rejected on target**: the same
  24-layer-voice simulator leaf took 18,131.54 µs vs 4,019.5 µs for
  stock 2×, before lookup, envelope, filters or arp. It exceeds the whole
  budget. [Paired native rejection](sj6-halfband-native-rejection/replay-elite1.log).
  The 4× Q14 integer variant also measured 17.21 ms in the
  comparable leaf, so switching arithmetic alone did not save this loop.
  The remaining waveform candidate must avoid running this 4×
  float loop, even though this was a useful diagnostic.
  [Donor A/B](sj6-halfband-packed-donor/README.md).
  **Less expensive phase prototype:** Fold64/Q8 keeps the source phase
  clock, uses generated prefiltered bands, and avoids live 4× filter
  histories. Blind median phase error ~.03° Sine/.02° Triangle, with
  better H2–18 agreement and exact Saw/PWM guards. A paired fully typed
  dry sequential stress test changed Sine 12.67→12.78 ms and Triangle
  15.87→12.87 ms (fixed cues); candidate worst 12.91 ms, plus 1,156 B
  application flash and no extra pool. This is useful but does **not**
  fit our combined build or clear its timing. [Leaf](sj6-fold64-q8-leaf/FOLD-RESULT.md),
  [paired dry evidence](sj6-fold64-q8-full-dry/FOLD-RESULT.md).
  **Hoisted Fold32** trades 6.98 dB Triangle relative shape fidelity
  against Fold64 median but is still 18.49 dB better than stock on
  that metric. Hoisting it changes no audio from the unhoisted Fold32.
  Both Sine and Triangle are now faster than stock in the same four
  paired dry tests; slowest candidate 12,115.79 µs sequential, still
  outside 10,909 µs. It adds 900 application bytes and no pool.
  [Exact paired comparison and rejected alternatives](sj6-fold32-hoisted-native/fold-compare.md).
  Already-folded voices can also retain the correct four-quarter phase
  clock across a held switch through Saw/PWM. Verified exact phase at
  four switch boundaries, unchanged cold Saw/PWM, muted output and voice
  steal guards. The new shared end-of-step ownership adds +128 application
  bytes over hoisted Fold32, zero pool, same dry signatures. It is not
  an exact fallback waveform or integrated dropout pass.
  [Warm handoff proof](sj6-fold32-warm-phase-handoff/live/README.md).
  The actual S1+Low+909+cold(+64 B) composition at +832/+844 B APP
  was independently verified. Hoisted Fold32 alone (without the
  separate +128 B warm handoff addition) fails the real strict links
  by 64 B Elite / 56 B Pro, or 68/60 including the final marker.
  Local Fold and Low edits commute byte-for-byte; actual host guards
  passed for all shapes, Low/Center/High, no Sub change, Saw/PWM,
  hold/bend and release. Do not ship or assume arithmetic budgets.
  [Composed evidence](sj6-fold32-guarded-composition/FOLD-COMPOSED.md).
  Exact cold companion serialization moved this to −52/−44 B including
  marker. The next real paired link now **does fit**: the Chicago factory
  name offset checkpoints save enough exact APP (10,001 name validation
  cases) for +72 B Elite/+80 B Pro with +564 pool. This is the exact
  oscillator plus S1 phaser plus Low Sub guarded pair, not the separate
  phase-handoff cost and not sound/zero-dropout qualification.
  [Both-engine fit receipt](guarded-both-engines-fold32-fit/NAMES.md).
  New no-shadow composition brings hoisted Fold32 to +348/+356 APP with
  the same +564 pool. The subsequently measured warm handoff adds 96 B
  on this exact donor, leaving **+252/+260 B APP** marker-inclusive.
  The actual source clocks and held mode/pad/bend guards passed,
  but this is INIT-only, with the native full scene run pending at seal.
  [Exact warm paired link](guarded-warm-fold32-pair/WARM.md).
  The isolated full SMK host A/B passed all 16 arp steps, held pad/knob
  edits, original MIDI PB/CC1 and actual ADC wheel scanner (simulated input),
  all 740 state rows and normal Saw/PWM digests while Fold was live.
  [Control regression](sj6-fold32-smk-control-regression/README.md).
  On actual whole saved Solidify dry, the Fold correction restores
  a previously missing pre-FX harmonic (stock near −93 dBc, Fold
  ~−31 and original ~−30). Physically scoped source sum then reduces
  whole level gap −27.03→−5.45 dB, spectral RMS 13.83→6.61 dB;
  early and late dry signed release instead regress. Do not introduce
  the common sum solely as a meter correction. The saved Solidify
  disabled-Enable state has a stored nonzero Amount; 12 authenticated
  Mix0 dose pairs are bit exact, admitting one guarded physical
  suffix diagnostic if no effects/Unison or already active taps.
  [Independent authentic holds](https://github.com/gobbajogger/DnBOX/commit/17ec80e3).
- **Upstream dry differences vary by patch.** The AU references for
  Brainwave, Solidify and Raw J106 have matched cue/UID, fresh instances
  and authored repeats. A supported-control compact projection is useful
  to diagnose, but not a full matched implementation. Authored versus live
  residue in Brainwave, mapping gaps, and Solidify's native repeat floor
  must be reported. New one-at-a-time probes: #180 first-note correction
  shifts Brainwave's overall gap by under .008 dB; higher-precision ADSR
  ingress shifts whole RMS at most .006 dB. Neither explains the large
  drop. The documented 10 Hz high-pass floor also changes whole level
  at most .071 dB. Keep active missing routing/control investigations separate
  from the sample-history and oscillator corrections.
  [Original captures](sj6-native-factory-cross/README.md),
  [compact projection](sj6-compact-factory-cross/sj6-cross-preset/README.md),
  [per-control tests](sj6-factory-control-primitives/sj6-cross-preset/PRIMITIVES.md). A separate
  source-backed AMP-before-IIR diagnostic shifted held level at most .004
  dB, so it does not explain a broad settled-level deficit. It matters on
  transitions (Raw dry tail about +.40 dB on that probe).
  [AMP-order receipts](sj6-amp-before-filter/sj6-cross-preset/AMP-INGRESS.md).
  Replacing the immutable open Normal filter for qualified Solidify was
  also under .002 dB in whole-level change, so it is not the large loss.
  [Static Normal scope](sj6-static-normal-rejected/sj6-cross-preset/NORMAL-STATIC.md).
- **The compact dry mixer explains only part of the loss.** At unchanged
  Master and typed settings, removing only the fixed six-voice divisor
  recovers ~15.6 dB. Restoring unity duplication only for centered layers
  recovers another ~6.0 dB. Brainwave, Solidify and Raw remain 24.55,
  7.23 and 16.17 dB too quiet, respectively, with strong remaining
  spectral differences. Solidify's signed waveform residual worsens.
  [Exact typed diagnostic](sj6-typed-mixer-diagnostic/sj6-mixer-diagnostic/README.md).
  No tested dry cue clips, but this is not proof of full FX or dynamic
  control headroom. Do not call the mixer experiment a sound fix.
- **Short Classic ring headroom, not long delay.** E4M11 removed clipping
  with eight true compact typed source stress tapes, −80.86…−81.37 dBr
  vs float, unchanged 512-byte ring. Guarded Elite and Pro linked and
  agreed, but this implementation adds 176 application bytes and
  1,089–1,155 µs per 256-frame two-owner block. **Reject it as written**.
  Native disassembly identified an out-of-line memcpy on each tap;
  the ABI-proven no-copy bit carrier was tested. It produced identical
  E4M11 PCM on both boards and recovered 881–942 µs versus memcpy,
  but still adds 208–214 µs per 256-frame two-owner call and 144 B
  application flash vs Q15, with no extra RAM. It is not admitted without
  verifying that much populated-scene margin. [No-copy native receipt](sj6-classic-e4m11-nocopy/sj6-e4m11-nocopy/README.md).
  Straight bfloat16 is ruled out separately: the same-size store was
  around −56…−57 dBr and degraded the quiet cue.
  [Native memcpy rejection](sj6-classic-e4m11-native-rejection/sj6-e4m11-native/README.md),
  [host bfloat rejection](sj6-classic-bfloat-rejected/README.md).
- **Sub-specific dry mismatch:** Compact ignored typed Range before its
  Sub zone and pitch. The source-backed host repair moves Brainwave Low
  from 66 to the original 33 Hz and materially improves dry levels in
  Brainwave and Raw; isolated Main and no-Sub Solidify are unchanged.
  The corrected Sub still has −25.8 dB fundamental error, different odd
  harmonics and onset-dependent phase. The initial 128 frames correlate
  almost perfectly with the original, then the mismatch changes sign:
  a constant phase offset or time shift is not the answer. Exact fader
  mapping only buys .031 dB; measured compact filter H1 loss is ~2.25 dB,
  not enough to explain the remainder. Resetting on the authentic
  pending commit frame without fixing the carrier/state worsened
  signed chord and release; do not adopt it alone.
  Independent established adjacent cue (source key55 / SFZ root54)
  shows the original H3 relative phase changing from −18.70° to
  +149.08°, while corrected compact stays near −180° on both.
  This is after canceling a global gain/time shift; don't use a single
  phase compensation across sample zones.
  The original scoped HP10 → connected crusher DC .995 → terminal DC .999
  accounts for most terminal rotation; direct chronological original-Core
  replay matches held H3 phase to about .04° and 1.2° respectively.
  Inverse terminal equivalents are −174.79° and −131.62°; actual source
  sampler postfilter −174.73° and −133.83°. Avoid fitting terminal phase
  as a direct oscillator correction. [Chronological suffix proof](sj6-sub-physical-suffix/sj6-sub-body/README.md).
  [Exact source and signed scorer](sj6-sub-range-and-signed-shape/README.md).
  Current original Core + authorized media in RAM, at matched held Sub
  input, shows carrier RMS within .63 dB of compact across the two zones.
  Compact post-voice filter path loses about 2.1–2.4 dB relative to the
  original receiver; the exact mixer divisor causes 21.58 dB. About
  4.7–6.3 dB remains after accounting for the divisor, including that
  filter loss. Nothing here attributes the remainder to the limiter.
  [Absolute stage proof](sj6-sub-absolute-decomposition/README.md).
  Joining the scoped upstream mixer and physical phase suffix does not
  qualify the sound: only saved Raw dry passed both route predicates;
  sustain improved but signed release regressed from −6.49 to +3.98 dBr.
  Five refused arms were bit-identical to mixer-only. Genuine six-voice
  overload tested two history paths: typed Classic Q12 wrote 20,480
  without clipping; legacy Classic saturated 1,368 of 10,240. This is
  a signal-stage risk, not a universal saturation bound.
  [Joined route rejection](sj6-sub-joined-phase-rejected/README.md).
  Raw release physical pole test: original is already effectively
  **unforced** after ~13 ms, modal residual ~−116…−121 dBr; the compact
  pre-filter input stops at +13.537 ms. The failure comes predominantly
  from differing source waveform/state driving the HP before release,
  plus subsequent exact tail state, not a long absent envelope or
  permission to mute the filter. [Source-causal test](sj6-raw-unforced-release/README.md).
  Exact generated compact tapes show inherited pre-off state alone
  already +4.75 dBr versus original release and opposing polarity.
  The forced compact release and inherited state reinforce, not cancel.
  A fixed-clock held lattice (before off) plus verified suffix inverse
  shows the main-dominant second harmonic is about 3.8 dB too weak
  relative to Sub, and its relative phase differs. This is conditional
  composite evidence, not a knob gain to apply. Original Raw Saw at
  this tested fixed scope ignores PWM width like the compact Saw;
  isolated relative harmonics already match closely. Do not change
  width or a global gate time to cover this.
  In-memory authentic Sub-carrier injection at actual existing note
  ownership moved Raw joined-physical-suffix single-release from
  +3.98 to −3.72 dBr, late tail +9.40 to −4.36 and whole −1.45 to
  −3.55 dBr. It fixes part of the initial pole amplitudes/signs.
  Without the physical suffix that same injection regresses the
  center-only release. Do not copy the original samples to firmware;
  this is a diagnostic confirmation of carrier × physical route,
  not a production or fully matching implementation.'
- **Unresolved:** source owner of the gain between the calibrated Crusher
  and Apple's compressor input; which missing or misrouted per-program
  states dominate different dry patches; delay-bandwidth correction that
  meets all ranges, retiming, stable feedback, RAM and cycles.

## Synth One: ruled out, supported, and open

- **No more global wet gain, EQ or patch-specific compensation as a fix for
  Pad's release:** full-rate delay on the host changed its error from
  about −6.3 to −.12 dB. That localizes loss to delay history bandwidth;
  the full history is not a firmware fit. For the Organ, the original
  50 ms right-channel widen and original reverb, rather than an unrelated
  width effect, explain the examined right-tail residual. Qualify any
  compact changes on all selected signed comparisons again.
- **No replay of rejected delay storage approaches:** public-duration cut
  removed unreachable allocation, not the cost of valid long history.
  Dense lossless alone, recent-only detail, a sparse recent reconstruction
  and cheap velvet/reverb approximations did not preserve tested tails.
  [Dense delay](s1-dense-delay-compression/README.md),
  [duration bound](s1-public-delay-horizon/README.md).
- **No assumed arena reuse:** both owners can carry delay, room and widen
  simultaneously, and the master delay remains independently live.
  The measured exact-quality E6M9 room representation needs 98,760 B for
  two owners and is 93,996 B short when keeping other tested FX. A 4.7 KB
  age-dependent RevSC alternative retained exact recent words but lost
  16–17 dB on the Organ tail and about 16 dB on Pad's first-echo midrange.
  This is about known representations, not a proof all codecs are impossible.
  [RAM liveness](s1-fx-ram-liveness-and-age/ram-liveness/README.md),
  [rejected age model](s1-fx-ram-liveness-and-age/ram-age-model/README.md).
- **Phaser optimization is real, but not full timing/fit.** Exact cached
  invariant math matched 906/906. The bounded cosine candidate precisely
  retained musical behavior on 906 host renders with very low error and
  reduced the isolated steady PI32 leaf from 58–59 to ~2 ms/512 frames.
  The rare slow paths are not hypothetical: real subnormal tails and
  continuously smoothed Width/Rate caused repeated fallback. Signed
  subnormal rounding and bounded coefficient math now have isolated
  and live-edit proofs; do not treat the earlier 906 run as proving the
  latest combined candidate. [Exact initial baseline](s1-phaser-invariant-coefficients/README.md),
  [bounded cosine](s1-bounded-phaser-cos/README.md),
  [signed tiny rounding](s1-phaser-tiny-rne/README.md). The newer combined native
  leaf measures ~2.12 ms/512 frames under every-frame in-range Width/Rate
  changes, vs 77.5–78.1 ms all-libm, with zero Width/Rate fallbacks on 906
  connected host replays. All note tapes matched, 445 PCM rows bit-identical,
  worst whole error −75.88 dBFS RMS. Release admission is an independent
  problem: 108 B compact owner vs 224 B matched owner in the checked
  layouts, and shipping verification disallows soft-double routines
  still invoked by guarded exact fallback and Level preparation.
  [Combined proof](s1-phaser-combined-guarded/README.md).
  Do not
  bypass that restriction or misreport the isolated fixture as target fit.
  An actual guarded Elite insertion exceeded application flash by
  **12,224 B** although backend union/pool was unchanged. The isolated
  code-size result cannot predict that linked soft-double/libm cost.
  The current compiled variant is rejected for normal firmware admission.
  A legal stationary Rate reaches an out-of-guard notch angle after
  119.575 seconds, from drift in the source rotor. The combined 906
  capture length alone cannot justify removing that guard. Private
  Level is fixed 0 dB and can use exactly 1; preserving longer valid
  sessions still needed a no-soft-double finite range reducer. The new
  192-bit Payne–Hanek guarded Float32 route preserved finite range
  and signed-tiny behavior; six million tested words max cosine error
  1.07e−7, and native signed-product error ≤5.96e−8. The strict Elite
  link map showed no soft-double calls, but the link **failed with APP
  overflow of 2,112 B** (2,116 B with the packer marker), up 2,552 B over the
  matched stock donor, unchanged pool. This is a ∼10 KiB reduction
  against the double variant, not a firmware fit. Next isolate the
  structural wrapper/library contributions in the new map.
  Sharing the exact stereo recurrence then saved 736 B in the paired
  Elite map; cold factoring saved another 464 B each, with 906/906
  before/after wet PCM and live arp bit-identical. Combining the
  separately verified exact font (+160 B) and grouped cold changes
  (+1,280 B) in a **fresh guarded paired link actually fits**:
  +528 B Elite/+540 B Pro APP, +564 usable pool above reserve.
  This is INIT-only never-install diagnostics. The full combined synth
  product and zero dropout remain unverified.
  [Strict failed map](s1-phaser-no-soft-double/README.md),
  [shared recurrence](s1-phaser-shared-recurrence/README.md),
  [cold exact](s1-phaser-cold-factoring/README.md),
  [verified guarded pair](s1-phaser-guarded-el-pro-fit/GROUP.md).

## Timing after the first actual S1 phaser fit

Exact never-install pair Elite 8aeaa2e6… / Pro 23970350… replayed the
sealed unchanged 922-event tape, zero skips, actual audio and no NOR
changes. Elite max 14,617 µs, 73 late/xruns; Pro 14,741 µs, 84 late/xruns.
Both fail independently of the missing rendered-frame anchor. Valid
point owner readbacks do not establish per-producer onset. Previously
observed exclusive load Core0 roughly 6 ms 909 + ~3 ms Chicago; Core1
~7.4 ms Synth One plus 808 prefix, with no proved safe IRQ removal.
The 909 triangle direct head optimization is exact over 56,000 state and
four-bus spans, 14.7–17.6% faster per isolated diode kernel. Do not figure
that percentage on the whole render or count its tiny leaf linker size
as actual APP. The new exact-source triangle hunk must be retested on
the later future-worker donor and measured in a fresh serialized scene.
[First fit pair results and 909 proof](strict-fitted-pair-and-909-leaf/).

On the exact S1+Low Range+909 diagnostic pair (APP +768/+780 B), the
unchanged serialized scene yielded Elite 14,817 µs/87 xruns and Pro
14,603 µs/104 xruns. The producer-anchor issue remains; there is no
single-candidate timing attribution from these numbers. The private
donor retains a **verify-only 909 future**: it computes a private
909 voice on Core1, then recomputes the actual voice and compares
state/PCM on Core0 before emitting original. Confirmed 1,638/1,561
exact duplicate verifications, 95 unavailable on each, and zero
state/PCM mismatches. This is superfluous measurement work, not the
fixed-head change. Test removing only that duplicate diagnostic path
on the same pinned inputs and unchanged actual C1 plan. An instrumented
Elite late DMA ordinal 419 had ~748 µs nested TIMER5, which cannot
explain the full overrun (>13.59 ms even without those ticks).
There is no evidence to disable required interrupts.
The actual no-shadow serial pair preserved 56,000 signed state/four-bus
host comparisons and recovered 480 B APP. It still failed unchanged
922-event no-write diagnostics: 14,301 µs/79 xruns Elite and 14,394 µs/77
Pro. Missing producer-anchor/unequal positive owner observations prevent
claiming a controlled causal timing delta. This is the donor for the
later actual Fold/warm fitting pair, not that pair's timing result.
[Exact no-shadow evidence](strict-909-no-shadow/strict-fit-range909/serial-without-shadow/README.md).

Exact latest warm-phase + Fold INIT-only guarded pair (+252/+260 B
APP; +564 pool) used matched regions and unchanged sealed 922 events.
Elite 11,976 µs / 1 xrun; Pro 14,509 µs / 70 xruns. Both actually
fail, even though Elite is much closer. Audio is nonzero; C1 active,
zero skips, exact virtual NOR unchanged. No producer anchor.
A separate active silent-Sub meter found no matched live owner-A
render/canary at all, so the silent-Sub speedup was **not** admitted
and cannot explain Elite/Pro from these captures. Prioritize proving
actual Pro late-half work rather than rerunning identical aggregate
scenes. [Exact warm pair and meter](strict-warm-fold-timing/strict-fit-warmfold/proof/README.md).

The exact Synth One silent-Sub fast path saves ~603 µs in a six-voice
512-frame *isolated* zero-Sub case; audible or exact-zero main fallback
can be slower by up to 97 µs. Saved end-of-run SUBV reads as zero with
all voices no longer active; that does **not** prove eligibility in
the worst half. Only test admission after contemporaneous active
signed owner counters on a clearly non-timing witness.
[Exact idle-Sub proof](s1-silent-sub-scoped/README.md).

## Further root-cause narrowing

- **SuperJ6 J60V2, not Classic.** On the Raw J106 authored case, Chorus I
  activates the original 96 kHz J60V2 path. The compact chorus adds wet
  audio immediately rather than reproducing the two converters' signed
  response. Generated converters with the independently clocked original
  middle kernel reached −145.71 dBr or better on eight host fixtures;
  +47/+48 frame impulse peak timing was exact at 44.1/48 kHz. Mix 0 and
  stopped were signed exact bypass. Not a target fit. [Exact cheap
  clock proof](sj6-j60v2-clock/README.md).
- **Solidify voice.** The real co-timed chord captures rule out slow
  held-note glide as the cause: the final note is already present in the
  earliest stable slice. Keep current ownership; trace envelope kinetics
  and per-layer amplitude independently. Do not revisit master volume.
- **Pro timing.** A second lean probe at ordinal 484 measured 14,305 µs
  and 62 dropouts. Core 0 waited 8.625 µs at the instrumented A-before-B
  dependency despite 6,457.875 µs elapsed on the Core 1 pre-stage path.
  That specific barrier cannot explain the 3,496 µs budget miss. Do not
  add measurements across differently instrumented runs. The guarded
  images have synthetic legacy drum hits in addition to the external
  tape, so this is stress evidence, not release admission.
- **High-pass behavior.** Seven saved zero-resonance fixed-cutoff captures
  matched two-state TPT to −143.68 dBr or better. This does not prove
  continuity at a parameter change: valid filters with different retained
  state diverge during a warm switch. The warm lab's strict dry suffix
  improved first-release signed error from +0.086 to −0.389 dBr; the
  production typed projection still conflates connected dry with bypass.

- **Sine gain alone rejected.** The original Sine per-layer factor
  1.237790942 made Solidify +1.853 dB louder but made centered dry
  signed whole residual +0.864 → +1.233 dBr. The modified compact output
  nulled against the previous compact output times that factor at
  −139 to −140 dBr. Raw and Brainwave remained byte-exact, so this
  does not repair shape or phase. No gain change adopted.
- **Amp envelope alone rejected as the remaining Solidify cause.**
  The original prep implementation matched 15,306 original stage and
  amplitude words, but replacing it on a reusable typed cold Main route
  barely moved Solidify signed whole error: −0.9747 → −0.9750 dBr.
  The phase-insensitive final early-release error moved 8.293 →
  9.226 dB and the whole spectral error 6.625 → 6.694 dB. Untouched
  controls/other route cases stayed exact. Do not infer that original
  note scheduling/smoothing was implemented everywhere.
- **Synth One delay split-band rejection.** With an optimistic 2,152 B
  pool and four bits per band coefficient with zero bookkeeping,
  Pad and Hall signed dated tap residuals remained roughly
  −23 and −16 to −19 dBFS. A short treble buffer cannot supply
  Pad's ~216 and ~432 ms history. This rejects the tested allocation,
  not every possible compact representation.
- **Pro serial mixer rejection.** Fusion preserved the exact raw scope
  and PCM, but saved only about 5 µs per measured 256-frame half, for
  126 extra bytes. Not adopted against the much larger system miss.

- **Sine phase is causal for the early attack, not sufficient.** A
  quarter-cycle authentic origin alone improves Solidify centered dry
  attack from +2.887 to −2.116 dBr, and hold from +3.039 to −1.442.
  The late dry tail gets worse (+0.344 → +3.331). Independent pure Sine
  4× phase with original halfband remained within 0.063° across six
  pitches, but it did not reconstruct the complete loaded modes.
  Raw and Brainwave were signed exact untouched. [Source and
  measurements](sj6-solidify-sine-phase/README.md).

## Release boundary

These are lab diagnostics. A host proof or a leaf emulator measurement
does not qualify full Elite/Pro fit, cold stack, or populated 0 dropouts.
The direct-write storage variant and boot/editor sharing are candidate-only
(16 B further saving, still 52/44 B over in the guarded diagnostic; no
native high-water and whole timing).
Do not treat INIT-only as downloadable preset admission: the strict
companion guards currently reject INIT_ONLY=0, and the original S1
pack reader expects the FDATA base where the fitting donor has D9RD.
The recovered cab25d44 prototype pool overlaps live BREAK slot 2
and 909 FDATA including both directory sectors. Do not overwrite
either or silently enable a new USB endpoint. A legally mapped,
guarded provider/installer arrangement is still required.

Keep arp and all agreed controls, never use PSRAM, and enforce the protected
pool/headroom. No release until the combined target gate passes. Add a row
and its evidence here whenever a hypothesis is rejected or corrected.
