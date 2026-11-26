# Current generated OSC2 source, direct native signed level, and static/open experiment

From workspace root with the cryptographically pinned private Mach-O pointer
and original bench/Unicorn on PYTHONPATH, run probe.py, transition.py and
cutoff_words.py. SHA256SUMS covers source and receipts. The alternate native
archive is authorized direct-AUv3 fan-in, SHA256 d3821515…df9f.

Actual latest original generated OSC2 uses the exact shape2 initializer
word 1.5309427976608276 from guest image rodata and its 48 zero tail bytes,
applied for all 128 keys. The actual generated helper executes at
0x1003b69a4. Main-note AMP/receiver program is historical projected; the
native direct oscillator-only stream is not used to drive or tune it.
At held, source leaf RMS .15537694; genuine native terminal .02832472.
The verified source-side centered suffix with no external Apple HP in the
primary arm is -8.88038 dB from actual current native, null -1.82557 dBr.
Dropping the source profile makes it -12.57955 dB; this +3.69918 dB is a
positive control for actual original profile loading, not a firmware fix.
One/two/four oversampling arms differ <.002 dB of held output. Source-loaded
4× leaf receives independent +.25 injection and preserves that through both
halfband averages within 3e-8. A transparent 10 Hz HP modeled separately
changes held level by only .0035 dB; not the actual connected Apple instance.

The fixed full chord probe finds signed correlation -.4504 immediately
precommit while its absolute error is only .0734 dB, and correlation .9937
just after with -8.5733 dB. Dynamic current Swift cutoff/envelope/modulation
and original connected Apple state remain unproved; do not correct gain from
this terminal data or bake the diagnostic phase event.

stage_open.py generates a patch of the already proven direct native host,
not a program binary. Capture exact same cue, OSC2 only and repeat, source
profile untouched; LFO1/2/3 off, raw cutoff1, tracking/strength/velocity0,
same authored resonance .4141378700733185 and master. Genuine current
compiled conversion gives Normal raw1 physical 20000.001953125 Hz
(OTA guard 30000.0078125 Hz). Once captures exist, open_match.py verifies
readbacks/cue/hashes and compares the new source output. Tool requires
recovered dependent Python source and existing private test archives,
not included in the three-file Mac source staging bundle. No installed
application, native capture, source-only candidate or product changed.

## Outcome against new direct static-open Mac capture

The matching new archived current Mac capture (SHA256 6cc86223…03384) ran
twice with unchanged parameter readbacks and genuine signed PCM. Use
open_match.py and fingerprint.py against an unpacked capture directory.
The guest source with scoped zero-effect suffix remains -10.45593 dB
held, repeat -72.52 dBr. Near the known chord handoff, negative correlation
still occurs at +1.12967 dB, then +.97502 correlation at -9.02101 dB.
Both original and current source peak at ~32.636 Hz (C1 because the actual
OSC2 key36 uses -12 semitones); independent RMS/phase harmonics disagree
by nonuniform amounts. This rejects filter-animation-only and simple
global gain, an octave mistake, and loss during halfband averaging.
The live connected Apple transfer at the same absolute input level,
and exact modern app-time loaded oscillator/filter branch, are distinct
unknowns. No equalization, authored changes, inferred gain or skew adopted.

## Corrected physical vs harmonic cutoff route and paired direct AU proof

The previous #129-only wavestream never established physical filter open.
Current registration readback helpers identify #7 by Core normalized cutoff
and #129 by oscillator harmonic filterCutoff. In modern Normal, actual
#7 dispatch 0x1000eeca8 updates all source lanes' addr36,
211.95989990234375 at .3345831036567688 and 20000.001953125 at 1.
The independent harmonic DSP addr6 changed 4→10 with exactly zero
waveform difference in this controlled OSC2 Normal replay.
The corrected same-physical arm is -9.0642 dB held versus the #129-only
native capture. Actual #83 .6666666865348816 drives enum2, Saw on,
other shapes off and physical DSP selector2 on OSC2.

The additional actual native #7 archive SHA 9581a0d4…5f09b was
scored with actual fixed-source DSP cutoff arms 211.96 / 300 / 20,000.
First 128-frame signed difference correlation is .97305; held AU
difference is -79.98 dBr versus source -4.58 dBr. This localizes
difference to physical filter state retention after initial onset or
a different route that dominates after. It does not establish the
specific live setter. The live stage generator tests step at24576
and restore61440, with pure actual compiled source prediction.

## First-note cutoff base restoration and active rollback

Current app first-note branch .018b44..c18 tests empty held-note list,
LFO1 delay >= .01s, and real LFO1 destination enum7. Current source
constructs its destination list at .059d9c, maps public #181 index1 to
enum7, and setter .0f8ddc maps real #91 to .851302187571 s. Actual note
source invokes .269e74 using stored #180 base without rewriting public #7.
Actual .269fa8 elapsed-time smoothing (tau .015) sends real sampler addr36
settled near 211.96 Hz, with the nominal audio-block tick test. In held,
short-delay and non-cutoff-destination guards do not trigger. The source
owner also verifies active direct #7 rollback applies a relative change
to its separate live target, clamping target from .33458 to zero in this
particular verified control sequence, without an immediate physical write.
The actual timer then converges toward 20 Hz. This is generalized source
behavior, not a per-preset tuning or CPU/timer claim.

note_filter_owner.py independently observes current compiled slot36 and
live Normal-filter coefficient inputs through source voice commit without
an app-originated write: physical inputs stay at known 211 or 20,000
integer Hz through chord; only the direct two control commands write them.
The higher-level app note relatch is therefore necessary to reproduce the
real original dynamic held/chord convergence for this set of inputs.

cutoff_latch_model.py is independent comparison control logic, not original
app code and not enabled in target firmware. verify_latch_model.py checks
all four source guard traces plus active rollback. score_live_latch.py
compares exact newest static + dynamic native archives and both no-latch
and causal candidates, using real callback sample-block elapsed time as
a tested control scheduler, without claiming native Apple timer cadence.

- Dynamic held after chord: baseline contrast -2.92 dBr, modeled causal
  restore -53.72 dBr, actual native -59.22 dBr.
- Static-open held: baseline contrast -4.59 dBr, causal restore -137.24,
  actual native -78.48 dBr (at repeat noise floor for that fixture).
- Dynamic restore while held: baseline incorrectly ~0 dB, candidate
  -16.22 dB, true native -29.99 dB. Absolute candidate-v-native level error
  on that arm improves +19.79 to +3.57 dB. Ordinary authored held waveform
  remains approximately 9–10 dB low; no whole-preset sound match claimed.
