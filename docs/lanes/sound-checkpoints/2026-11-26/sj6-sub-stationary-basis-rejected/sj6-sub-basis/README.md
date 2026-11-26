# Stationary Sub source-zone basis: promising controlled roots, rejected holdout

Source-oriented model, not program tuning. Each old authorized SFZ recording
is decoded in private process memory. The model fits only the middle-late
20% of each true physical loop span (60–80%), never onset, early held/release,
or any prescribed MIDI/preset cue. Nominal clock fixed from root key, absolute
time origin remains the actual sample starting address. Seven signed
partials + DC, quantized to Float16 in memory only. No coefficient table,
recordings, extracted sound, source binary, normalization, inferred gain,
phase shifts, different waveform clocks, or product writes. Probe passes
synthetic and true media separately through the actual current compiled
original sampled receiver with unchanged callback, fader, hold/release,
note handoffs and filtering, not the public Mac terminal graph.

Logical descriptor size 15 × 12 × 2 = 360 bytes, before renderer or mutable
states. There is no flash/target claim because the sound acceptance fails.
Actual through-current-receiver signed results (onset / held / release):
- Root36 +4.425/+4.200/+4.147 → −17.857/−16.379/−16.932 dBr.
- Root54 +5.234/+5.298/+5.317 → −6.750/−13.264/−12.727 dBr.
- Blind played key73 (root72) −5.260/−6.241/−7.102 → −2.224/+.914/+1.229
  despite held level error just −.007 dB; exact levels do not prove fidelity.
- Original pending region signed improves −1.595→−12.654 dBr; commit and
  reset timing are unchanged, not evidence of equivalent time-varying sound.

Independent source-clock cause: lower raw body-window H1 starts within
0.53°/4.24° of the late body calibration phase in roots36/54. Root72
start differs 43.56°, and its native per-0.2-s physical phase progression
in these windows is ~1.386° against the frozen nominal carrier, directly
observed on authorized old media clock. A stationary per-zone model
doesn't preserve that early temporal behavior. This shows a real
source-time dependency, not grounds to fit a phase offset, time shift or
program-specific pitch from held audio. A general source-cursor/structural
basis that tracks the underlying sample phase with blind note/loop/release
validation remains open. Do not adopt this model on its two successes.
Older media may not be an identical transient source of all installed
newer AU zones; no such claim is supplied.

Reproduce with prior authorized source kit and pinned current oracle and
archive, from workspace root:
  PYTHONPATH=/tmp/sj6-owner-packages:ab-effects-readonly/sj6-fullbench/bench \
  OPENBLAS_NUM_THREADS=1 python3 sj6-sub-basis/probe.py
  PYTHONPATH=/tmp/sj6-owner-packages:ab-effects-readonly/sj6-fullbench/bench \
  OPENBLAS_NUM_THREADS=1 python3 sj6-sub-basis/source-drift.py

No original waveform or fitted coefficients leave the process. Receipt
contains only signed comparison results and bounded analytic phase observations.
