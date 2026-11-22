# Variant C is compiled and frozen; note audio has not been rendered

This is the one centered-feedback variant requested by the root owner. Its
implementation, host renderer, separate scalar tests, binaries and declarations
are confined to `/workspace/scratch/e803955aec60/s1_dry_voice_c`. Frozen A/B and
canonical production files are unchanged. There is no audio-gate result for C.

## Change under review

C keeps B's forward pole recurrence, prepared pole coefficient, two held-input
filter substeps, output averaging, raw oscillator, envelope preparation,
squared velocity, pre-filter VCA, physical domains and absolute gain.

At nonzero resonance, C replaces the two-substep feedback delay with

`d = (y_current + 4*y[-1] + 6*y[-2] + 4*y[-3] + y[-4]) / 16`.

The old forward states first produce the current zero-feedback output `q`
without mutation. With `b=1-A`, the scalar equation is

`d + c*phi(d) = w`,

where `w=(q+4*y[-1]+6*y[-2]+4*y[-3]+y[-4])/16` and
`c=(R/16)*b^4`, with B's `R=0.8*physical_resonance`. Starting at `d=w`, exactly
six iterations perform `d=w-c*phi(d)`. The same forward recurrence then commits
once using `input-R*phi(d)`. The scalar loop was moved into the private helper
`feedback_solution` without changing its arithmetic.

The diagnosis agent's original proposal and derivation are preserved under
`inputs/`. Its real-arithmetic bound is maximum contraction approximately
0.0964564 and six-step error at most 1.5536261e-8 over the stated physical
domain. These bounds do not include binary32 rounding or qualify nonlinear
audio behavior. The compiled selected checks found a largest float contraction
of 0.0964563638.

The r=0 branch retains B's complete floating-point audio arithmetic, including
the nonlinear evaluation and zero multiplication. Only history assignments are
added. Existing `filter_history[3]` and `[4]` contain `y[-1]` and `y[-2]` before
a substep. Two new floats contain `y[-3]` and `y[-4]`; they shift before the old
histories are overwritten. They reset to zero.

Compiled state is **160 bytes**: **92 prepared/physical bytes**, **68 history
and flag bytes**. This adds eight bytes to B. It is host layout evidence only.

No output gain, postgain, fitted coefficients, physical clamp, shortened tail,
original engine source/assets, allocation or mutable globals were introduced.
The candidate object has no libm dependency. Native cost remains unmeasured.

## What actually ran

All three host C compilations succeeded with C11/O2, no fast math, no FMA
contraction, warnings as errors, and no libm link. `build.sh` records the exact
commands; compiler and undefined-symbol logs are retained. The pre-test
declaration pins source and binary hashes before any test was run.

| Check | Result | Scope |
| --- | --- | --- |
| Host `--describe` | 160-byte voice, expected six envelope words | Metadata and coefficient preparation |
| Host `--selftest` | PASS | Configuration, gate ownership, invalid-input nonmutation, complete physical endpoints, squared velocity, reset of added lags and explicit unsupported retrigger |
| Separate `scalar_test` | 38/38 solver outputs bit-exact | Independent diagnosis's fixed binary32 scalar numeric points |
| Separate `scalar_test` | 360/360 r=0 substeps bit-exact with B | Five cutoffs, positive/negative zero resonance, three initial-state patterns and twelve fixed scalar inputs |
| Separate `scalar_test` | 720/720 history/state checks | Four nonzero resonances, five cutoffs, three initial-state patterns and twelve fixed scalar inputs |

The separate scalar executable includes the actual candidate source and checks
its private solver/substep. Its comparison helper contains only B's frozen
zero-resonance filter arithmetic and is absent from the candidate/renderer
binary. It performs bounded scalar unit vectors, not a note render, a reference
PCM comparison, a timing benchmark or an audio acceptance test.

The C host selftest intentionally omits B's note-audio/block-partition loop so
this preparation phase does not generate note audio. Frozen B's own source,
host tests and results remain unchanged. C therefore makes no new block-
partition audio claim yet. This distinction is explicit in the saved outputs.

All tests passed on attempt 01. Each exact command, status, stdout and stderr is
saved under `logs/*-attempt01.json`; no failed attempt was discarded. No C PCM
file has been written, and no reference PCM was read during these C checks.

## Files for root review

- `src/s1_dry_voice.c`, `src/s1_dry_voice.h`: candidate and state layout.
- `src/host.c`: compatible renderer CLI, metadata and preparation-only host tests.
- `tests/scalar_test.c`, `tests/scalar_cases.h`: separate scalar/history checks.
- `tests/make_scalar_fixtures.py`: reproducible numeric-fixture extraction.
- `inputs/`: exact independent diagnosis proposal, derivation and scalar data.
- `PRETEST-DECLARATION.json`: source/binary snapshot taken before tests.
- `DECLARATION.json`: final source/binary/test-result freeze for root review.
- `results/describe.json`, `host-selftest.json`, `scalar-selftest.json`: results.
- `logs/`: build, compiler, symbols and exact test-command receipts.

## Pending action — root review and explicit authorization first

The root should register the six file hashes in `COMPARATOR-SOURCE.json` as
variant `c` in the current portable checker, then execute exactly one serial
180-note comparison after approving this freeze. The old `run_screen.py` is
neither present nor used.

```sh
python /workspace/scratch/e803955aec60/DnBOX/experiments/sound-hardware-v1/dry_voice_followup/check.py \
  --variant c \
  --reference-root /workspace/scratch/e803955aec60/dry_reference_packet/reference_pcm \
  --output /workspace/scratch/e803955aec60/s1_dry_voice_c/results/audio180 \
  --all
```

This command has **not** been executed. The output directory does not exist.
The portable checker visits the selected `physical-gain/100_2000_.9` case first
and then the remaining physical cases. The unchanged gate is gain 1, RMS at
most 1 dB, spectral at most 3 dB, nonfinite zero. C has no accepted-note count
until that check actually runs. Dynamic cases, intact presets, full controls,
FX, native fit, callback deadline and production activation remain unqualified.
