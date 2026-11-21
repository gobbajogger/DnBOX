# Variant D is frozen for root review

The single authorized D implementation is compiled and its bounded host and
ownership checks pass. **No D reference-PCM comparison or component capture has
run.** The ownership test evaluated the explicitly permitted in-memory sample
sequences. This is preparation evidence, not an audio-fidelity result.

All work is confined to `/workspace/scratch/e803955aec60/s1_dry_voice_d`. The six
frozen C renderer source hashes remain unchanged. Root remains the sole
canonical writer and owns review, checker registration and any subsequent
audio authorization.

## Exact change

D changes only C's nonzero-resonance feedback:

```c
static float feedback_solution(float w, float c)
{
    return w / (1.0f + 5.0f * c);
}

/* Existing nonzero commit path, in its original order: */
float nonlinear = 5.0f * d;
float v = input - R * nonlinear;
```

The complete `R==0.0f` branch is byte-identical to C, including its Chicago
helper evaluation and multiplication by zero. The helper itself is unchanged.
The five other renderer source files are byte-identical to C. The saved
`reports/C-to-D.patch` contains the two semantic changes and their explanatory
comments; no other executable expression changes.

The forward poles, `q/w/c` evaluation order, state/history shifts, reset,
`R=physical_resonance*0.8f`, coefficient preparation, two held-input substeps
and mean, physical domains, oscillator phase, squared velocity, component
gain, pre-filter VCA, envelopes and note events are retained. There is no
fitted gain, slope or cutoff mapping, no new clamp, and no tail retirement.
Removing the Chicago subtraction changes its quantized microtail naturally;
this is not a promise of bit-identical nonzero-resonance tails.

The measured host state remains **160 bytes**: 92 prepared/control bytes and
68 mutable history/flag bytes. This is host layout evidence, not target fit.
The core object's only undefined symbols are the two permitted envelope
component functions; no libm or allocation dependency is introduced. [1–3]

## Why this one hypothesis

The matched C filter-only numeric records show resonance-0.9 steady sine gain
falling from +7.386123159 dB at amplitude 0.0001 to +5.411479316 dB at 0.01 and
−2.711459097 dB at 0.25. The corresponding original numeric observations stay
near +8.219749 dB across the three levels. Those measurements motivate
removing nominal-range compression. They do not establish global linearity
of the original engine. [4, 5]

The slope five is the permitted Chicago primitive's existing derivative at
zero, rather than a fit to these recordings. Substituting `phi(d)=5*d` in the
existing centered equation `d+c*phi(d)=w` gives the direct solve. The independent
diagnosis bounds its denominator to `[1,1.101)` over the physical coefficient
domain. Its frozen-coefficient, exact-real stability derivation uses maximum
actual-binary32 `5R=3.920000195503235<4`. This is not a proof of arbitrary
parameter modulation or exhaustive binary32 stability. In particular, the
old saturated-loop bound `|input_to_poles|<=|input|+R` no longer applies. [4]

At cutoff/frequency 2,000 Hz, the independent analytic prediction is
−12.055958344 dB for resonance zero and +7.415658970 dB for binary32 resonance
0.9, amplitude-independent in that model. The latter still predicts about
−0.804012 dB against the recorded original fundamental. These are predictions,
not D measurements or acceptance results. Remaining phase and high-cutoff
transfer differences are not resolved by this change. [4]

## What ran

`prepare.py` wrote `BUILD-DECLARATION.json` before compilation, then compiled
the host core, host harness and reused ownership test serially. It wrote
`PRETEST-DECLARATION.json` with source and binary hashes before any candidate
executable ran. All commands have preserved attempt-01 receipts, timestamps,
exit statuses, stdout and stderr. There were no failed attempts. [1, 2]

Compiler flags were C11, `-O2`, `-ffp-contract=off`, `-fno-fast-math`, warnings
as errors, `-Wdouble-promotion`, and `-DX0X_HOST=1`. The compiler is the pinned
host GCC 14 binary recorded in the build declaration; there was no native
build, emulator, timing benchmark or full suite.

