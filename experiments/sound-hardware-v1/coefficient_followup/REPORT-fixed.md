# Improved source-free integer coefficient candidate

`fixed_candidate.h` closes every observed fastmath coefficient failure in this
review without target binary64, libm, tables or shared mutable state. It matches
**5,360,032 scalar coefficient comparisons exactly** against the contract's
inferred double-expression hypothesis, including the previously failing legal
attack/decay/release times and the small filter-release region. It also retains
**zero differing words across all 379,288 pinned envelope outputs**.

These are finite, reproducible results. They do not establish every possible
binary32 control/rate combination, whole S1 behavior, target fit or target
control-update cost. The production coefficient path remains the root's
selection; no production source has been changed here.

## Candidate and numeric construction

The C entry point is:

```c
float s1_retention_fixed(float seconds, float sample_rate, int attack);
```

Its explicit caller contract is finite binary32 `seconds` in 0..2, finite
binary32 `sample_rate` in 8,000..192,000 and boolean attack. Enforce the narrower
public control bounds in the adapter: attack .0005..2, decay .005..2, amplitude
release .004..2 and filter release 0..2. A zero duration produces zero as an
explicit hypothesis extension; original recordings do not qualify that edge.
Invalid or nonfinite inputs are outside this helper's preconditions and must
be rejected by the real control adapter.

1. Decode the two binary32 inputs into integer significands and powers of two.
   Their exact product has at most 48 significand bits. Use attack scale 3/5,
   giving a denominator smaller than 2^50 and a numerator of either 1 or 5.
2. Form `x/128` in Q62 using integer restoring division and round to nearest,
   ties to even. An early cap handles values so large that the final coefficient
   is necessarily below the smallest binary32 result.
3. Evaluate the independently derived 20-term alternating Taylor series for
   `exp(-x/128)` in Q62. The 64x64 multiplication uses four 32x32 products and
   explicit high/low limbs; there is no compiler-specific 128-bit integer type.
4. Normalize a mantissa/exponent representation and square seven times. Keeping
   the exponent separate preserves the small coefficients that existing
   fastmath flushes to zero.
5. Round the resulting mantissa to binary32, ties to even, with an explicit
   subnormal-output path. No envelope level is flushed or retired here.

The helper is deterministic integer arithmetic after decoding its inputs, with
only local variables. It uses no coefficient table, dynamic allocation or
per-voice storage. It belongs outside the sample recurrence and can prepare
the shared current coefficients once per effective control update. It must not
overwrite an envelope stage's latched retention while that stage is running.

This uses exact rational 3/5 for the attack scale. The contract's host oracle
instead multiplies by the binary64 representation of 0.6 and rounds its
intermediates. Those expressions are mathematically slightly different. None
of the tested inputs exposes a binary32 output difference, but the finite
tests are not a proof that no binary32 duration/rate can expose one. The Q62
series itself also remains a finite approximation. No epsilon has been used
to accept either difference, and no universal correct-rounding claim is made.

## Exact evidence

`RESULT.json` is the final pinned-packet replay with source hashes and per-case
metrics. The C coefficient routine is called from the independently authored
Python envelope proposal. This establishes the coefficient path on the packet;
the root independently owns the C state-machine API tests.

`SWEEP-fixed.csv` records 3,200,032 coefficient tests for the integer method:
four rates, four control roles, and 100,001 linear plus 100,001 logarithmic
duration samples for each role/rate. Every integer coefficient word equals
the hypothesis oracle. The old methods and every old failure are retained in
both `SWEEP.csv` and `SWEEP-fixed.csv`.

`STRESS-fixed.csv` records a further 2,160,000 exact coefficient comparisons,
compiled with undefined-behavior sanitizer, `-Wall -Wextra -Werror`:

| Check | Comparisons | Differing coefficient words |
| --- | ---: | ---: |
| Random binary32 positive durations 0..2 and candidate rates 8k..192k, plus legal attack controls | 600,000 | 0 |
| Duration and adjacent binary32 durations obtained by inverting output rounding midpoints, four roles at four rates | 1,200,000 | 0 |
| Dense durations and neighbors across exponential normal/subnormal/zero transition | 360,000 | 0 |

UBSan reported no errors; stress exit status was zero. The random seed and
generation are pinned in `stress_fixed.c`. Midpoint inversions use host log
only to construct difficult test inputs; log is absent from the candidate.

Original output observations remain limited to 44,100 Hz. The 8k, 48k, 192k
and random-rate comparisons are mathematical-hypothesis/robustness tests, not
new original-engine support claims. All scalar counts include repeated
endpoints if the input generator produces them.

The retained negative control still rejects an attack coefficient raised by
one ULP: 8,332 of 16,384 live outputs differ, starting at frame zero. Existing
FM failures remain explicit: legal attack .0005599849974 gives oracle
`3f6f4afb` versus FM `3f6f4afc`; legal small filter release 2.597062689e-7 gives
oracle `00830c37` versus FM zero. The fixed candidate matches the oracle for
both cases and the full sweep that exposed them.

## Source and cost boundaries

`fixed_candidate.h` is **121 lines / 4,190 bytes** of independently authored
source. The helper contains one restoring-division loop, 20 series iterations
and seven squarings. It may cost more target cycles and flash than existing
FM. Its 64-bit division by the term index and integer operations may pull
target runtime helpers. Control smoothing may make many coefficients change;
no control-update frequency reduction is authorized by this result.

The isolated x86-64 `-Os` object is 792 text bytes, data/BSS zero, with no
undefined symbols (`fixed-host-object.json`). **That is a host object measure,
not target flash or link evidence.** No native compiler/link, timing benchmark,
full suite, emulator, RDC, GitHub, Git mutation or production mutation ran.
Two previously terminal native links were not reused or restarted.

Final resource receipts:

| Run | Exit | Wall seconds | Maximum RSS KiB |
| --- | ---: | ---: | ---: |
| Five-method 379,288-sample packet replay | 0 | 1.931532 | 15,232 |
| Four-method scalar sweep; fixed portion 3,200,032 points | 0 | 1.574090 | 9,344 |
| UBSan scalar stress, 2,160,000 points | 0 | 0.974533 | 9,216 |

All runs were serial and bounded to this task's isolated directory. The
directory is below 10 MB, comfortably below its 150 MB task limit. The old
FM-only report and CSV remain present for negative evidence. The original
source/packet boundary and applicable repository rules are recorded in
`REPORT.md`; no additional engine code was consulted for this improved path.

## Reproduce

From this directory, using host GCC 14.2.0:

```sh
cc -std=c99 -O2 -shared -fPIC -ffp-contract=off -fno-fast-math coefficients.c -lm -o coefficients.so
python check.py
cc -std=c99 -O2 -ffp-contract=off -fno-fast-math sweep.c coefficients.c -lm -o sweep
./sweep > SWEEP-fixed.csv
cc -std=c99 -O2 -Wall -Wextra -Werror -fsanitize=undefined -fno-sanitize-recover=all -ffp-contract=off -fno-fast-math stress_fixed.c -lm -o stress_fixed
./stress_fixed > STRESS-fixed.csv
cc -std=c99 -Os -Wall -Wextra -Werror -ffp-contract=off -fno-fast-math -c fixed_only.c -o fixed_only.o
nm -u fixed_only.o
size fixed_only.o
```

The host oracle in the harness requires libm. `fixed_only.c` and its candidate
header do not. Compiler and executable exit statuses are recorded in the
tool outputs and resource JSON; all final versions exited zero.
