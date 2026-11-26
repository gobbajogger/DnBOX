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
  an ABI-proven no-copy bit carrier may be worth one independent test.
  Straight bfloat16 is ruled out separately: the same-size store was
  around −56…−57 dBr and degraded the quiet cue. [Host bfloat rejection](sj6-classic-bfloat-rejected/README.md).
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

## Release boundary

These are lab diagnostics. A host proof or a leaf emulator measurement
does not qualify full Elite/Pro fit, cold stack, or populated 0 dropouts.
The direct-write storage variant and boot/editor sharing are candidate-only
(16 B further saving, still 52/44 B over in the guarded diagnostic; no
native high-water and whole timing).
Keep arp and all agreed controls, never use PSRAM, and enforce the protected
pool/headroom. No release until the combined target gate passes. Add a row
and its evidence here whenever a hypothesis is rejected or corrected.