| Check | Observed result | Meaning |
| --- | --- | --- |
| Host `--describe` | 160-byte voice; six expected envelope coefficient words | Metadata and unchanged coefficient preparation |
| Unchanged host `--selftest` | PASS | Input validation/nonmutation, complete physical endpoints, squared velocity, gate/reset, unsupported retrigger |
| Reused ownership test | 8,192 sample-word comparisons exact | Independent single-frame executions equal interleaved 1/17/64 block partitions |
| Reused ownership test | 310 full 160-byte state checkpoints exact | Each processed owner's state agrees with its independent execution |
| Reused ownership test | 310 untouched-owner checks pass | Processing or changing one owner does not mutate the other |
| Reused ownership test | 16,384 finite-output checks pass | These bounded live-control trajectories remain finite |

The two owners have different controls, frequencies, velocities and phases,
four versus five live physical edits, and asymmetric note-off frames 2049 and
3071. They include cutoff endpoints, resonance up to 0.98 and filter depth
1.2. Each logical owner is evaluated for 4,096 frames independently and 4,096
interleaved, totaling 8,192 frames per owner. No PCM file is produced and no
reference PCM is read. The reused test source retains its historical “frozen
C” header comment, but the recorded build links D's frozen core object. Its
source SHA-256 is unchanged from the focused C follow-up. [1, 2]

These checks establish deterministic independent ownership and partitioning
for those trajectories. They do not qualify dynamic-control sound, every
possible modulation path, or the 180-note reference gate. C's nonlinear scalar
fixtures were not reused for a different feedback law.

## Freeze and next command

`DECLARATION.json` is the final pre-audio freeze. `COMPARATOR-SOURCE.json`
contains the six source hashes for root to register as variant `d`. The core
SHA-256 is `dab6afb5374a826bdd696f750709f22eaf658da8d9ddbf959646e95318cb43b1`;
the host binary SHA-256 is
`1a9691cedd3836364dc3d4e96f7fed87604b00570efabf729e7e4e6e7f6e8751`.

After root reviews this freeze, registers the six hashes and authorizes the
single comparison, the exact proposed command is:

```sh
python /workspace/scratch/e803955aec60/DnBOX/experiments/sound-hardware-v1/dry_voice_followup/check.py \
  --variant d \
  --reference-root /workspace/scratch/e803955aec60/dry_reference_packet/reference_pcm \
  --output /workspace/scratch/e803955aec60/s1_dry_voice_d/results/audio180 \
  --all
```

That command has not run, and the output directory does not exist at freeze.
The unchanged acceptance conditions remain comparison gain 1, RMS error at
most 1 dB, spectral error at most 3 dB and zero nonfinite samples, with no gain
fit, time alignment or normalization. The metric and input-manifest identities
are pinned. No D accepted-note count is claimed.

The 72 dynamic cases, 694 intact presets, all 150 control IDs, FX, full note
lifecycle, native cost/fit, callback deadline, phase/high-cutoff fidelity and
production activation remain unqualified.

## Evidence

1. `BUILD-DECLARATION.json`, `PRETEST-DECLARATION.json`, `PREPARATION-RESULT.json`,
   `logs/*-attempt01.json`: exact commands and observed terminal results.
2. `results/describe.json`, `results/host-selftest.json`,
   `results/ownership-selftest.json`, `tests/ownership_test.c`: actual bounded
   checks, with source and binary identities.
3. `reports/C-to-D.patch`, `logs/core-undefined-symbols-attempt01.json`: source
   change and core dependency evidence.
4. `inputs/NONLINEAR-HYPOTHESIS.md`: frozen independent numeric diagnosis and
   derivation, SHA-256
   `ea2e830a98392567a9055b71296feb4764fbb32ecbbf613641823de8f16ace11`.
5. `inputs/C-filter-observations.json`: completed matched C observations,
   SHA-256 `85c088668cb1106cc5c32e6438c0e6535c82ea328d21435c91bb45863c3b372a`.
   No original capture source, build inputs, headers, objects or algorithm
   implementations were read in this D work.
