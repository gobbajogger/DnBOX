# Independent S1 coefficient preparation experiments

The reusable firmware envelope accepts prepared coefficients. These experiments
compare inexpensive existing DnBOX math and a separate integer candidate; neither
is wired into the production renderer. `fixed_candidate.h` is unchanged from its
independent source-free review. It is a finite numerical approximation, not a
proof of correct rounding for all controls or original-engine behavior.

From the repository root, use a fresh output directory:

```sh
python3 -B experiments/sound-hardware-v1/coefficient_followup/check.py --output /tmp/dnbox-coeff-new
```

This portable entry builds only a small host scalar library, verifies the pinned
fastmath, numeric packet and metadata hashes, compares all 379,288 output words for each
of five methods, and rejects a one-ULP attack mutation. It returns nonzero on a
failed scoped comparison. `--write-pcm` also saves candidate streams. Libm and
double arithmetic are comparison-oracle dependencies, not fixed-candidate code.

The isolated run's unchanged `RECOVERED-RESULT.json`, reports and CSVs remain
historical evidence. The original five-method packet result was exact, while
existing fastmath disagreed with the inferred coefficient formula for some
legal controls (`SWEEP.csv`). Those failures are preserved. The integer proposal
matched 5,360,032 scalar comparisons, including rounding/subnormal boundaries;
only 44,100-Hz scalar envelope recordings are original behavior observations.
Its 792-byte host object is not target size or cost evidence. Rational 3/5 attack
scaling differs mathematically from the oracle's binary64 0.6 intermediates.

The larger mathematical sweep and UBSan stress are separately reproducible:

```sh
cd experiments/sound-hardware-v1/coefficient_followup
cc -std=c99 -O2 -DX0X_HOST -ffp-contract=off -fno-fast-math -I../candidate_review/compact_profile_review/snapshot sweep.c coefficients.c -lm -o /tmp/dnbox-coeff-sweep
/tmp/dnbox-coeff-sweep > /tmp/dnbox-coeff-sweep.csv
cc -std=c99 -O2 -Wall -Wextra -Werror -fsanitize=undefined -fno-sanitize-recover=all -ffp-contract=off -fno-fast-math stress_fixed.c -lm -o /tmp/dnbox-coeff-stress
/tmp/dnbox-coeff-stress > /tmp/dnbox-coeff-stress.csv
```

The sweep intentionally reports the old methods' differences and is a diagnostic;
it does not return failure for those retained negative results. The stress
returns nonzero if the fixed proposal differs. No new native link, emulator,
full-suite, control-rate shortcut or production admission follows from this unit.

`PORTABLE-RESULT.json` is the final guarded run; `PORTABLE-BEFORE-GUARDS.json`
retains the initial portable run. Neither replaces the isolated historical receipt.
